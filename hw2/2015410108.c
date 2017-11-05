#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

//Message types of IR protocol. If one of 4 bits is set to 1, the others must be 0.
//처음 4bit는 0b0001, 0b0010, 0b0100, 0b1000 중 하나여야 한다.
//그 다음의 4bit는 reserve bit로, 0000이어야 한다.
//그 다음은 8bit 짜리 overation field. -> client가 무슨 일을 해야 하는지 정해준다.
//그 다음은 16bit 짜리 data length. Data length in byte.
//sequential number는 response message와 instruction message가 서로 같아야 한다.ㄴ
#define FLAG_HELLO          ((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION    ((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE       ((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE      ((unsigned char)(0x01 << 4))

//operation field. Server가 instruction message를 보냈을 때. packet을 보내면 0으로 설정.
#define OP_ECHO             ((unsigned char)(0x00)) //'response' client가 server에 받은 그대로 전송
#define OP_INCREMENT        ((unsigned char)(0x01)) //'response' server가 보낸 숫자에 1을 더해서 전송
#define OP_DECREMENT        ((unsigned char)(0x02)) //'response' server가 보낸 숫자에 1을 빼서 전송

#define SERVER_PORT 47500

struct hw_packet {
    unsigned char flag; //4b + 4b는 empty
    unsigned char operation; // 8b 
    unsigned short data_len; // 16b
    unsigned int seq_num; // 32b
    char data[1024];
};

struct header {
	unsigned char flag; //4b
    unsigned char operation; // 8b 
    unsigned short data_len; // 16b
    unsigned int seq_num; // 32b
};

int main(void) {
	char buf[1024]; //내가 보낸다.
	char buf_get[1024]; //내가 받는다.
	char data_str_get[1024]; //내가 받은 내용.
	int* data_int = (int*)malloc(sizeof(int)); //inc,dec 용도.

	struct sockaddr_in server; //server address
	int s;
	
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET; //connect internet
	server.sin_port = htons(SERVER_PORT);

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk : socket");
		exit(1);
	}

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }

	unsigned int value;
    struct hw_packet hello;
    hello.flag = FLAG_HELLO;
    hello.operation = OP_ECHO;
    hello.seq_num = htons((unsigned short)0);
    hello.data_len = htonl(4);
    value = 2015410108;
    printf("*** starting ***\n\n");
    memcpy(hello.data, &value, sizeof(unsigned int)); //1111000001000001011011110111100
    memcpy(buf, &hello, sizeof(struct hw_packet));
 	printf("sending first hello msg...\n");
    send(s, buf, 1023,0);
    //printf("%d", hello.data);
    recv(s, buf_get,1023,0); //hello 받고
    printf("received hello message from the server!\n");
	memset(buf, '\0', sizeof(buf));
	memset(buf_get, '\0', sizeof(buf));
	printf("waiting for the first instruction message...\n\n");
    recv(s, buf_get,1023,0); //다음거

    
    struct hw_packet bye;
    //memcpy(&bye, buf_get, sizeof(struct hw_packet));
    int i=0;
    if (buf_get[0] == FLAG_INSTRUCTION) {
    	while(buf_get[0]!=FLAG_TERMINATE) {
    		memcpy(&bye, buf_get, sizeof(struct hw_packet));
    		printf("operation type is ");
    		memset(buf, '\0', sizeof(buf));
    		//unsigned short _seq_num = memcpy(&_seq_num, buf_get.data; sizeof(unsigned short));
    		//unsigned short _seq_num = ntohs(bye.seq_num);
    		unsigned short _seq_num = bye.seq_num;
    		//printf("%d\n", _seq_num);
    		//unsigned short _seq_num;
    		//memcpy(_seq_num, &bye.seq_num, sizeof(unsigned short));
    		if (buf_get[1]==OP_ECHO) {
    			struct hw_packet hi;
    			printf("echo.\n");
    			printf("echo : ");
    			printf("%s\n", bye.data);
    			int data_len = ntohl(*(int *)(buf_get + 4));

    			for (i =0;i<data_len;i++) {
    				data_str_get[i] = (buf_get + sizeof(struct header))[i];
    			}

    			hi.flag = FLAG_RESPONSE;
    			hi.operation=OP_ECHO;
    			//hello.seq_num=htons((unsigned short)_seq_num);
    			hi.seq_num=bye.seq_num;
    			//printf("%d\n", hello.seq_num);
    			hi.data_len=htonl(data_len);
    			memcpy(hi.data, data_str_get,1023);
    			memcpy(buf, &hi,sizeof(buf));

    			send(s, buf, 1023,0);
    			memset(buf, '\0', sizeof(buf));
    		}
    		else if (buf_get[1]==OP_INCREMENT) {
    			struct hw_packet hi;
    			printf("increment.\n");
    			printf("increment : ");
    			printf("%s\n", bye.data);
    			*data_int = ntohl(*(int *)(buf_get + sizeof(struct header)));

    			hi.flag = FLAG_RESPONSE;
    			hi.operation=OP_ECHO;
    			//hello.seq_num=htons((unsigned short)_seq_num);
    			hi.seq_num=bye.seq_num;
    			//printf("%d\n", hello.seq_num);
    			hi.data_len=htonl(4);
    			*data_int = *data_int + 1;
    			*data_int = htonl(*data_int);
    			//hello.data=data_int;
    			memcpy(hi.data, data_int,4);
    			memcpy(buf, &hi,sizeof(buf));

    			send(s, buf, 1023,0);
    			memset(buf, '\0', sizeof(buf));
    		}
			else if (buf_get[1]==OP_DECREMENT) {
				struct hw_packet hi;
				printf("decrement.\n");
				printf("decrement : ");
    			printf("%s\n", bye.data);
    			*data_int = ntohl(*(int *)(buf_get + sizeof(struct header)));

    			hi.flag = FLAG_RESPONSE;
    			hi.operation=OP_ECHO;
    			//hello.seq_num=htons((unsigned short)_seq_num);
    			hi.seq_num=bye.seq_num;
    			//printf("%d\n", hello.seq_num);
    			hi.data_len=htonl(4);
    			*data_int = *data_int - 1;
    			*data_int = htonl(*data_int);

    			memcpy(hi.data, data_int,4);
    			memcpy(buf, &hi,sizeof(buf));

    			send(s, buf, 1023,0);
    			memset(buf, '\0', sizeof(buf));
    		}
    		printf("sent response msg with seq.num. %d to server.\n\n", bye.seq_num);
    		memset(buf_get, '\0', sizeof(buf_get));
            read(s, buf_get, 1024);
    	}
    }
    close(s);
    return 0;
}