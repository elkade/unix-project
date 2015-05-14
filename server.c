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
#include <stdbool.h>
#include <assert.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
#define PARTY_COUNT 5
#define PARTY_NAME_LENGTH 3
#define VOTES_LENGTH 4
#define MSG_SIZE 2048
#define BACKLOG 5

#define NAME_LENGTH 17
#define FLOAT_LENGTH 49
#define INT_LENGTH 17
#define ENUM_LENGTH 5
#define MAX_DB_LINE_LENGTH 128
#define CLIENT_DISPLAY_SIZE 128

#define ADMIN_PORT 5555

volatile sig_atomic_t stop = 0;
static const char CLIENTS_DB_NAME[] = "clients.txt";
static const char CLIENTS_DB_NAME_BUF[] = "clients_buf.txt";
static const char SERVICES_DB_NAME[] = "services.txt";
static const char SERVICES_DB_NAME_BUF[] = "services_buf.txt";

void admin_listen();
int sethandler( void (*f)(int), int);

typedef enum role{
	USER,
	OWNER
}role;
typedef enum client_display_mode{
	ALL,
	COUNTERS
}client_display_mode;
typedef struct service{
	char name[16];
	char host[16];
	uint16_t port;
}service;
typedef enum tariff{
	PREPAID,
	SUBSCRIPTION
}tariff;
typedef struct client{
	char name[NAME_LENGTH];
	bool is_active;
	tariff tariff_plan;
	int capacity;
	float amount;
}client;

