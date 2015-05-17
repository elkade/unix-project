#define _GNU_SOURCE 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define PARTY_COUNT 5
#define PARTY_NAME_LENGTH 3
#define VOTES_LENGTH 4
#define MSG_SIZE 2048
#define BACKLOG 5

volatile sig_atomic_t stop = 0;

typedef struct sockinfo{
	int *fdmax;
	fd_set *fds;
}sockinfo;

ssize_t bulk_read(int fd, char *buf, size_t count){
	int c;
	size_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(read(fd,buf,count));
		if(c<0) return c;
		if(0==c) return len;
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}
ssize_t bulk_write(int fd, char *buf, size_t count){
	int c;
	size_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(write(fd,buf,count));
		if(c<0) return c;
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}
int sethandler( void (*f)(int), int sigNo){
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) return -1;
	return 0;
}

void sigint_handler(int sig){
	stop = 1;
}

void multicast(fd_set fds, int fdmax, char* buf){
	int i;
	for (i = 0; i < fdmax + 1; i++){
		if (FD_ISSET(i, &fds)){
			if(bulk_write(i,buf,MSG_SIZE)<0){}// ERR("write");
			//printf("wysyłam do %d\n",i);
		}
	}
	//puts("");
}
void close_all(fd_set fds, int fdmax){
	int i;
	for (i = 0; i<fdmax + 1; i++)
		if (FD_ISSET(i, &fds)){
			shutdown(i,SHUT_RDWR);
			if(TEMP_FAILURE_RETRY(close(i))<0)ERR("close:");
		}
}

int addnewclient(int s, fd_set *mfds, int *fdmax){
	int fd;
	if ((fd = TEMP_FAILURE_RETRY(accept(s, NULL, NULL))) == -1)	ERR("Cannot accept connection");
	FD_SET(fd, mfds);
	*fdmax = (*fdmax < fd) ? fd : *fdmax;
	return fd;
}
void add(char** parties, int* votes, char* buffer){
	int i, k;
	char buf[MSG_SIZE];
	for (i = 0; i < PARTY_COUNT; i++)
		for (k = 0; k < PARTY_COUNT; k++)
			if(strncmp(parties[i],buffer + k*(PARTY_NAME_LENGTH+VOTES_LENGTH),PARTY_NAME_LENGTH)==0){
				votes[i]+=atol(buffer + PARTY_NAME_LENGTH + k*(PARTY_NAME_LENGTH+VOTES_LENGTH));
				break;
			}
	bzero(buffer,MSG_SIZE);
	for (i = 0; i < PARTY_COUNT; i++){
		snprintf(buf,MSG_SIZE-1,"%s\t%d\n",parties[i],votes[i]);
		strncat(buffer, buf, MSG_SIZE - strlen(buffer) - 1);
	}
}

sem_t mutex;

void *thread_handler( void *ptr ){
	char buf[128];
	int i;
	sockinfo *si = (sockinfo*)ptr;
	for (i = 0; ; i++){
		sleep(1);
		sem_wait(&mutex);
		bzero(buf,128);
		snprintf(buf,128,"%d: %s",i,"gówno");
		multicast(*si->fds, *si->fdmax, buf);
		sem_post(&mutex); 
	}

	return NULL;
}


void get_results(char** parties, int* votes, int s, int m, fd_set *fds, int *fdmax){
	//char buf[MSG_SIZE];
	int i, clientcount = 0;
	fd_set mfds, curfds;
	FD_ZERO(&mfds);
	FD_SET(s, &mfds);
	
	sockinfo si;
	si.fds = fds;
	si.fdmax = fdmax;
	
	sem_init(&mutex, 0, 1);
	
	pthread_t thread1;
    int iret1;

	//działa ale nie wyrejestrowuje
	
	iret1 = pthread_create( &thread1, NULL, thread_handler, (void*) &si);
    if(iret1){
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
        exit(EXIT_FAILURE);
    }

	
	
	for (curfds = mfds;; curfds = mfds){
		if (select(*fdmax + 1, &curfds, NULL, NULL, NULL) == -1){
			if (errno != EINTR) ERR("Cannot select");
			else if(stop){
				close_all(mfds, *fdmax);
				ERR("SIGINT");
			}
		}
		for (i = 0; i<*fdmax + 1; i++)
			if (FD_ISSET(i, &curfds)){
				if (s == i){
					clientcount++;
					sem_wait(&mutex); 
					int fd = addnewclient(s, &mfds, fdmax);
					FD_SET(fd, fds);
					sem_post(&mutex); 
					printf("połączono z %d\n",fd);
					
					
				}
				else{
				}
			}
	}
	pthread_join( thread1, NULL);
	sem_destroy(&mutex);
}
int create_socket(int port){
	struct sockaddr_in serv_addr;
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR("socket:");
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) ERR("bind:");
	if(listen(sockfd, BACKLOG) < 0) ERR("listen");
	return sockfd;
}
int main(int argc, char** argv){
	char *parties[] = {"ABC", "CDE", "EFG", "GHI", "IJK"};
	int votes[] = {0, 0, 0, 0, 0}, sockfd, fdmax;
	fd_set fds;
	FD_ZERO(&fds);
	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGPIPE:");
	fdmax = sockfd = create_socket(atoi("9998"));
	get_results(parties, votes, sockfd, 3, &fds, &fdmax);
	if(TEMP_FAILURE_RETRY(close(sockfd))<0)ERR("close:");
	close_all(fds,fdmax);
	exit(EXIT_SUCCESS);
}
