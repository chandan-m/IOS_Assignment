#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

int sockfd = 0;
int bytesReceived = 0;
char recvBuff[256];

void *myThread(void *vargp)
{
	//printf("Ready to Recieve\n");
	while(1)
	{
		while(bytesReceived=read(sockfd,recvBuff,sizeof(recvBuff))>0)
		{
			printf("Response : %s\n",recvBuff);
		}
	}
}

int main(int argc, char **argv) 
{
	

	memset(recvBuff, 0, sizeof(recvBuff));

	struct sockaddr_in serv_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 		// Create socket
	{
		printf("\n Error : Could not create socket \n");
	    return 1;
  	}

	serv_addr.sin_family = AF_INET;		
	serv_addr.sin_port = htons(5050);	// port:5050
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	// local host IP address: 127.0.0.1

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	}
	
	pthread_t thread_id;  
	pthread_create(&thread_id, NULL, myThread, NULL);
	
	char ch[256];
	printf("Enter a message :\n");
	while(1)
	{
		scanf("%s",ch);
		write(sockfd,ch,sizeof(recvBuff));
	}

	pthread_join(thread_id, NULL);
	
	return 0;
}