client* new_client(char* name, bool is_active, tariff tariff_plan, int capacity, float amount){
	client *c = malloc(sizeof(client));
	strncpy(c->name,name,NAME_LENGTH);
	c->amount=amount;
	c->capacity=capacity;
	c->tariff_plan = tariff_plan;
	c->is_active=is_active;
	return c;
}
void delete_client(client* c){
	free(c);
}
void display_client(client *c, char* result, client_display_mode display_mode){
	if(c==NULL)
		return;
	char buf[CLIENT_DISPLAY_SIZE];
	bzero(result,CLIENT_DISPLAY_SIZE);
	bzero(buf,CLIENT_DISPLAY_SIZE);
	
	snprintf(buf,CLIENT_DISPLAY_SIZE,"name:  %s\t",c->name);
	strncat(result,buf, CLIENT_DISPLAY_SIZE);
	bzero(buf,CLIENT_DISPLAY_SIZE);
	if(display_mode==ALL){
		snprintf(buf,CLIENT_DISPLAY_SIZE, "is active: %d\t",c->is_active);
		strncat(result,buf,  CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
	if(display_mode==ALL){
		snprintf(buf,CLIENT_DISPLAY_SIZE,"tariff plan: %d\t",c->tariff_plan);
		strncat(result, buf, CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
	if(display_mode==ALL){
		snprintf(buf,CLIENT_DISPLAY_SIZE,"capacity: %d\t",c->capacity);
		strncat(result, buf, CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
	if(display_mode==ALL || display_mode==COUNTERS){
		snprintf(buf,CLIENT_DISPLAY_SIZE,"amount: %.2f\n",c->amount);
		strncat(result,buf, CLIENT_DISPLAY_SIZE);
		bzero(buf,CLIENT_DISPLAY_SIZE);
	}
}
int db_row_to_client(char* db_row, client* c ){
	char buf[MAX_DB_LINE_LENGTH];
	bzero(buf,MAX_DB_LINE_LENGTH);
	puts("DESERIALIZACJA\n");
	
	strncpy(buf,db_row,INT_LENGTH-1);
		printf("buf: %s\n",buf);

	int offset = atoi(buf);
	printf("offset: %d\n",offset);
	strncpy(c->name,db_row+INT_LENGTH-1,offset);
			printf("name: %s\n",c->name);
	offset+=INT_LENGTH-1;
		printf("offset: %d\n",offset);
	bzero(buf,MAX_DB_LINE_LENGTH);
	strncpy(buf,db_row+offset,ENUM_LENGTH-1);
			printf("buf2: %s\n",buf);

	c->is_active = atoi(buf);
	offset+=ENUM_LENGTH-1;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,ENUM_LENGTH-1);
			printf("buf3: %s\n",buf);

	c->tariff_plan = atoi(buf);
	offset+=ENUM_LENGTH-1;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,INT_LENGTH-1);
			printf("buf4: %s\n",buf);

	c->capacity = atoi(buf);
	offset+=INT_LENGTH-1;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,FLOAT_LENGTH-1);
			printf("buf5: %s\n",buf);

	c->amount = atof(buf);
		puts("\nKONIEC\n");

	return 0;
}
int client_to_db_row(client* c, char* buf){
	char buf2[MAX_DB_LINE_LENGTH];
	bzero(buf,MAX_DB_LINE_LENGTH);
	bzero(buf2,MAX_DB_LINE_LENGTH);
		puts("SERIALIZACJA\n");

	printf("%s\t%ld\n",c->name,strlen(c->name));
	
	snprintf(buf2, INT_LENGTH, "%.*ld", INT_LENGTH-1 , strlen(c->name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, NAME_LENGTH, "%s", c->name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, ENUM_LENGTH, "%.*d", ENUM_LENGTH-1, c->is_active);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, ENUM_LENGTH, "%.*d", ENUM_LENGTH-1, c->tariff_plan);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, INT_LENGTH, "%.*d", INT_LENGTH-1, c->capacity);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, FLOAT_LENGTH + 1, "%0*.*f\n", FLOAT_LENGTH -1-2-1,2, c->amount);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	return 0;
}
int db_insert_client(client* c){
	FILE *f = fopen(CLIENTS_DB_NAME, "a");
	char buf[MAX_DB_LINE_LENGTH];
	client_to_db_row(c, buf);
		printf("%s",buf);

	fwrite(buf,1,strlen(buf),f);
	fclose(f);
	return 0;
}
int db_select_client(char* name, client* c){
	FILE *f;
	printf("name: %s\t%ld\n", name, strlen(name));
	f = fopen(CLIENTS_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}
	puts("Plik otwarty pomyslnie!");

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		//printf("line: %s\t%ld\n", buf, strlen(buf));
		db_row_to_client(buf, c);
		//printf("name:%s\t%ld\n", c->name, strlen(c->name));
		if(strcmp(name,c->name)==0){
			printf("znaleziono");
			if (c == NULL){
				fclose(f);
				return 1;
			}
			fclose(f);
			return 0;
		}
		bzero(c->name,NAME_LENGTH);
	}
	puts("nie znaleziono");
	fclose(f);
	return 1;
}
int db_count_clients(){
	FILE *f;
	int count = 0;
	f = fopen(CLIENTS_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		count++;
	}
	fclose(f);
	return count;
}
int db_select_clients(client* clist, int n){//po tej funkcji trzeba zwalniać. zwraca liczbę elementów
	FILE *f;
	int i = 0;
	f = fopen(CLIENTS_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}
	puts("Plik otwarty pomyslnie!");

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		bzero(clist[i].name,NAME_LENGTH);
		db_row_to_client(buf, &clist[i++]);
		bzero(buf,MAX_DB_LINE_LENGTH);
		if(i>=n)
			break;
	}
	fclose(f);
	return 0;
}
int db_delete_client(char* name){
	FILE *oldTodoFile;
	client c;
	if(name==NULL)//poprawić, bo przy nullu segmentation fault
		return 1;
	oldTodoFile = fopen(CLIENTS_DB_NAME, "r");

	FILE *todoFile;
	todoFile = fopen(CLIENTS_DB_NAME_BUF, "w");
	int lineNumber = 0;
	int len;
	char line[MAX_DB_LINE_LENGTH];

	if (oldTodoFile != NULL) {
		while (fgets(line, sizeof line, oldTodoFile)) {
			len = strlen(line);
			if (len && (line[len - 1] != '\n')) {} else {
				lineNumber++;
				db_row_to_client(line,&c);
				if(strcmp(name,c.name)==0) {
					// Do nothing   
				} else {
					fputs(line, todoFile);
				}
						bzero(c.name,NAME_LENGTH);

			}
		}
	} else {
		printf("ERROR");
	}
	remove(CLIENTS_DB_NAME);
	rename(CLIENTS_DB_NAME_BUF, CLIENTS_DB_NAME);
	fclose(oldTodoFile);
	fclose(todoFile);
return 0;
}
int db_update_client(char* name, client* c){
	if(db_delete_client(name)!=0) return 1;
	if(db_insert_client(c)!=0) return 1;
	return 0;
}
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
	admin_listen();
	return 0;
	char in[NAME_LENGTH];
	bzero(in,NAME_LENGTH);
	//client *c;
	fgets(in,NAME_LENGTH, stdin);
	//c = new_client(in,true,SUBSCRIPTION,123,22.93);
	//db_insert_client(c);
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

char** str_split(char* a_str, const char a_delim)
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

void close_all(fd_set fds, int fdmax){
	int i;
	for (i = 0; i<fdmax + 1; i++)
		if (FD_ISSET(i, &fds)){
			shutdown(i,SHUT_RDWR);
			if(TEMP_FAILURE_RETRY(close(i))<0)ERR("close:");
		}
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
		}
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
