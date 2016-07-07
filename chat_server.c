#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<pthread.h>
#include<unistd.h>

#define BUFSIZE 100
#define NAMESIZE 20

void* send_thread(void* arg);
void* recv_thread(void* arg);
char name[NAMESIZE] = "[NULL]";
char message[BUFSIZE];
int send_cancel;

pthread_cond_t condid;
pthread_mutex_t mutexid;

int main(void)
{
	int serv_sd;
	int clnt_sd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	pthread_t snd_thread, rcv_thread;
	int sockopt = 1;

	if(pthread_mutex_init(&mutexid,NULL)!=0)
	{
		perror("pthread_mutex_init");
		exit(1);
	}
	if(pthread_cond_init(&condid,NULL)!=0)
	{
		perror("pthread_cond_init1");
		exit(1);
	}
	if((serv_sd = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		exit(1);
	}

	setsockopt(serv_sd,SOL_SOCKET, SO_REUSEADDR,&sockopt, sizeof(sockopt));
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(60000);

	if((bind(serv_sd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)))==-1)
	{
		perror("bind()");
		exit(1);
	}
	if(listen(serv_sd,5)==-1)
	{
		perror("listen()");
		exit(1);
	}

	while(1)
	{
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sd = accept(serv_sd,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
		printf("Client IP address : %s \n", inet_ntoa(clnt_addr.sin_addr));
		printf("\nname ?");
		fgets(name,NAMESIZE,stdin);	//input server user name
		name[strlen(name)-1]='\0';

		if(pthread_create(&snd_thread,NULL,send_thread,&clnt_sd)!=0)
		{
			perror("pthread_create");
			exit(1);
		}
		if(pthread_create(&rcv_thread,NULL,recv_thread,&clnt_sd)!=0)
		{
			perror("pthread_create");
			exit(1);
		}

		pthread_mutex_lock(&mutexid);
		pthread_cond_wait(&condid,&mutexid);
		pthread_mutex_unlock(&mutexid);

		if(send_cancel==1)
		{
			pthread_join(snd_thread,NULL);
			pthread_cancel(rcv_thread);
		}else {
			pthread_join(rcv_thread,NULL);
			pthread_cancel(snd_thread);
		}
		close(clnt_sd);


	}
	pthread_cond_destroy(&condid);
	return 0;
}




void* send_thread(void* arg)
{
	int sock = *((int*)arg);
	char name_message[NAMESIZE+BUFSIZE];
	char message[BUFSIZE];

	while(1)
	{
		fgets(message, BUFSIZE, stdin);
		sprintf(name_message,"%s> %s",name,message);
		write(sock,name_message,strlen(name_message));
		if(!strcmp(message,"bye\n"))
		{
			send_cancel = 1;	//exit by send thread
			pthread_cond_signal(&condid);
			return 0;
		}
	}
}

void* recv_thread(void* arg)
{
	int sock = *((int*)arg);
	char message[NAMESIZE+BUFSIZE];
	int str_len;
	while(1)
	{
		str_len = recv(sock, message,NAMESIZE+BUFSIZE-1,0);
		if(str_len==0)
		{
			printf("server:close from client\n");
			pthread_cond_signal(&condid);
			pthread_exit(NULL);
		}else if(str_len<0)
		{
			perror("recv");
			pthread_exit(NULL);
		}
		message[str_len]=0;
		printf("%s\n",message);
	}
}
