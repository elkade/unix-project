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
#include <netdb.h>

#include "header.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define PARTY_COUNT 5
#define PARTY_NAME_LENGTH 3
#define VOTES_LENGTH 4
#define BACKLOG 5

void usage(){
	printf("./client <adres IPv4> <port> \n");
	exit(EXIT_FAILURE);
}

int main(int argc , char *argv[]){
	char message[MSG_SIZE];
	int sockfd;
    //if(argc!=3) usage();
    if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGPIPE:");
    sockfd = create_socket_client("127.0.0.1",atoi("5578"));
    puts("connected");
	bzero(message,MSG_SIZE);

	strcpy(message,"kasia cichopek");
	read_line(message,MSG_SIZE);
	bulk_write(sockfd, message /*"andrzej duda"*/, MSG_SIZE);
		bzero(message,MSG_SIZE);
    while(1){
		//read_line(message,MSG_SIZE);
		int r = bulk_read(sockfd, message, MSG_SIZE);
        if( r < 0) ERR("recv");
        //else if(r == 0) break;
        puts(message);
    }
    if(TEMP_FAILURE_RETRY(close(sockfd))<0)ERR("close:");
    exit(EXIT_SUCCESS);
}


