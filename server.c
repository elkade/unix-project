#include "header.h"
#include "client.h"
#include "db.h"
#include "service.h"
#include "wrapped_message.h"
#include "set.h"
#include <pthread.h>
volatile sig_atomic_t stop = 0;


void admin_listen();
void user_listen();
void connect_to_services();

typedef enum role{
	USER,
	OWNER
}role;

bool auth(char* input){
	char owner_name[NAME_LENGTH] = "admin";
	if(strncmp(input, owner_name, strlen(owner_name))==0)
		return true;
	return false;
}
void handle_client(char* name){
	int i,  n = db_count_clients();
	client c[n];
	db_select_clients(c,n);
	char buf[CLIENT_DISPLAY_SIZE];
	for (i = 0; i < n; i++){
		display_client(&c[i],buf,ALL);
		puts(buf);
	}
	
}
int main(int argc, char** argv){
	if(sethandler(SIG_IGN,SIGPIPE))
	    ERR("Setting SIGPIPE:");
	user_listen();
	return 0;
	exit(EXIT_SUCCESS);
}


void close_all(fd_set fds, int fdmax){
	int i;
	for (i = 0; i<fdmax + 1; i++)
		if (FD_ISSET(i, &fds)){
			shutdown(i,SHUT_RDWR);
			if(TEMP_FAILURE_RETRY(close(i))<0)ERR("close:");
		}
}

#include "admin.h"

typedef struct thread_data{
	char first_message[MSG_SIZE];
	int *fdmax;
	int client_fd;
}thread_data;

