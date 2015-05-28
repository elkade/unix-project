#define _GNU_SOURCE 

#include <semaphore.h>
#include "header.h"
#include "wrapped_message.h"
#include "set.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define MSG_SIZE 2048
#define BACKLOG 5
sem_t mutex;

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

typedef struct thread_data{
	char local_port_number[PORT_NUMBER_LENGTH];
	char remote_port_number[PORT_NUMBER_LENGTH];
	char service_name[SERVICE_NAME_LENGTH];
	char *client_name;
	int *fdmax;
}thread_data;

int send_register(int serverfd, char* client_name){
	char message[MSG_SIZE];
	bzero(message,MSG_SIZE);
	wrapped_message msg;
	bzero(msg.content,MSG_CONTENT_SIZE);
	strcpy(msg.content,"");
	
	bzero(msg.service_name,SERVICE_NAME_LENGTH);
	strcpy(msg.service_name,"");
	
	bzero(msg.client_name,NAME_LENGTH);
	strcpy(msg.client_name,client_name);
	msg.status = REGISTER;
	bzero(message,MSG_SIZE);
	wrapped_message_to_str(message,msg,MSG_SIZE);
	puts("wysyłam2");
	printf("%s\n",message);
	if( bulk_write(serverfd, message, MSG_SIZE) < 0){
		puts("błąd wysyłania");
		return 1;
	}
	return 0;
}

