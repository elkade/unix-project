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

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define PARTY_COUNT 5
#define PARTY_NAME_LENGTH 3
#define VOTES_LENGTH 4
#define MSG_SIZE 256
#define BACKLOG 5

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


void usage(){
	printf("./client <adres IPv4> <port> \n");
	exit(EXIT_FAILURE);
}

int create_socket(char *hostname, int port){
	struct sockaddr_in serv_addr;
    struct hostent *server;
	int status, s = socket(AF_INET , SOCK_STREAM , 0);
	socklen_t size = sizeof(int);
	fd_set fds;
    if (s == -1) ERR("socket");
    if ((server = gethostbyname(hostname)) == NULL) ERR("gethostbyname");
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr = *(struct in_addr *) server->h_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (connect(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
		if (errno != EINTR)	ERR("connect");
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		if (TEMP_FAILURE_RETRY(select(s + 1, NULL, &fds, NULL, NULL)) == -1) ERR("select");
		if (getsockopt(s, SOL_SOCKET, SO_ERROR, &status, &size) == -1) ERR("getsockopt");
		if (status != 0) ERR("connect");
	}
    return s;
}
int main(int argc , char *argv[]){
	char message[MSG_SIZE];
	int sockfd;
    //if(argc!=3) usage();
    if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGPIPE:");
    sockfd = create_socket("127.0.0.1",atoi("5555"));
	bzero(message,MSG_SIZE);
	if( bulk_write(sockfd, message, MSG_SIZE) < 0) ERR("send");
    while(1){
		scanf("%s",message);
		if( bulk_write(sockfd, message, MSG_SIZE) < 0) ERR("send");
		int r = bulk_read(sockfd, message, MSG_SIZE);
        if( r < 0) ERR("recv");
        else if(r == 0) break;
        puts(message);
    }
    if(TEMP_FAILURE_RETRY(close(sockfd))<0)ERR("close:");
    exit(EXIT_SUCCESS);
}

