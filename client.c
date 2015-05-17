#define _GNU_SOURCE 

#include "header.h"
#include "wrapped_message.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define MSG_SIZE 2048
#define BACKLOG 5



void usage(){
	printf("./client <adres IPv4> <port> \n");
	exit(EXIT_FAILURE);
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


int main(int argc , char *argv[]){
	char name[NAME_LENGTH];
	char message[MSG_SIZE];
	int n = 1, i, j;//i to indeksator
	local_endpoint endpoints[n];
	int fdmax;
	fd_set allfds, curfds, serverfds;
	FD_ZERO(&allfds);
	FD_ZERO(&serverfds);
	
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
		FD_ZERO(&endpoints[i].fds);
		FD_SET(endpoints[i].sockfd,&endpoints[i].fds);
	}
	puts("after init");
	int serverfd = create_socket_client("127.0.0.1",USER_PORT);
	addnewfd_listen(serverfd,&serverfds,&fdmax);
	FD_SET(serverfd,&allfds);
	
	puts("connected to server");
	bzero(name,NAME_LENGTH);
	bzero(message,MSG_SIZE);

	strcpy(name,"aleksander");
	//read_line(name,NAME_LENGTH);
	
	int select_all_number, select_one_number;
	
	struct timeval tt = {0,0};
	while(true){
start:
		curfds = allfds;
		if ((select_all_number = select(fdmax + 1, &curfds, NULL, NULL, NULL))<0) perror("select");//jest coś ale nie wiadomo co
		curfds = serverfds;
		if (select(fdmax + 1, &curfds, NULL, NULL, &tt)<0) perror("select");//sprawdzam serwer
		if(FD_ISSET(serverfd,&curfds)){
			select_all_number--;
			if( bulk_read(serverfd, message, MSG_SIZE) < 0) ERR("read");
			puts("new message from server:");
			puts(message);
			//trzeba sparsować wiadomość
			
			wrapped_message msg_from_server;
			memset(&msg_from_server,'\0',sizeof(msg_from_server));
			str_to_wrapped_message(message,&msg_from_server,MSG_SIZE);
			printf("%s\n",msg_from_server.service_name);
			printf("%s\n",msg_from_server.client_name);
			printf("%s\n",msg_from_server.content);
			//trzeba sprawdzić, czy przyszła do dobrego klienta
			
			if(strcmp(msg_from_server.client_name,name)==0)
				printf("dobry klient\n");
			else
				printf("zły klient %s\t%s\n",msg_from_server.client_name,name);
						
			//a teraz trzeba ją wysłać tam gdzie trzeba
			
			for (i = 0; i < n; i++)
				if(strcmp(endpoints[i].service_name,msg_from_server.service_name)==0){
					puts("znaleziono");
					for (j = 0; j <= fdmax ; j++)
						if(FD_ISSET(j,&endpoints[i].fds) && j!=endpoints[i].sockfd)
							if(bulk_write(j,msg_from_server.content,MSG_SIZE)<0)
								ERR("write");//tu się wypierdala, bo nie zamykam socketów
				}
		}
		if(select_all_number<=0) goto start;
		for (i = 0; i < n; i++){
			curfds = endpoints[i].fds;
			if ((select_one_number = select(fdmax + 1, &curfds, NULL, NULL, &tt))<0) perror("select");//sprawdzam kolejne porty
			if(select_one_number==0)continue;
			for (j = 0; j <= fdmax; j++){
				if(FD_ISSET(j,&curfds)){//coś jest
					select_all_number--;
					select_one_number--;
					if(j==endpoints[i].sockfd){//nowe połączenie
						int newfd = addnewfd(endpoints[i].sockfd,&endpoints[i].fds,&fdmax);
						FD_SET(newfd, &allfds);
					}
					else{//nowa wiadomość
						int r = bulk_read(j, message, MSG_SIZE);
						if( r < 0) ERR("recv");
						else if(r == 0) {
							FD_CLR(j,&allfds);
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
				if(select_all_number<=0) goto start;
				if(select_one_number<=0) continue;
			}
		}
	}

    if(TEMP_FAILURE_RETRY(close(serverfd))<0)ERR("close:");//zamykanie trzeba zrobić dobrze
    for (i = 0; i < n; i++){
		if(TEMP_FAILURE_RETRY(close(endpoints[0].sockfd))<0)ERR("close:");
	}
    exit(EXIT_SUCCESS);
}