int send_service_disconnected(int client_fd, char* service_name){
	int bulk;
	char message[MSG_SIZE];
	bzero(message,MSG_SIZE);
	wrapped_message msg;
	bzero(msg.content,MSG_CONTENT_SIZE);
	strcpy(msg.content,"");
	
	bzero(msg.service_name,SERVICE_NAME_LENGTH);
	strcpy(msg.service_name,service_name);
	
	bzero(msg.client_name,NAME_LENGTH);
	strcpy(msg.client_name,"");
	msg.status = SERVICE_DISCONNECTED;
	bzero(message,MSG_SIZE);
	wrapped_message_to_str(message,msg,MSG_SIZE);
	puts("wysyłam2");
	printf("%s\n",message);
	if((bulk = bulk_write(client_fd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
		puts("błąd wysyłania że disconnected");
		return 1;
	}else if(bulk<0)ERR("write");
	return 0;
}

void *admin_thread_handler( void *ptr ){
	//bool is_authenticated = false;
	return NULL;
}

void *user_thread_handler( void *ptr ){
	puts("nowy wątek");
	thread_data td = *((thread_data*) ptr);
	puts(td.first_message);
	int client_fd = td.client_fd, bulk;
	char message[MSG_SIZE], name[NAME_LENGTH];
	bzero(message,MSG_SIZE);
	bzero(name,NAME_LENGTH);
	int *fdmax = td.fdmax, i, j, k;
	fd_set curfds, allfds;
	
	
	FD_ZERO(&allfds);
	
	addnewfd_listen(td.client_fd,&allfds,fdmax);
	
	int m = 128;//ile serwisów
	int n = 128;//ile aplikacji do serwisu
	double_set servset;//many services -> many apps
	double_set_init(&servset,m);
	
	//single_set sset;
	//single_set_init(&sset,n);
	puts("czekam na register");
	wrapped_message msg_from_client;
	str_to_wrapped_message(td.first_message,&msg_from_client,MSG_SIZE);
	if(msg_from_client.status!=REGISTER){
		//send_wrong_syntax();DOKOŃCZYĆ
		puts("powiniem być register");
		return NULL;
	}
	strcpy(name,msg_from_client.client_name);
	puts(name);



	int select_all_number;
	
	while(true){//BRAKUJE OBSŁUGI PRZEPEŁNIENIA SETÓW I ROZŁĄCZENIA SIĘ SERWISU I JAK SERWIS NIE DZIAŁA, A KLIENT CHCE SIĘ POŁĄCZYĆ
start:
	puts("czekam");
	curfds = allfds;//POPRAWIĆ fdmax NA WSKAŹNIK
	if ((select_all_number = select(*fdmax + 1, &curfds, NULL, NULL, NULL))<0) perror("select");//jest coś ale nie wiadomo co
	printf("dostałem %d rzeczy\n",select_all_number);
	puts("fd:");
	for (i = 0; i < *fdmax+1; i++)
	{
		if(FD_ISSET(i,&curfds)){
			printf("%d\n",i);
		}
	}
	sleep(1);
	if(FD_ISSET(client_fd,&curfds)){
		puts("dostałem coś od klienta");
		if((bulk = bulk_read(client_fd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
			puts("błąd odczytu");
			for (k = 0; k < servset.n; k++){
				if(servset.arr[k].is_empty)
					continue;
					//DLA KAŻDEJ APLIKACJI
				//disconnect(servset.arr[k].fd, &allfds);
				//double_set_remove_by_fd(&servset,servset.arr[k].fd);
			}
			disconnect(client_fd, &allfds);
			//client_set_remove_by_fd(&cset,celem->fd);
			goto end;//po disconnectcie trzeba wyjść z ifa
		}else if(bulk<0) ERR("read:");
		puts("otrzymuję1");
		
		
		
		printf("%s\n",message);

		wrapped_message msg_from_client;
		str_to_wrapped_message(message,&msg_from_client,MSG_SIZE);
		if(msg_from_client.status==DEREGISTER){
			puts("\n\nDEREGISTER\n\n");
			double_set_elem *dsm;
			single_set_elem *ssm;
			
			double_set_select_by_name(&servset,msg_from_client.service_name, &dsm);
			single_set_select_by_name(&dsm->sset,msg_from_client.app_name,&ssm);
			
			shutdown(ssm->fd,SHUT_RDWR);
			FD_CLR(ssm->fd, &allfds);
			if(TEMP_FAILURE_RETRY(close(ssm->fd))<0)ERR("close:");
			single_set_remove_by_fd(&dsm->sset, ssm->fd);
			if(single_set_count(&dsm->sset)==0)
				double_set_remove_by_name(&servset, msg_from_client.service_name);
			goto start;
		}
		else if(msg_from_client.status==REGISTER)
			goto start;
		printf("\n%s\n",msg_from_client.service_name);
		printf("%s\n",msg_from_client.client_name);
		printf("%s\n",msg_from_client.app_name);
		printf("%s\n",msg_from_client.content);
		printf("%d\n\n",msg_from_client.status);
		
		
		puts("rejestracja");
		client c;
		if(db_select_client(msg_from_client.client_name,&c)!=0){puts("nie ma klienta w bazie");}//nie ma w bazie
		//porównywanie w BAZIE POPRAWIĆ BO BIERZE DŁUGOŚĆ TYLKO TEGO OK
		else{
			puts("klient jest w bazie. rejestruję serwis");
			service s;
			if(db_select_service(msg_from_client.service_name,&s)!=0){
				puts("nie ma serwisu w bazie");
				send_service_disconnected(client_fd, msg_from_client.service_name);
				goto start;
			}
			//strcpy(name,c.name);
			bool is_found_service = false;
			bool is_found_app = false;
			double_set_elem* servelem;
			for (j = 0; j < servset.n; j++){
				if(servset.arr[j].is_empty)
					continue;
				servelem = &servset.arr[j];
				if(strcmp(servelem->name, msg_from_client.service_name)==0){
					puts("mam service");
					is_found_service = true;
					for (k = 0; k < servset.arr[j].sset.n; k++)	{
						if(servset.arr[j].is_empty)
							continue;
						single_set_elem appelem = servset.arr[j].sset.arr[k];
						if(strcmp(appelem.name, msg_from_client.app_name)==0){
							puts("mam app");
							is_found_app = true;
							printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
							if((bulk = bulk_write(appelem.fd, msg_from_client.content, MSG_CONTENT_SIZE) ) == 0 ||
									(bulk < 0 && errno == EPIPE)){
								puts("błąd wysyłania");
								disconnect(appelem.fd, &allfds);
								send_service_disconnected(client_fd, msg_from_client.service_name);
								goto start;
							}else if(bulk<0) ERR("write:");
							puts("wysyłam1");
							printf("%s\n",msg_from_client.content);
							//break;//znaleziono
						}
					}
					//break;//znaleziono
				}
			}
			if(!is_found_service){
				puts("rejestruję serwis");
				double_set_add_by_name(&servset, s.name, n);
				double_set_select_by_name(&servset, s.name,&servelem);
			}
			puts(servelem->name);
			if(!is_found_app){
				printf("%s\n",message);
				
				int fd = create_socket_client(s.host_name,atoi(s.port_number));
				printf("rejestruję aplikację fd: %d\n",fd);
				single_set_add_by_fd(&servelem->sset, fd);
				
				single_set_update_by_fd(&servelem->sset, fd, msg_from_client.app_name);
				
				addnewfd_listen(fd,&allfds,fdmax);
				
				printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
				if((bulk = bulk_write(fd, msg_from_client.content, MSG_CONTENT_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
					puts("write");
					disconnect(fd, &allfds);
					send_service_disconnected(client_fd, msg_from_client.service_name);
					goto start;
				}
				else if(bulk<0) ERR("write:");
				puts("wysłałem");
			}
		}
	}else{
		puts("dostałem coś od serwisu");
		if(select_all_number<=0) goto start;
		puts("nie wróciłem jeszcze na start - szukam serwisu");
		for (j = 0; j < servset.n; j++){//dla każdego serwisu
			//puts("pętla");
			if(servset.arr[j].is_empty)
				continue;
			double_set_elem servelem = servset.arr[j];
			printf("sprawdzam serwis %d\n",j);
			for (k = 0; k < servelem.sset.n; k++)	{//dlla każdej aplikacji
				//puts("pętla");
				if(servelem.sset.arr[k].is_empty)
					continue;
				single_set_elem appelem = servelem.sset.arr[k];
				printf("sprawdzam aplikację %d\to fd %d\n",k,appelem.fd);
			if(FD_ISSET(appelem.fd,&curfds)){//mamy coś od serwisu
				puts("mam coś od tego serwisu");
				select_all_number--;
				if((bulk = bulk_read(appelem.fd, message, MSG_SIZE) ) == 0 || (bulk < 0 && errno == EPIPE)){
					puts("błąd odczytu");
					disconnect(appelem.fd, &allfds);
					send_service_disconnected(client_fd, msg_from_client.service_name);
					goto start;
				}else if(bulk < 0)
					ERR("read:");
				puts("otrzymuję2");
				printf("%s\n",message);
				wrapped_message msg_from_service;
				bzero(msg_from_service.content,MSG_CONTENT_SIZE);
				strcpy(msg_from_service.content,message);
				
				bzero(msg_from_service.service_name,SERVICE_NAME_LENGTH);
				strcpy(msg_from_service.service_name,servelem.name);
				
				bzero(msg_from_service.app_name,APP_NAME_LENGTH);
				strcpy(msg_from_service.app_name,appelem.name);
				
				bzero(msg_from_service.client_name,NAME_LENGTH);
				strcpy(msg_from_service.client_name,name);
				msg_from_service.status = REGULAR;
				bzero(message,MSG_SIZE);
				wrapped_message_to_str(message,msg_from_service,MSG_SIZE);
				puts("wysyłam2");
				printf("%s\n",message);
				if( (bulk = bulk_write(client_fd, message, MSG_SIZE) ) == 0 || (bulk < 0 /*&& errno == EPIPE*/)){
					puts("błąd wysyłania");
					for (k = 0; k < servset.n; k++){
						if(servset.arr[k].is_empty)
							continue;
						printf("odłączam serwis. k=%d\n",k);
						//DLA KAŻDEJ APLIKACJI
						//disconnect(servset.arr[k].fd, &allfds);
						//double_set_remove_by_fd(&servset,servset.arr[k].fd);
					}
					puts("odłączam klienta");
					disconnect(client_fd,&allfds);
					//client_set_remove_by_fd(&cset,celem->fd);
					goto end;
				}else if(bulk<0)ERR("write:");//TRZEBA JESZCZE INNE BŁĘDY OBSŁUGIWAĆ
				if(select_all_number<=0) goto start;
			}
			if(select_all_number<=0) goto start;
			}
		}
		if(select_all_number<=0) goto start;
	}
	}
end:
	puts("closing thread...");
	return NULL;
}

void user_listen(){
	int thread_num = 0;
	pthread_t thread[256];//DO POPRAWIENIA - raczej na jakiś set
	thread_data td[256];

	
	char message[MSG_SIZE];
	int clifd = create_socket(USER_PORT), fdmax, i;
	int admin_listen_socket = create_socket(ADMIN_PORT), admfd = -1;
	fd_set curfds, allfds;
	bool is_authenticated = false;
	fdmax = admin_listen_socket;
	
	FD_ZERO(&allfds);
	
	FD_SET(clifd,&allfds);
	FD_SET(admin_listen_socket,&allfds);

	int m = 128;
	single_set cset;
	single_set_init(&cset,m);
	
	int select_all_number;
	
	//struct timeval tt = {0,0};
	while(true){//BRAKUJE OBSŁUGI PRZEPEŁNIENIA SETÓW I ROZŁĄCZENIA SIĘ SERWISU I JAK SERWIS NIE DZIAŁA, A KLIENT CHCE SIĘ POŁĄCZYĆ
start:
		puts("główny: czekam");
		curfds = allfds;//POPRAWIĆ fdmax NA WSKAŹNIK
		if ((select_all_number = select(fdmax + 1, &curfds, NULL, NULL, NULL))<0) perror("select");//jest coś ale nie wiadomo co
		printf("główny: dostałem %d rzeczy\n",select_all_number);
		puts("główny: fd:");
		for (i = 0; i < fdmax+1; i++)
		{
			if(FD_ISSET(i,&curfds)){
				printf("główny: %d\n",i);
			}
		}
		sleep(1);
		if(FD_ISSET(admin_listen_socket,&curfds)){
			select_all_number--;
			printf("główny: %s\n","admin chce się połączyć");
			admfd = addnewfd(admin_listen_socket,&allfds,&fdmax);
			printf("główny: przydzielam adminowi: %d\n",admfd);
			disconnect(admin_listen_socket,&allfds);
		}
		if(select_all_number<=0) goto start;
		
		
		if(FD_ISSET(admfd, &curfds)){
			select_all_number--;
			bzero(message,MSG_SIZE);
			if(bulk_read(admfd,message,MSG_SIZE)<0){
				disconnect(admfd,&allfds);
				admin_listen_socket = create_socket(ADMIN_PORT);
				addnewfd_listen(admin_listen_socket,&allfds,&fdmax);
				is_authenticated = false;
				goto start;
			}
			puts("główny: otrzymano");
			puts(message);
			trim(message,strlen(message));
			puts(message);
			char response[MSG_SIZE];
			if(!is_authenticated){
				if((is_authenticated = (auth(message))))
					strcpy(response,"authentication successful");
				else
					strcpy(response,"authentication failed");
			}
			else
				admin_handle_message(message, response, is_authenticated);
			puts(response);
			puts("główny: wysyłam do admina");
			if(bulk_write(admfd,response,MSG_SIZE)<0){
				disconnect(admfd,&allfds);
				admin_listen_socket = create_socket(ADMIN_PORT);
				addnewfd_listen(admin_listen_socket,&allfds,&fdmax);
				is_authenticated = false;
				goto start;
			}
		}
		if(select_all_number<=0) goto start;
		
		if(FD_ISSET(clifd,&curfds)){
			select_all_number--;
			printf("główny: %s\n","nowy klient");
			
			int fd = addnewfd(clifd,&allfds,&fdmax);
			single_set_add_by_fd(&cset,fd);
			printf("główny: przydzialem: %d\n",fd);
		}
		if(select_all_number<=0) goto start;

		for (i = 0; i < cset.n; i++){//dla każdego zarejestrowanego klienta
			if(cset.arr[i].is_empty)
				continue;
			single_set_elem *celem = &cset.arr[i];
			printf("główny: sprawdzam klienta %d o fd %d\n",i,cset.arr[i].fd);
			if(FD_ISSET(celem->fd,&curfds)){
				puts("znalazłem fd\n");
				//trzeba sprawdzić, czy klient jest w bazie
				select_all_number--;
				if( bulk_read(celem->fd, message, MSG_SIZE) < 0){
					puts("główny: błąd odczytu");
					disconnect(celem->fd, &allfds);
					single_set_remove_by_fd(&cset,celem->fd);
					goto start;//po disconnectcie trzeba wyjść z ifa
				}
				puts("główny: otrzymuję1");
				printf("główny: %s\n",message);

				wrapped_message msg_from_client;
				str_to_wrapped_message(message,&msg_from_client,MSG_SIZE);
				
				printf("główny: \n%s\n",msg_from_client.service_name);
				printf("główny: %s\n",msg_from_client.client_name);
				printf("główny: %s\n",msg_from_client.content);
				printf("główny: %d\n\n",msg_from_client.status);
				
				//mam już wiadomość. Trzeba ją wysłać do odpowiedniego wątku. ZROBIĆ NOWY WĄTEK
				//jeżeli nie ma takiego klienta, to ogarnie to wątek, żeby nie blokować głównego
				
				bzero(td[thread_num].first_message, MSG_SIZE);
				strncpy(td[thread_num].first_message, message,MSG_SIZE);
				td[thread_num].fdmax = &fdmax;
				td[thread_num].client_fd = cset.arr[i].fd;
				if(pthread_create( &thread[thread_num], NULL, user_thread_handler, (void*)&td[thread_num]))
					ERR("pthread:");
				
				FD_CLR(celem->fd,&allfds);
				single_set_remove_by_fd(&cset,celem->fd);
				thread_num++;
			}
		}
}


}
