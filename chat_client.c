#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>

#define BUFSIZE 100
#define NAMESIZE 20

void* send_thread(void* arg);
void* recv_thread(void* arg);

char name[NAMESIZE] = "[NULL]";
char message[BUFSIZE];
int send_cancel;
pthread_cond_t condid;

//
pthread_mutex_t mutexid;
//


int main(int argc, char** argv)
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void* thread_result;

	sock = socket(AF_INET, SOCK_STREAM,0);

	if(sock==-1)
	{
		perror("socket");
		exit(1);
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(60000);

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
	{
		perror("connect");
		exit(1);
	}

	printf("Name ? ");
	fgets(name,NAMESIZE,stdin);
	name[strlen(name)-1]='\0';

	if(pthread_create(&snd_thread,NULL,send_thread,(void*)&sock)!=0)
	{
		perror("pthread_create");
		exit(1);
	}
	if(pthread_create(&rcv_thread,NULL,recv_thread,(void*)&sock)!=0)
	{
		perror("pthread_create");
		exit(1);
	}
	
	/*
	if(pthread_join(snd_thread,&thread_result)!=0)
	{
		perror("pthread_join");
	}
	if(pthread_join(rcv_thread,&thread_result)!=0)
	{
		perror("pthread_join");
	}
	*/
	
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

	close(sock);
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
                        send_cancel = 1;        //exit by send thread
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
                        printf("client:close from server\n");
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