void *thread_handler( void *ptr ){
	puts("starting thread");
	thread_data td = *((thread_data*) ptr);
	fd_set allfds, curfds, serverfds;
	FD_ZERO(&allfds);
	FD_ZERO(&serverfds);
	
	puts(td.service_name);
	puts(td.client_name);
	puts(td.remote_port_number);
	puts(td.local_port_number);
	
	int *fdmax = td.fdmax, i, m=4, next_app_id = 0, bulk;//n to max liczba aplikacji
	puts("tworzę socket oczekujący");
	int sockfd = create_socket(atoi(td.local_port_number));
	puts("stworzyłem");
	
	sem_wait(&mutex);
	addnewfd_listen(sockfd,&allfds,fdmax);//LOCK
	sem_post(&mutex);
	//OBSŁUŻYĆ MOŻLIWOŚĆ PRZEPEŁNIENIA SETU



	char message[MSG_SIZE], *client_name = td.client_name, *service_name = td.service_name;
	single_set aset;
	single_set_init(&aset,m);
	
	printf("łączę się z portem %d\n",atoi(td.remote_port_number));
	int serverfd = create_socket_client("127.0.0.1",atoi(td.remote_port_number));
	puts("połączyłem");
	
	sem_wait(&mutex);
	addnewfd_listen(serverfd,&allfds,fdmax);//lock
	sem_post(&mutex);

	bzero(message,MSG_SIZE);
	send_register(serverfd, td.client_name);
	int select_all_number;
	while(true){
start:
		puts("czekam");
		curfds = allfds;
		if ((select_all_number = select(*fdmax + 1, &curfds, NULL, NULL, NULL))<0)
			perror("select");
		printf("dostałem %d rzeczy\n",select_all_number);
		puts("fd:");
		for (i = 0; i < *fdmax+1; i++)
		{
			if(FD_ISSET(i,&curfds)){
				printf("%d\n",i);
			}
		}
		sleep(1);
		if(FD_ISSET(serverfd,&curfds)){
			select_all_number--;
			if( (bulk = bulk_read(serverfd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
				puts("server stopped working");
				goto end;
			}
			else if(bulk < 0)
				ERR("read:");
			puts("new message from server:");
			puts(message);
			
			wrapped_message msg_from_server;
			str_to_wrapped_message(message,&msg_from_server,MSG_SIZE);
			printf("%s\n",msg_from_server.app_name);//dodać obsługę app_name
			printf("%s\n",msg_from_server.service_name);
			printf("%s\n",msg_from_server.client_name);
			printf("%s\n",msg_from_server.content);
			
			if(msg_from_server.status==SERVICE_DISCONNECTED){
				puts("\n\n\n\n\nSERWIS ROZŁĄCZONY\n\n\n\n\n");
				goto end;
			}
			
			if(strncmp(msg_from_server.client_name,client_name,NAME_LENGTH)==0)
				printf("dobry klient\n");
			else
				printf("zły klient %s\t%s\n",msg_from_server.client_name,client_name);	
			
			for (i = 0; i < aset.n; i++){
				if(aset.arr[i].is_empty)
					continue;
				//if(strncmp(aset.arr[i].name, msg_from_server.app_name, APP_NAME_LENGTH)==0)
					if( (bulk = bulk_write(aset.arr[i].fd, msg_from_server.content,MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
						puts("app stopped working");
						FD_CLR(aset.arr[i].fd,&allfds);
						single_set_remove_by_fd(&aset,aset.arr[i].fd);
						wrapped_message msg_from_app;
						strncpy(msg_from_app.service_name, service_name, SERVICE_NAME_LENGTH);
						strncpy(msg_from_app.client_name, client_name, NAME_LENGTH);
						strncpy(msg_from_app.app_name, aset.arr[i].name, APP_NAME_LENGTH);
						strncpy(msg_from_app.content, "", MSG_CONTENT_SIZE);
						msg_from_app.status = DEREGISTER;
						wrapped_message_to_str(message,msg_from_app,MSG_SIZE);
						puts("wysyłam wyrejestrowanie do serwera");
						if( (bulk = bulk_write(serverfd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
							puts("server stopped working");
							goto end;
						}
						else if(bulk < 0)
							ERR("write:");
						goto start;
					}
					else if(bulk < 0)
						ERR("write:");
			}
		}
		
		if(select_all_number<=0) goto start;
		
		if(FD_ISSET(sockfd,&curfds)){
			puts("nowa aplikacja");
			char buf[APP_NAME_LENGTH];
			bzero(buf, APP_NAME_LENGTH);
			snprintf(buf, APP_NAME_LENGTH, "%d", next_app_id++);
			select_all_number--;
			sem_wait(&mutex);
			int newfd = addnewfd(sockfd,&allfds,fdmax);//lock
			sem_post(&mutex);
			single_set_add_by_fd(&aset, newfd);
			single_set_update_by_fd(&aset, newfd, buf);
			printf("przydzieliłem nowemu fd: %d name: %s\n",newfd, buf);
		}		
		if(select_all_number<=0) goto start;
		puts("szukam na liście aplikacji bo coś od nich dostałem");
		for (i = 0; i < aset.n; i++){
			if(aset.arr[i].is_empty)
				continue;
			printf("mam na liście fd %d\n",aset.arr[i].fd);
			if(FD_ISSET(aset.arr[i].fd,&curfds)){
				select_all_number--;
				wrapped_message msg_from_app;
				strncpy(msg_from_app.service_name, service_name, SERVICE_NAME_LENGTH);
				strncpy(msg_from_app.client_name, client_name, NAME_LENGTH);
				strncpy(msg_from_app.app_name, aset.arr[i].name, APP_NAME_LENGTH);
				if((bulk = bulk_read(aset.arr[i].fd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
					puts("app stopped working");
					//tu trzeba wywołać jakąś funkcję, która by usuwała z setu i fdmax zmieniała
					FD_CLR(aset.arr[i].fd,&allfds);
					single_set_remove_by_fd(&aset,aset.arr[i].fd);
					strncpy(msg_from_app.content, "", MSG_CONTENT_SIZE);
					msg_from_app.status = DEREGISTER;
				}else if(bulk < 0)
					ERR("read:");
				else{
					puts("new message from app:");
					puts(message);
					strncpy(msg_from_app.content, message, MSG_CONTENT_SIZE);
					msg_from_app.status = REGULAR;
				}
				wrapped_message_to_str(message,msg_from_app,MSG_SIZE);
				
				puts("sending:");
				puts(message);
				if( (bulk = bulk_write(serverfd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
					puts("server stopped working");
					goto end;
				}
				else if(bulk < 0)
					ERR("write:");
				if(select_all_number<=0) goto start;
			}
		}
	}
end:
	puts("thread ending");
	if(TEMP_FAILURE_RETRY(close(serverfd))<0)
		ERR("close:");
	if(TEMP_FAILURE_RETRY(close(sockfd))<0)
		ERR("close:");
	for (i = 0; i < aset.n; i++){
		if(aset.arr[i].is_empty)
			continue;
		if(TEMP_FAILURE_RETRY(close(aset.arr[i].fd))<0)
			ERR("close:");
	}
	
	return NULL;
}


int main(int argc , char *argv[]){
	char name[NAME_LENGTH];
	int n = 1, i;//i to indeksator
	thread_data td[n];
	int fdmax = 2, port[]={5578,5571};
	
	strcpy(name,"aleksander");
	
	sem_init(&mutex, 0, 1);
	
	pthread_t thread[n];

	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGPIPE:");
	puts("before init");
	for (i = 0; i < n; i++){
		int_to_str(td[i].local_port_number,PORT_NUMBER_LENGTH,port[i]);
		int_to_str(td[i].remote_port_number,PORT_NUMBER_LENGTH,USER_PORT);
		strncpy(td[i].service_name,"multicast",SERVICE_NAME_LENGTH);

		td[i].client_name = name;
		td[i].fdmax = &fdmax;//fdmaxa zawsze lockować
		
		if(pthread_create( &thread[i], NULL, thread_handler, (void*)&td[i]))
			ERR("pthread:");
	}
	puts("after init");	

	for (i = 0; i < n; i++)
		pthread_join(thread[i], NULL);

    exit(EXIT_SUCCESS);
}


