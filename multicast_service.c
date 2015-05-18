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
#include "header.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define PARTY_COUNT 5
#define PARTY_NAME_LENGTH 3
#define VOTES_LENGTH 4
#define MSG_SIZE 2048
#define BACKLOG 5

volatile sig_atomic_t stop = 0;
char bufff[MSG_CONTENT_SIZE];

typedef struct sockinfo{
	int *fdmax;
	fd_set *fds;
}sockinfo;


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
	char buf[MSG_CONTENT_SIZE];
	int i;
	sockinfo *si = (sockinfo*)ptr;
	for (i = 0; ; i++){
		sleep(1);
		sem_wait(&mutex);
		bzero(buf,MSG_CONTENT_SIZE);
		snprintf(buf,MSG_CONTENT_SIZE,"%d: %s",i,bufff);
		multicast(*si->fds, *si->fdmax, buf);
		puts(buf);
		sem_post(&mutex); 
	}

	return NULL;
}


void get_results(char** parties, int* votes, int s, int m, fd_set *fds, int *fdmax){
	//char buf[MSG_CONTENT_SIZE];
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
					FD_SET(fd, &mfds);
					FD_SET(fd, fds);
					sem_post(&mutex); 
					printf("połączono z %d\n",fd);
					
					
				}
				else{
					puts("siema");
					sem_wait(&mutex);
					puts("no hej");
					bzero(bufff,MSG_CONTENT_SIZE);
					bulk_read(i,bufff,MSG_CONTENT_SIZE);
					FD_CLR(i,&mfds);
					sem_post(&mutex); 
					puts("bye bye");
				}
			}
	}
	pthread_join( thread1, NULL);
	sem_destroy(&mutex);
}

int main(int argc, char** argv){
	bzero(bufff,MSG_CONTENT_SIZE);
	strcpy(bufff,"bronkobus");
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
