#include "header.h"
#include "client.h"
#include "db.h"

volatile sig_atomic_t stop = 0;


void admin_listen();
void user_listen();

typedef enum role{
	USER,
	OWNER
}role;
typedef struct service{
	char name[16];
	char host[16];
	uint16_t port;
}service;

int auth(char* input, role who){
	char owner_name[NAME_LENGTH] = "admin";
	client c;
	switch (who)
	{
		case USER:
			//sprawdzić, czy input jest na liście klientów
			if(db_select_client(input, &c)==0){
					//klient istnieje
					return 0;
				}
			break;
		case OWNER:
			//sprawdzić, czy input to nazwa właściciela
			if(strncmp(input, owner_name, strlen(owner_name))==0)
				return 0;
			break;
	}
	return 1;
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
	admin_listen();
	return 0;
	char in[NAME_LENGTH];
	bzero(in,NAME_LENGTH);
	client *c;
	fgets(in,NAME_LENGTH, stdin);
	trim(in,strlen(in));
	c = new_client(in,true,SUBSCRIPTION,123,22.93);
	db_insert_client(c);
	handle_client(in);
	//delete_client(c);
	return 0;
	if(auth(in, USER)==0){
		puts("OK");
		handle_client(in);
	}
	else
		puts("failed");
	exit(EXIT_SUCCESS);
}

char** str_split(char* a_str, const char a_delim)//to nie jest zbyt dobra funkcja
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
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

int admin_process(int sockfd, int fdmax){
	char buf[MSG_SIZE];
	fd_set afds;
	int afd;//file descriptor admina
	bool is_authenticated = false;
	
	if ((afd = TEMP_FAILURE_RETRY(accept(sockfd, NULL, NULL))) == -1)
		ERR("Cannot accept connection");
	FD_SET(afd, &afds);
	puts("odebrano połączenie");
	//przyszło połączenie od admina
	while(true){//zmienić!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (select(fdmax + 1, &afds, NULL, NULL, NULL) == -1){
			if (errno != EINTR) ERR("Cannot select");
			else if(stop){
				close_all(afds, fdmax);
				ERR("SIGINT");
			}
		}//nieeeeeeeeeeeeeeeee jeeeeeeeeeeeeeest dddddddddddddddobrze curfds
		puts("odebrano wiadomość");
				//słucham państwa
		if(bulk_read(afd,buf,MSG_SIZE)<0){
			//ERR("read");
			puts("connection lost");
			return 1;
		}
		puts(buf);
		trim(buf,strlen(buf));
		puts(buf);
		char response[MSG_SIZE];
		if(!is_authenticated){
			if((is_authenticated = (auth(buf,OWNER)==0)))
				strcpy(response,"authentication successful");
			else
				strcpy(response,"authentication failed");//coś jest nie tak, bo mimo, że idzie admin, to pisze failed
		}
		else
			admin_handle_message(buf, response, is_authenticated);
		puts(response);
		if(bulk_write(afd,response,MSG_SIZE)<0){
			//ERR("write");
			puts("connection lost");
			return 1;
		}
	}
}

void admin_listen(){
	int sockfd = create_socket(ADMIN_PORT);
	int fdmax = sockfd + 1;
	fd_set sfds;
	FD_ZERO(&sfds);
	FD_SET(sockfd, &sfds);
	while (true){
		puts("czekam...");
		if (select(fdmax + 1, &sfds, NULL, NULL, NULL) == -1){
			if (errno != EINTR) ERR("Cannot select");
			else if(stop){
				close_all(sfds, fdmax);
				ERR("SIGINT");
			}
		}
		if (FD_ISSET(sockfd, &sfds))
			admin_process(sockfd, fdmax);
	}
}


int user_process(int sockfd, int fdmax){
	char buf[MSG_SIZE];
	fd_set afds, curfds;
	int afd, i, j;//file descriptor admina
	
	if ((afd = TEMP_FAILURE_RETRY(accept(sockfd, NULL, NULL))) == -1)
		ERR("Cannot accept connection");
	FD_SET(afd, &afds);
	puts("odebrano połączenie");
	//przyszło połączenie od admina
	
	struct timeval tt = {0,0};
	for(i=0;;i++){//zmienić!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		snprintf(buf,16,"%d\n",i);
		puts(buf);
		//if(bulk_write(afd,buf,MSG_SIZE)<0){
			//ERR("write");
		//	puts("connection lost");
		//	return 1;
		//}
		sleep(1);
		puts("before select");
		curfds = afds;
		if(select(fdmax + 2, &curfds, NULL, NULL, &tt)<0) perror("select");
		for (j = 0; j < fdmax+2; j++){
			if (FD_ISSET(j, &curfds)){
				bulk_read(afd,buf,MSG_SIZE);
				puts(buf);
				if(bulk_write(afd,buf,MSG_SIZE)<0){
					//ERR("write");
					puts("connection lost");
					return 1;
				}
			}
		}
		puts("after select");
		
	}
}

void user_listen(){
	int sockfd = create_socket(USER_PORT);
	int fdmax = sockfd + 1;//przez to mogą nie działać równolegle trzeba to ogarnąć jedną funkcją rejestrującą
	fd_set sfds;
	FD_ZERO(&sfds);
	FD_SET(sockfd, &sfds);
	while (true){
		puts("czekam...");
		if (select(fdmax + 1, &sfds, NULL, NULL, NULL) == -1){
			if (errno != EINTR) ERR("Cannot select");
			else if(stop){
				close_all(sfds, fdmax);
				ERR("SIGINT");
			}
		}
		if (FD_ISSET(sockfd, &sfds))
			user_process(sockfd, fdmax);
	}
}
