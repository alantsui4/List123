#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>


#define Buffer_Size 256
struct message_s
{
	unsigned char protocol[5]; /* protocol string (5 bytes) */
	unsigned char type;		   /* type (1 byte) */
	unsigned int length;	   /* length (header + payload) (4 bytes) */
} __attribute__((packed));

struct packet
{
	struct message_s header;
	char payload[1024];
} __attribute__((packed));

//Ensure complete data transfer
int sendn(int sd, void *buf, int buf_len)
{
	int n_left = buf_len;
	int n;
	while (n_left > 0)
	{
		if ((n = send(sd, buf + (buf_len - n_left), n_left, 0)) < 0)
		{
			if (errno == EINTR)
				n = 0;
			else
				return -1;
		}
		else if (n == 0)
		{
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

int recvn(int sd, void *buf, int buf_len)
{
	int n_left = buf_len;
	int n;
	while (n_left > 0)
	{
		if ((n = recv(sd, buf + (buf_len - n_left), n_left, 0)) < 0)
		{
			if (errno == EINTR)
				n = 0;
			else
				return -1;
		}
		else if (n == 0)
		{
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

void quitWithUsageMsg()
{
	printf("Usage: ./myftpclient <server ip addr> <server port> <list|get|put> <file>\n");
	exit(0);
}

void myftpList(int sd){
	//Work here

	//Construct List Request
	struct message_s list_request; 
	memcpy(list_request.protocol,  (unsigned char[]){ 'm', 'y', 'f', 't', 'p' }, 5); 
	list_request.type = 0xA1;
	list_request.length = sizeof(struct message_s);
	struct packet list_request_packet;
	list_request_packet.header = list_request;

	//Send List Request (Packet Size = Header Size, Payload Size = 0)
	if(sendn(sd, &list_request_packet,list_request_packet.header.length) < 0){
		printf("Send Error: %s (Errno:%d)\n", strerror(errno), errno);
		exit(0);
	}
	/* my Code */
	int len;
	struct packet list_reply;
	int totalsize=0;
	if((len=recvn(sd,&list_reply,10))<0){
		printf("Send error: %s (Errno:%d)\n",strerror(errno),errno);
		return;
	}
	if(len==0)
		return;
	if(list_reply.header.length > 10){
		printf("---file list start---");
		char payload[Buffer_Size+1];
		while(1){
			memset(&payload,0,Buffer_Size+1);
			if((len = recv(sd,&payload, Buffer_Size,0)) < 0){
				printf("Send error: %s (Errno:%d)\n",strerror(errno),errno);
				break;
			}
			printf("%s",payload);
			totalsize+=len;
			if(list_reply.header.length-10 <= totalsize)
				break;
		}
		
		printf("----- file list end -----\n");
	}
	/* my Code */

	printf("List Request Sent.\n");
	return;
}

void myftpGet(int sd, char* filename){
	//Work here
	printf("Get (%s)\n", filename);
}

void myftpPut(int sd, char* filename){
	//Work here
	printf("Put (%s)\n", filename);
}


int main(int argc, char **argv)
{
	//Mode
	// 0 - LIST
	// 1 - GET
	// 2 - PUT
	int mode;
	char filename[255];

	//Input Checking
	if (argc < 4)
	{
		quitWithUsageMsg();
	}

	if ((strcmp(argv[3], "get")) == 0 || (strcmp(argv[3], "put") == 0))
	{
		if (argc == 5)
		{
			strcpy(filename, argv[4]);
			if(strcmp(argv[3], "get") == 0){
				mode = 1;
			}else{
				mode = 2;
			}
		}
		else
		{
			quitWithUsageMsg();
		}
	}
	else if ((strcmp(argv[3], "list")) == 0)
	{
		mode = 0;
	}
	else
	{
		quitWithUsageMsg();
	}

	//Connection Setup
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("connection error: %s (Errno:%d)\n", strerror(errno), errno);
		exit(0);
	}
	// int le = 0x12345678;
	// printf("%x\n", le);
	// le = htonl(le);
	// printf("%x\n", le);

	//Operation
	switch (mode)
	{
	case 0:
		myftpList(sd);
		break;
	case 1:
		myftpGet(sd, filename);
		break;
	case 2:
		myftpPut(sd, filename);
		break;
	}

	close(sd);
	// while (1)
	// {
	// 	char buff[100];
	// 	memset(buff, 0, 100);
	// 	scanf("%s", buff);
	// 	printf("%s\n", buff);
	// 	int len;
	// 	if ((len = sendn(sd, buff, strlen(buff))) < 0)
	// 	{
	// 		printf("Send Error: %s (Errno:%d)\n", strerror(errno), errno);
	// 		exit(0);
	// 	}
	// 	printf("Send completed.\n");
	// 	if (strcmp(buff, "exit") == 0)
	// 	{
	// 		close(sd);
	// 		break;
	// 	}
	// }
	return 0;
}