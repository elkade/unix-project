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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include "header.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define MSG_SIZE 2048
#define BACKLOG 5



void usage(){
	printf("./client <adres IPv4> <port> \n");
	exit(EXIT_FAILURE);
}

int create_socket_client(char *hostname, int port){
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

void int_to_str(char* str, int n, int i){//zwraca z terminatorem więc dla 4 trzeba podać 5
	char buf[n];
	bzero(buf,n);
	snprintf(buf,n,"%0*d",n-1,i);
	strncpy(str,buf,n);
}

//local_endpoint local_endpoint_new(char* port_number, char* service_name, int socket_number){
	//local_endpoint endpoint;
	//endpoint.port_number
//}

int addnewfd_listen(int s, fd_set *fds, int *fdmax){
	int fd = s;
	FD_SET(fd, fds);
	*fdmax = (*fdmax < fd) ? fd : *fdmax;
	return fd;
}

int addnewfd(int s, fd_set *fds, int *fdmax){
	int fd;
	if ((fd = TEMP_FAILURE_RETRY(accept(s, NULL, NULL))) == -1)	ERR("Cannot accept connection");
	FD_SET(fd, fds);
	*fdmax = (*fdmax < fd) ? fd : *fdmax;
	return fd;
}

void wrapped_message_to_str(char* buf, wrapped_message msg, int n){
	char buf2[MSG_SIZE];
	bzero(buf2,MSG_SIZE);

	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.service_name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, SERVICE_NAME_LENGTH+1, "%s", msg.service_name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.client_name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, NAME_LENGTH+1, "%s", msg.client_name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.content));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, MSG_CONTENT_SIZE+1, "%s", msg.content);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	}

int main(int argc , char *argv[]){
	char name[NAME_LENGTH];
	char message[MSG_SIZE];
	int n = 1, i, j;//i to indeksator
	local_endpoint endpoints[n];
	int fdmax;
	fd_set allfds, curfds;
	FD_ZERO(&allfds);
	
	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGPIPE:");
	puts("before init");
	for (i = 0; i < n; i++){
		int_to_str(endpoints[i].port_number,PORT_NUMBER_LENGTH,5578);
		strncpy(endpoints[i].service_name,"multicast",SERVICE_NAME_LENGTH);
		puts("before socket creation");
		endpoints[i].sockfd = create_socket(atoi(endpoints[i].port_number));
		puts("after socket creation");
		addnewfd_listen(endpoints[i].sockfd,&allfds,&fdmax);
		puts("after adding new fd");
		endpoints[i].connections_number = 0;
	}
	puts("after init");
	int serverfd = create_socket_client("127.0.0.1",USER_PORT);
	addnewfd_listen(serverfd,&allfds,&fdmax);
	puts("connected to server");
	bzero(name,NAME_LENGTH);
	bzero(message,MSG_SIZE);

	read_line(name,NAME_LENGTH);
		
	for (curfds = allfds;; curfds = allfds){
		//puts("czekam...");
		if (select(fdmax + 1, &curfds, NULL, NULL, NULL) == -1)
			if (errno != EINTR) ERR("Cannot select");
		if (FD_ISSET(endpoints[0].sockfd, &curfds)){
			puts("jest połączenie:");
		}
		if(FD_ISSET(serverfd,&curfds)){
			if( bulk_read(serverfd, message, MSG_SIZE) < 0) ERR("read");
			//puts("new message from server:");
			//puts(message);
		}
		for (i = 0; i < n; i++){
			printf("%s\n",endpoints[i].port_number);
			if (FD_ISSET(endpoints[i].sockfd, &curfds)){//nowe połączenie na i-ty port
				puts("new connection");
				if(endpoints[i].connections_number + 1 < MAX_SOCKETS_TO_ONE_PORT){
					endpoints[i].connected_sockets[endpoints[i].connections_number] = addnewfd(endpoints[i].sockfd,&allfds,&fdmax);
					endpoints[i].connections_number++;
				}
				else
					puts("too many connections");
			}
			for (j = 0; j < endpoints[i].connections_number; j++){
				if (FD_ISSET(endpoints[i].connected_sockets[j], &curfds)){//jak połączenie się skończy to indeks przepada
					int r = bulk_read(endpoints[i].connected_sockets[j], message, MSG_SIZE);
					if( r < 0) ERR("recv");
					else if(r == 0) {
						FD_CLR(endpoints[i].connected_sockets[j],&allfds);
						continue;
					}
					puts("new message from app:");
					puts(message);
					//ubieram wiadomość i przesyłam do serwera
					wrapped_message msg;
					memset(&msg,'\0',sizeof(msg));
					
					strcpy(msg.service_name, endpoints[i].service_name);
					strcpy(msg.client_name, name);
					strcpy(msg.content,message);
					
					bzero(message,MSG_SIZE);
					
					wrapped_message_to_str(message,msg,MSG_SIZE);
					puts(message);
					if(bulk_write(serverfd,message,MSG_SIZE)<0)
						ERR("write");
				}
			}
		}
		

			
			//while(1){
				//if( bulk_read(serverfd, message, MSG_SIZE) < 0) ERR("read");
				//if( bulk_write(afd, message, MSG_SIZE) < 0) ERR("send");
				//puts(message);
		//}
	}

    if(TEMP_FAILURE_RETRY(close(serverfd))<0)ERR("close:");
    for (i = 0; i < n; i++){
		if(TEMP_FAILURE_RETRY(close(endpoints[0].sockfd))<0)ERR("close:");
	}
    exit(EXIT_SUCCESS);
}


