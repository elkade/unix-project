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


//void *thread_handler( void *ptr ){
	//int *pipeout = (int*)ptr, n = 16;
	
	//char buf[n];
	//bzero(buf,n);
	
	//fd_set curfds, fds;
	
	//FD_ZERO(&fds);
	//FD_SET(*pipeout,&fds);
	
	//while(true){
		//curfds = fds;
		//if (select(*(pipeout + 1), &curfds, NULL, NULL, NULL)<0)
			//ERR("select:");
		//if(FD_ISSET(*pipeout,&curfds)){
			//if(bulk_read(*pipeout,buf,16)<0)
				//ERR("read:");
			//printf("%s\n",buf);
		//}
	//}
	//if(TEMP_FAILURE_RETRY(close(*pipeout))<0)
		//ERR("close:");
	//return NULL;
//}
//main!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
int main(int argc, char** argv){
	if(sethandler(SIG_IGN,SIGPIPE))
	    ERR("Setting SIGPIPE:");
	user_listen();
	return 0;
	//admin_listen();
	//return 0;
	//char in[NAME_LENGTH];
	//bzero(in,NAME_LENGTH);
	//client *c;
	//fgets(in,NAME_LENGTH, stdin);
	//trim(in,strlen(in));
	//c = new_client(in,true,SUBSCRIPTION,123,22.93);
	//db_insert_client(c);
	//handle_client(in);
	////delete_client(c);
	//return 0;
	//if(auth(in, USER)==0){
		//puts("OK");
		//handle_client(in);
	//}
	//else
		//puts("failed");
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

int admin_info(char* response){
	puts("admin_info");
	strcat(response,"\ninfo\treturns the server functionality\n");
	strcat(response,"list_clients\treturns the list of all clients\n");
	strcat(response,"add_client <>\tdo zmiany\n");
	strcat(response,"delete_client <name>\tremoves client from the list\n");
	strcat(response,"get_data_counters\treturns counter values for all clients\n");
	strcat(response,"boost_prepaid <name> <amount>\tincreases the prepaid amount for specific client\n");
	strcat(response,"start_new_sub_period\treturns amounts and resets them for all clients with subscription\n");
	strcat(response,"lock_client <name>\tlocks/unlocks the client\n");
	strcat(response,"delete_service\tremoves the service from the list\n");
	strcat(response,"add_service\tdo zmiany\n");
	strcat(response,"list_services\tgets the list of all services\n");
	return 0;
}
int admin_list_clients(char* response){
	puts("admin_list_clients");//uwaga, bo liczba klientów może przekroczyć pojemmność wiadomości
	char buf[CLIENT_DISPLAY_SIZE];
	
	int i,  n = db_count_clients();
	client clist[n];
	db_select_clients(clist,n);
	
	
	bzero(buf,CLIENT_DISPLAY_SIZE);
	for (i = 0; i < n; i++){
		display_client(&clist[i],buf,ALL);
		strncat(response,buf,CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
	return 0;
}
int admin_add_client(char* name,/* char* plan, char* capacity, */char* response){
	puts("admin_add_client");
	return 0;
}
int admin_delete_client(char* arg, char* response){
	puts("admin_delete_client");
	printf("arg: %s\n\n",arg);
	if(db_delete_client(arg)==0){
		strcat(response,"client removed\n");
		return 0;
	}
	else{
		strcat(response,"delete failed\n");
		return 1;
	}
}
int admin_get_data_counters(char* response){
	puts("admin_get_data_counters");//uwaga, bo liczba klientów może przekroczyć pojemmność wiadomości
	char buf[CLIENT_DISPLAY_SIZE];
	
	int i,  n = db_count_clients();
	client clist[n];
	db_select_clients(clist,n);
	
	
	bzero(buf,CLIENT_DISPLAY_SIZE);
	for (i = 0; i < n; i++){
		display_client(&clist[i],buf, COUNTERS);
		strncat(response,buf,CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
	return 0;
}
int admin_start_new_sub_period(char* response){
	puts("admin_start_new_sub_period");
	char buf[CLIENT_DISPLAY_SIZE];
	
	int i,  n = db_count_clients();
	client clist[n];
	db_select_clients(clist,n);
	
	
	bzero(buf,CLIENT_DISPLAY_SIZE);
	for (i = 0; i < n; i++){
		if(clist[i].tariff_plan==SUBSCRIPTION){
			display_client(&clist[i],buf, COUNTERS);
			clist[i].amount = 0;
			db_update_client(clist[i].name,&clist[i]);
			strncat(response,buf,CLIENT_DISPLAY_SIZE);
			bzero(buf,CLIENT_DISPLAY_SIZE);
		}
	}
	return 0;
}
int admin_boost_prepaid(char* name, char* amount , char* response){
	puts("admin_boost_prepaid");
	client c;
	db_select_client(name,&c);
	c.amount+=atof(amount);
	db_update_client(c.name,&c);
	return 0;
}
int admin_lock_client(char* name, char* response){
	puts("admin_lock_client");
	client c;
	db_select_client(name,&c);
	c.is_active=!c.is_active;
	db_update_client(c.name,&c);
	return 0;
}
int admin_delete_service(char* name, char* response){
	puts("admin_delete_service");
	return 0;
}
int admin_add_service(char* name, char* host, char* port, char* response){
	puts("admin_add_service");
	return 0;
}
int admin_list_services(char* response){
	puts("admin_list_services");
	return 0;
}
int admin_handle_message(char* msg, char* response, bool is_authenticated){
	int result = 0;
	bzero(response,MSG_SIZE);
	char *cmd;
	char** args;
	puts("admin_handle_message");
	if(is_authenticated)
		puts("authenticated");
	
	args = str_split(msg, ' ');//to nie jest dobrze bo nie sprawdzam, czy nie przyszedł syf
	if(args==NULL){
		puts("unhandled message");
		strcpy(response,"unhandled message");
		return 1;
	}
	cmd = args[0];
	
	if(!is_authenticated){
		puts("authentication failed");
		strcpy(response,"authentication failed");
		result = 1;
	}else if(cmd==NULL){
		puts("wrong syntax");
		strcpy(response,"wrong syntax");
		result = 1;
	}
	else if(strcmp(cmd,"info")==0)
		result = admin_info(response);
	else if(strcmp(cmd,"list_clients")==0)
		result = admin_list_clients(response);
	else if(strcmp(cmd,"add_client")==0)
		result = admin_add_client(args[1], response);
	else if(strcmp(cmd,"delete_client")==0)
		result = admin_delete_client(args[1], response);
	else if(strcmp(cmd,"get_data_counters")==0)
		result = admin_get_data_counters(response);
	else if(strcmp(cmd,"start_new_sub_period")==0)
		result = admin_start_new_sub_period(response);
	else if(strcmp(cmd,"boost_prepaid")==0)
		result = admin_boost_prepaid(args[1], args[2], response);
	else if(strcmp(cmd,"lock_client")==0)
		result = admin_lock_client(args[1], response);
	else if(strcmp(cmd,"list_services")==0)
		result = admin_list_services(response);
	else if(strcmp(cmd,"delete_service")==0)
		result = admin_delete_service(args[1],response);
	else if(strcmp(cmd,"add_service")==0)
		result = admin_add_service(args[1], args[2], args[3],response);
	else{
		puts("unhandled message");
		strcpy(response,"unhandled message");
		result = 1;
	}
	free(args);
	return result;
}

//typedef struct service_fd{
	//int fd;
	//service s;
//}service_fd;

//typedef struct connection{
	//int fd;
	//client c;
	//service_fd slist[100];
	//int n;
//}connection;

void user_listen(){
	char message[MSG_SIZE];
	int clifd = create_socket(USER_PORT), fdmax, i, j, k;
	int admin_listen_socket = create_socket(ADMIN_PORT), admfd = -1;
	fd_set curfds, allfds;
	bool is_authenticated = false;
	fdmax = clifd;
	
	FD_ZERO(&allfds);
	
	FD_SET(clifd,&allfds);
	FD_SET(admin_listen_socket,&allfds);

	int m = 128;
	int n = 128;
	client_set cset;
	client_set_init(&cset,m);
	
	int select_all_number;
	
	//struct timeval tt = {0,0};
	while(true){//BRAKUJE OBSŁUGI PRZEPEŁNIENIA SETÓW I ROZŁĄCZENIA SIĘ SERWISU I JAK SERWIS NIE DZIAŁA, A KLIENT CHCE SIĘ POŁĄCZYĆ
start:
		puts("czekam");
		curfds = allfds;
		if ((select_all_number = select(fdmax + 1, &curfds, NULL, NULL, NULL))<0) perror("select");//jest coś ale nie wiadomo co
		printf("dostałem %d rzeczy\n",select_all_number);
		puts("fd:");
		for (i = 0; i < fdmax+1; i++)
		{
			if(FD_ISSET(i,&curfds)){
				printf("%d\n",i);
			}
		}
		sleep(1);
		//getchar();
		//curfds = clifds;
		//if (select(fdmax + 1, &curfds, NULL, NULL, NULL)<0) perror("select");//sprawdzam sockety klienckie
		
		if(FD_ISSET(admin_listen_socket,&curfds)){
			select_all_number--;
			printf("%s\n","admin chce się połączyć");
			admfd = addnewfd(admin_listen_socket,&allfds,&fdmax);
			printf("przydzialem adminowi: %d\n",admfd);
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
			puts("otrzymano");
			puts(message);
			trim(message,strlen(message));
			puts(message);
			char response[MSG_SIZE];
			if(!is_authenticated){
				if((is_authenticated = (auth(message))))
					strcpy(response,"authentication successful");
				else
					strcpy(response,"authentication failed");//coś jest nie tak, bo mimo, że idzie admin, to pisze failed
			}
			else
				admin_handle_message(message, response, is_authenticated);
			puts(response);
			puts("wysyłam do admina");
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
			printf("%s\n","nowy klient");
			
			int fd = addnewfd(clifd,&allfds,&fdmax);
			client_set_add_by_fd(&cset,fd, n);
			printf("przydzialem: %d\n",fd);
		}
		if(select_all_number<=0) goto start;
		for (i = 0; i < cset.n; i++){//dla każdego zarejestrowanego klienta
			if(cset.arr[i].is_empty)
				continue;
			client_set_elem *celem = &cset.arr[i];
			printf("sprawdzam klienta %d o fd %d\n",i,cset.arr[i].fd);
			if(FD_ISSET(cset.arr[i].fd,&curfds)){
				puts("znalazłem fd\n");
				//trzeba sprawdzić, czy klient jest w bazie
				select_all_number--;
				if( bulk_read(celem->fd, message, MSG_SIZE) < 0){
					puts("błąd odczytu");
					for (k = 0; k < celem->sset.n; k++){
						if(celem->sset.arr[k].is_empty)
							continue;
						disconnect(celem->sset.arr[k].fd, &allfds);
						service_set_remove_by_fd(&celem->sset,celem->sset.arr[k].fd);
					}
					disconnect(celem->fd, &allfds);
					client_set_remove_by_fd(&cset,celem->fd);
					goto start;//po disconnectcie trzeba wyjść z ifa
				}
				puts("otrzymuję1");
				printf("%s\n",message);

				wrapped_message msg_from_client;
				str_to_wrapped_message(message,&msg_from_client,MSG_SIZE);
				if(msg_from_client.status==DEREGISTER)
					puts("\n\nDEREGISTER\n\n");
				
				printf("\n%s\n",msg_from_client.service_name);
				printf("%s\n",msg_from_client.client_name);
				printf("%s\n",msg_from_client.content);
				printf("%d\n\n",msg_from_client.status);
				
				puts("rejestracja");
				client c;
				if(db_select_client(msg_from_client.client_name,&c)!=0){puts("nie ma klienta w bazie");}//nie ma w bazie
				//porównywanie w BAZIE POPRAWIĆ BO BIERZE DŁUGOŚĆ TYLKO TEGO OK
				else{
					puts("klient jest w bazie. n=0 więc rejestruję serwis");
					strcpy(celem->name,c.name);
					bool is_found = false;
					//TRZEBA SPRAWDZIĆ CZY W BAZIE MAMY W OGÓLE TAKI SERWIS
					for (j = 0; j < celem->sset.n; j++){//POPRAWIC
						if(celem->sset.arr[j].is_empty)
							continue;
						service_set_elem *selem = &celem->sset.arr[j];
						if(strcmp(selem->name, msg_from_client.service_name)==0){
							is_found = true;
							printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
							if( bulk_write(selem->fd, msg_from_client.content, MSG_CONTENT_SIZE) < 0){
								puts("błąd wysyłania");
								disconnect(selem->fd, &allfds);
								client_set_remove_by_fd(&cset,selem->fd);
								goto start;
							}
							puts("wysyłam1");
							printf("%s\n",msg_from_client.content);
						}
					}
					if(!is_found){
						printf("%s\n",message);
						service s;
						//trzeba sprawdzić, czy taki serwis jest w ogóle w bazie
						if(db_select_service(msg_from_client.service_name,&s)!=0){puts("nie ma serwisu w bazie");}//nie znaleziono - trzeba zamknąć fd
						else{
							puts("rejestruje serwis");
							int fd = create_socket_client(s.host_name,atoi(s.port_number));
							
							service_set_add_by_fd(&celem->sset, fd);
							
							service_set_update_by_fd(&celem->sset, fd, s.name);
							
							addnewfd_listen(fd,&allfds,&fdmax);
							printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
							if( bulk_write(fd, msg_from_client.content, MSG_CONTENT_SIZE) < 0){
								puts("write");
								disconnect(fd,&allfds);//tu i trochę wyżej ogarnąć rozłączenie się serwisu
								client_set_remove_by_fd(&cset,fd);
								goto start;
							}
							puts("wysłałem");
						}
					}
				}
			}
			if(select_all_number<=0) goto start;
			for (j = 0; j < cset.arr[i].sset.n; j++){//dla każdego serwisu
				if(cset.arr[i].sset.arr[j].is_empty)
					continue;
				service_set_elem *selem = &cset.arr[i].sset.arr[j];
				printf("sprawdzam serwis %d\to fd %d\n",j,selem->fd);
				if(FD_ISSET(selem->fd,&curfds)){//mamy coś od serwisu
					select_all_number--;
					if( bulk_read(selem->fd, message, MSG_SIZE) < 0){
						puts("błąd odczytu");
						disconnect(selem->fd,&allfds);
						client_set_remove_by_fd(&cset,selem->fd);
						goto start;
					}
					puts("otrzymuję2");
					printf("%s\n",message);
					wrapped_message msg_from_service;
					bzero(msg_from_service.content,MSG_CONTENT_SIZE);
					strcpy(msg_from_service.content,message);
					
					bzero(msg_from_service.service_name,SERVICE_NAME_LENGTH);
					strcpy(msg_from_service.service_name,selem->name);
					
					bzero(msg_from_service.client_name,NAME_LENGTH);
					strcpy(msg_from_service.client_name,celem->name);
					msg_from_service.status = REGULAR;
					bzero(message,MSG_SIZE);
					wrapped_message_to_str(message,msg_from_service,MSG_SIZE);
					puts("wysyłam2");
					printf("%s\n",message);
					if( bulk_write(celem->fd, message, MSG_SIZE) < 0){
						puts("błąd wysyłania");
						for (k = 0; k < celem->sset.n; k++){
							if(celem->sset.arr[k].is_empty)
								continue;
							printf("odłączam serwis. k=%d\n",k);
							disconnect(celem->sset.arr[k].fd, &allfds);
							service_set_remove_by_fd(&celem->sset,celem->sset.arr[k].fd);
						}
						puts("odłączam klienta");
						disconnect(celem->fd,&allfds);
						client_set_remove_by_fd(&cset,celem->fd);
						goto start;
					}
				}
				if(select_all_number<=0) goto start;
			}
			if(select_all_number<=0) goto start;
		}
	}

}
