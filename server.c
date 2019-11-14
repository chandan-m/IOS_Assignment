#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 	
#include <dirent.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <sys/select.h>

#define N_WORK 5
#define BUFF_SIZE 64
#define BUFF_NO 10

//Structure to implement Shared Memory and synchronisation

typedef struct shmem
{
	int state;//Represent State of the Memory Segment
	//0 - Empty, 1 - Ready for Worker, 2 - Processed by Worker, 3 - Result Ready

	sem_t lock; // To ensure only one process can access it at one time.

	char text[BUFF_SIZE]; //Stores the Request
	//To be converted to Uppercase.
}shmem;


int main(void)
{
	int sockfd = 0;
	int connfd = 0;

	char recvBuff[1024];
	//char sendBuff[1024];
	char message[1024];
	
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket retrieve success\n");

  	memset(&serv_addr, 0, sizeof(serv_addr));
	//memset(sendBuff, 0, sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	serv_addr.sin_port = htons(5050);		// port:5050

	bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	if (listen(sockfd, 10) == -1)
  	{
		printf("Failed to listen\n");
		return -1;
  	}

  	int i,j;

	//Shared Memory
    int shmid;
    shmem *memory;
    
	//Variables used for select() to avoid Blocking Read
    fd_set set;
 	struct timeval timeout;
	int rv;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
		
	

	while (1)
  	{
		//Creating shared memory between Master Process and Worker Processes
		shmid = shmget((key_t)1111, sizeof(shmem)*BUFF_NO, 0666|IPC_CREAT);
		if (shmid == -1) {
    		perror("Error in creating Shared Memory");
      		return 1;
		}

		// Attach to the segment to get a pointer to it.
		memory = shmat(shmid, NULL, 0);
		if (memory == (void *) -1) {
    		perror("Error in attaching Shared Memory");
    		return 1;
		}

		//Initializing Shared Memory Array
		for(i=0;i<BUFF_NO;i++)
		{
			memory[i].state=0;
			sem_init(&(memory[i].lock),1,1);
			//memset(memory[i].text,sizeof(memory[i].text),0);
		}

		int pid = 1;
		//Create N_WORK no. of Worker Processes ONLY in the Master Process.
		for(i=0;i<N_WORK && pid!=0;i++)
		{
			pid = fork();
		}	

		if(pid > 0)//In Parent Process
		{	
			connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);

			printf("Parent Started\n");
			rv=1;
			while(1)
			{
				FD_ZERO(&set); /* clear the set */
  				FD_SET(connfd, &set);
				rv = select(connfd + 1, &set, NULL, NULL, &timeout);
				// Check if there is any request from the Client.

				if(rv>0)
				{
					read(connfd,recvBuff,sizeof(recvBuff));
					strcpy(message,recvBuff);

					//Check for an empty Segment and insert the request into it (State=0).
					for(i=0;i<BUFF_NO;i++)
					{
						if(memory[i].state==0)
						{
							sem_wait(&memory[i].lock);
							memory[i].state=1;
							strcpy(memory[i].text,message);
							memory[i].text[strlen(message)]=0;
							sem_post(&memory[i].lock);
							break;
						}
					}

					if(i==BUFF_NO)
					{
						printf("\nServer Output : BUFFER FULL\nCannot Process : %s\n\n",recvBuff);
						write(connfd,"BUFFER FULL : Cannot Process",BUFF_SIZE);
					}
					else printf("In Buffer : %s\n",recvBuff);
					memset(recvBuff,sizeof(recvBuff),0);
				}
				//Check for any Results that are Ready (State=3) and print the result.
				for(i=0;i<BUFF_NO;i++)
				{
					if(memory[i].state==3)
					{
						sem_wait(&memory[i].lock);
						printf("Server Output : %s\n",memory[i].text);
						memset(memory[i].text,sizeof(memory[i].text),0);
						write(connfd,memory[i].text,BUFF_SIZE);
						memory[i].state=0;
						sem_post(&memory[i].lock);
						sleep(0.1);
					}
				}

			}
		  	close(connfd);		// Close the socket connection
    		sleep(1);		// Suspend for 1 second
		}
		if(pid == 0)// In Child process
		{
			struct timespec tm;
			printf("Child Started\n");
			while(1)
			{
				//Find a Segment that is Ready to be Processed (State=1).
				for(i=0;i<BUFF_NO;i++)
				{
					if(memory[i].state==1)
					{
						clock_gettime(CLOCK_REALTIME, &tm);
        				tm.tv_sec += 1;
						//Timed Wait on the semaphore to Prevent a process to wait for an other Process, 
						//when both of them acces the same segment at the same time
        				if(sem_timedwait(&memory[i].lock,&tm)==-1)continue;

        				printf("Worker started working on memory %d\n",i);
						memory[i].state=2;//Change State to indicate that it is being processed.
						//Convert it to Uppercase
						for(j=0;j<strlen(memory[i].text);j++)
						{
							if(memory[i].text[j]>=97 && memory[i].text[j]<=122)
							{
								memory[i].text[j]-=32;
							}
						}
						//Sleep to demonstrate Synchronisation
						sleep(5);
						printf("Worker finished working on memory %d\n",i);
						memory[i].state=3;//Change state to indicate that Result is ready
						sem_post(&memory[i].lock);
					}
				}
			}
		}
       
	}
	return 0;
}
