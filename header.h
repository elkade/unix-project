#define _GNU_SOURCE 
#include <stdio.h> 
#include <termios.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define ADMIN_PORT 5555
#define USER_PORT 5565     
#define NAME_LENGTH 16
#define SIZE 128
#define MSG_SIZE 2048
#define MSG_CONTENT_SIZE (2048-256)
#define BACKLOG 5

#define NAME_LENGTH 16
#define FLOAT_LENGTH 48
#define INT_LENGTH 16
#define ENUM_LENGTH 4
#define MAX_DB_LINE_LENGTH 128


#define PORT_NUMBER_LENGTH 8
#define SERVICE_NAME_LENGTH 16

#define MAX_SOCKETS_TO_ONE_PORT 16

typedef struct wrapped_message{
	char service_name[SERVICE_NAME_LENGTH];
	char client_name[NAME_LENGTH];
	char content[MSG_SIZE];
}wrapped_message;

typedef struct local_endpoint{
	char port_number[PORT_NUMBER_LENGTH];
	char service_name[SERVICE_NAME_LENGTH];
	int sockfd;
	int connected_sockets[MAX_SOCKETS_TO_ONE_PORT];
	int connections_number;
}local_endpoint;

static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}
/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}
/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}
/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}
/* Read 1 character with echo */
char getche(void) 
{
  return getch_(1);
}
void delete(int n){
	int i;
	printf("%c[2K",27);
	for (i = 0; i < n; i++){
		printf("\b");
	}
}
void read_line(char* buf, int m){
	static char inputs[SIZE][SIZE];
	static int n;
	char c;
	int i=0, j=0, k=0;
	bzero(buf,m);
	n++;
	n %= SIZE;
	strncpy(inputs[n],buf,SIZE);	
	while((c=getch())!='\n'){
		if(c=='\033'){
			delete(k);
			k=0;
			bzero(buf,m);
			i=0;
			if((c=getch())!=91)
				continue;
			if((c=getch())=='A')
				j=(j-1+SIZE)%SIZE;
			else if(c=='B')
				j=(j+1+SIZE)%SIZE;
			else
				continue;
			strncpy(buf,inputs[(n+j+SIZE)%SIZE],SIZE);
			i=k=strlen(buf);
			delete(i);
			printf("%s",buf);
		}
		else if(c==127){
			delete(k);//to jest słabe
			if(k>0)
				k--;
			if(i>0)
				i--;
			buf[i]='\0';
			strncpy(inputs[n],buf,SIZE);
			printf("%s",buf);
		}
		else{
			k++;
			printf("%c",c);
			if(i<m){
				buf[i++]=c;
				if(i<m)
					buf[i]='\0';
			}
			strncpy(inputs[n],buf,SIZE);
		}
	}
	strncpy(inputs[n],buf,SIZE);
	puts("");
}

int trim(char* str, int n){
	int i;
	for (i = n -1; i >=0 ; i--){
		if(str[i]==' ' || str[i]=='\n' || str[i]=='\t')
			str[i]='\0';
		else
			break;
	}
	return 0;
}
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
