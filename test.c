#include "header.h"
#include "wrapped_message.h"

#define SET_MAX 64

typedef struct service_set_elem{
	int fd;
	char name[SERVICE_NAME_LENGTH];
	bool is_empty;
}service_set_elem;

typedef struct service_set{
	service_set_elem arr[SET_MAX];
	int n;//size
}service_set;



typedef struct client_set_elem{
	int fd;
	char name[NAME_LENGTH];
	bool is_empty;
	service_set sset;
}client_set_elem;

typedef struct client_set{
	client_set_elem arr[SET_MAX];
	int n;//size
}client_set;







void service_set_init(service_set *ss, int n){
	int i;
	if(n>=SET_MAX)
		ERR("set:");
	ss->n = n;
	for (i = 0; i < ss->n; i++){
		ss->arr[i].is_empty = true;
	}
}

int service_set_add(service_set *ss, char* name){
	int i;
	for (i = 0; i < ss->n; i++){
		if(ss->arr[i].is_empty){
			bzero(ss->arr[i].name,NAME_LENGTH);
			strcpy(ss->arr[i].name,name);
			ss->arr[i].is_empty = false;
			return 0;
		}
	}
	return 1;
}

int service_set_update(service_set *ss, char* name, int fd){
	int i;
	for (i = 0; i < ss->n; i++){
		if(!ss->arr[i].is_empty && strcmp(ss->arr[i].name,name)==0){
			ss->arr[i].fd = fd;
			return 0;
		}
	}
	return 1;
}
/*
 * Removes all elements from s equal to val.
 * Returns number of removed elements.
 */
int service_set_remove(service_set *ss, char* name){
	int i, c = 0;
	for (i = 0; i < ss->n; i++){
		if(strcmp(ss->arr[i].name, name)==0 && !ss->arr[i].is_empty){
			ss->arr[i].is_empty = true;
			c++;
		}
	}
	return c;
}
int service_set_count(service_set *ss){
	int i, c = 0;
	for (i = 0; i < ss->n; i++)
		if(ss->arr[i].is_empty == true)
			c++;
	return c;
}





void client_set_init(client_set *cs, int n){
	int i;
	if(n>=SET_MAX)
		ERR("set:");
	cs->n = n;
	for (i = 0; i < cs->n; i++){
		cs->arr[i].is_empty = true;
	}
}

int client_set_add(client_set *cs, char* name){
	int i;
	for (i = 0; i < cs->n; i++){
		if(cs->arr[i].is_empty){
			bzero(cs->arr[i].name,NAME_LENGTH);
			strcpy(cs->arr[i].name,name);
			cs->arr[i].is_empty = false;
			return 0;
		}
	}
	return 1;
}

int client_set_update(client_set *cs, char* name, int fd){
	int i;
	for (i = 0; i < cs->n; i++){
		if(!cs->arr[i].is_empty && strcmp(cs->arr[i].name,name)==0){
			cs->arr[i].fd = fd;
			return 0;
		}
	}
	return 1;
}
/*
 * Removes all elements from s equal to val.
 * Returns number of removed elements.
 */
int client_set_remove(client_set *cs, char* name){
	int i, c = 0;
	for (i = 0; i < cs->n; i++){
		if(strcmp(cs->arr[i].name, name)==0 && !cs->arr[i].is_empty){
			cs->arr[i].is_empty = true;
			c++;
		}
	}
	return c;
}
int client_set_count(service_set *cs){
	int i, c = 0;
	for (i = 0; i < cs->n; i++)
		if(cs->arr[i].is_empty == true)
			c++;
	return c;
}



int main(int argc , char *argv[]){
	char buf[MSG_SIZE];
	bzero(buf,MSG_SIZE);
int n = MSG_SIZE;
	wrapped_message msg_from_app;
	wrapped_message new;
	strncpy(msg_from_app.service_name, "multicast", SERVICE_NAME_LENGTH);
	strncpy(msg_from_app.client_name, "aleksander", NAME_LENGTH);
	strncpy(msg_from_app.app_name, "0000000000000000", APP_NAME_LENGTH);
	strncpy(msg_from_app.content, "bbb", MSG_CONTENT_SIZE);
	msg_from_app.status = REGULAR;
	
	printf("%s\n",msg_from_app.app_name);//dodać obsługę app_name
	printf("%s\n",msg_from_app.service_name);
	printf("%s\n",msg_from_app.client_name);
	printf("%s\n",msg_from_app.content);
	printf("%d\n",msg_from_app.status);
	
	wrapped_message_to_str(buf, msg_from_app, n);
	puts(buf);
	str_to_wrapped_message(buf, &new,  n);
	
	printf("%s\n",new.service_name);
	printf("%s\n",new.client_name);
	printf("%s\n",new.content);
	printf("%d\n",new.status);
    exit(EXIT_SUCCESS);
}

//#define NAME_LENGTH 17
//#define FLOAT_LENGTH 49
//#define INT_LENGTH 17
//#define ENUM_LENGTH 5
//#define MAX_DB_LINE_LENGTH 128
//static const char CLIENTS_DB_NAME[] = "clients.txt";
//static const char CLIENTS_DB_NAME_BUF[] = "clients_buf.txt";
//static const char SERVICES_DB_NAME[] = "services.txt";
//static const char SERVICES_DB_NAME_BUF[] = "services_buf.txt";

//typedef enum role{
	//USER,
	//OWNER
//}role;

//typedef struct service{
	//char name[16];
	//char host[16];
	//uint16_t port;
//}service;

//typedef enum tariff{
	//PREPAID,
	//SUBSCRIPTION
//}tariff;

//typedef struct client{
	//char name[NAME_LENGTH];
	//bool is_active;
	//tariff tariff_plan;
	//int capacity;
	//float amount;
//}client;

//client* new_client(char* name, bool is_active, tariff tariff_plan,
		//int capacity, float amount){
	//client *c = malloc(sizeof(client));
	//strncpy(c->name,name,NAME_LENGTH);
	//c->amount=amount;
	//c->capacity=capacity;
	//c->tariff_plan = tariff_plan;
	//c->is_active=is_active;
	//return c;
//}

//void delete_client(client* c){
	//free(c);
//}

//int db_row_to_client(char* db_row, client* c ){//powinno zwracać int
	//char buf[MAX_DB_LINE_LENGTH];
	//bzero(buf,MAX_DB_LINE_LENGTH);
	//puts("DESERIALIZACJA\n");
	
	//strncpy(buf,db_row,INT_LENGTH-1);
		//printf("buf: %s\n",buf);

	//int offset = atoi(buf);
	//printf("offset: %d\n",offset);
	//strncpy(c->name,db_row+INT_LENGTH-1,offset);
			//printf("name: %s\n",c->name);
	//offset+=INT_LENGTH-1;
		//printf("offset: %d\n",offset);
	//bzero(buf,MAX_DB_LINE_LENGTH);
	//strncpy(buf,db_row+offset,ENUM_LENGTH-1);
			//printf("buf2: %s\n",buf);

	//c->is_active = atoi(buf);
	//offset+=ENUM_LENGTH-1;
		//bzero(buf,MAX_DB_LINE_LENGTH);

	//strncpy(buf,db_row+offset,ENUM_LENGTH-1);
			//printf("buf3: %s\n",buf);

	//c->tariff_plan = atoi(buf);
	//offset+=ENUM_LENGTH-1;
		//bzero(buf,MAX_DB_LINE_LENGTH);

	//strncpy(buf,db_row+offset,INT_LENGTH-1);
			//printf("buf4: %s\n",buf);

	//c->capacity = atoi(buf);
	//offset+=INT_LENGTH-1;
		//bzero(buf,MAX_DB_LINE_LENGTH);

	//strncpy(buf,db_row+offset,FLOAT_LENGTH-1);
			//printf("buf5: %s\n",buf);

	//c->amount = atof(buf);
		//puts("\nKONIEC\n");

	//return 0;
//}

//int client_to_db_row(client* c, char* buf){
	//char buf2[MAX_DB_LINE_LENGTH];
	//bzero(buf,MAX_DB_LINE_LENGTH);
	//bzero(buf2,MAX_DB_LINE_LENGTH);
		//puts("SERIALIZACJA\n");

	//printf("%s\t%ld\n",c->name,strlen(c->name));
	
	//snprintf(buf2, INT_LENGTH, "%.*ld", INT_LENGTH-1 , strlen(c->name));
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//snprintf(buf2, NAME_LENGTH, "%s", c->name);
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//snprintf(buf2, ENUM_LENGTH, "%.*d", ENUM_LENGTH-1, c->is_active);
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//snprintf(buf2, ENUM_LENGTH, "%.*d", ENUM_LENGTH-1, c->tariff_plan);
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//snprintf(buf2, INT_LENGTH, "%.*d", INT_LENGTH-1, c->capacity);
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//snprintf(buf2, FLOAT_LENGTH + 1, "%0*.*f\n", FLOAT_LENGTH -1-2-1,2, c->amount);
	//strncat(buf,buf2,strlen(buf2));
	//bzero(buf2,MAX_DB_LINE_LENGTH);
	//return 0;
//}

//int db_insert_client(client* c){
	//FILE *f = fopen(CLIENTS_DB_NAME, "a");
	//char buf[MAX_DB_LINE_LENGTH];
	//client_to_db_row(c, buf);
		//printf("%s",buf);

	//fwrite(buf,1,strlen(buf),f);
	//fclose(f);
	//return 0;
//}

//int db_select_client(char* name, client* c){
	//FILE *f;
	//printf("name: %s\t%ld\n", name, strlen(name));
	//f = fopen(CLIENTS_DB_NAME, "r");
	//if (f == NULL){
		//perror("Nie udalo sie otworzyc pliku notatki.txt");
		//return 1;
	//}
	//puts("Plik otwarty pomyslnie!");

	//char buf[MAX_DB_LINE_LENGTH];
	//while (fgets (buf, sizeof(buf), f)) {
		//printf("line: %s\t%ld\n", buf, strlen(buf));
			//db_row_to_client(buf, c);
			//printf("name:%s\t%ld", c->name, strlen(c->name));
			//if(strcmp(name,c->name)==0){
				//printf("znaleziono");
			//fclose(f);
			//if (c == NULL)
				//return 1;
			//return 0;
		//}else{
		//}
	//}
	//if (ferror(f)) {
		//fprintf(stderr,"Oops, error reading stdin\n");
		//abort();
	//}
	//fclose(f);
	//return 1;
//}

//int db_delete_client(char* name){
	//FILE *oldTodoFile;
	//client c;

	//oldTodoFile = fopen(CLIENTS_DB_NAME, "r");

	//FILE *todoFile;
	//todoFile = fopen(CLIENTS_DB_NAME_BUF, "w");
	//int lineNumber = 0;
	//int len;
	//char line[MAX_DB_LINE_LENGTH];

	//if (oldTodoFile != NULL) {
		//while (fgets(line, sizeof line, oldTodoFile)) {
			//len = strlen(line);
			//if (len && (line[len - 1] != '\n')) {} else {
				//lineNumber++;
				//db_row_to_client(line,&c);
				//if(strcmp(name,c.name)==0) {
					//// Do nothing   
				//} else {
					//fputs(line, todoFile);
				//}
			//}
		//}
	//} else {
		//printf("ERROR");
	//}
	//remove(CLIENTS_DB_NAME);
	//rename(CLIENTS_DB_NAME_BUF, CLIENTS_DB_NAME);
	//fclose(oldTodoFile);
	//fclose(todoFile);
//return 0;
//}

//int db_update_client(char* name, client* c){
	//if(db_delete_client(name)!=0) return 1;
	//if(db_insert_client(c)!=0) return 1;
	//return 0;
//}

//int auth(char* input, role who){
	//char owner_name[NAME_LENGTH] = "admin";
	//client c;
	//switch (who)
	//{
		//case USER:
			////sprawdzić, czy input jest na liście klientów
			//if(db_select_client(input, &c)==0){
					////klient istnieje
					//return 0;
				//}
			//break;
		//case OWNER:
			////sprawdzić, czy input to nazwa właściciela
			//if(strncmp(input, owner_name, strlen(owner_name))==0)
				//return 0;
			//break;
	//}
	//return 1;
//}

//void display_client(client *c){
	//if(c==NULL)
		//return;
	//printf("name:  %s",c->name);
	//printf("is active: %d",c->is_active);
	//printf("tariff plan: %d",c->tariff_plan);
	//printf("capacity: %d",c->capacity);
	//printf("amount: %f",c->amount);
//}

//void handle_client(char* name){
	//client c;
	//if(db_select_client(name,&c)==1) return;
	//display_client(&c);
//}

//int main(int argc, char** argv){
	//char in[NAME_LENGTH];
	//bzero(in,NAME_LENGTH);
	//client *c;
	//fgets(in,NAME_LENGTH, stdin);
	//c = new_client(in,true,SUBSCRIPTION,123,22.93);
	//db_insert_client(c);
	//handle_client(in);
	//delete_client(c);
	//return 0;
	//if(auth(in, USER)==0){
		//puts("OK");
		//handle_client(in);
	//}
	//else
		//puts("failed");
	//exit(EXIT_SUCCESS);
//}











////#include <stdio.h> 
////#include <termios.h>
////#include <stdio.h>
////#include <string.h>

////#define SIZE 8

////static struct termios old, new;

/////* Initialize new terminal i/o settings */
////void initTermios(int echo) 
////{
  ////tcgetattr(0, &old); /* grab old terminal i/o settings */
  ////new = old; /* make new settings same as old settings */
  ////new.c_lflag &= ~ICANON; /* disable buffered i/o */
  ////new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  ////tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
////}

/////* Restore old terminal i/o settings */
////void resetTermios(void) 
////{
  ////tcsetattr(0, TCSANOW, &old);
////}

/////* Read 1 character - echo defines echo mode */
////char getch_(int echo) 
////{
  ////char ch;
  ////initTermios(echo);
  ////ch = getchar();
  ////resetTermios();
  ////return ch;
////}

/////* Read 1 character without echo */
////char getch(void) 
////{
  ////return getch_(0);
////}

/////* Read 1 character with echo */
////char getche(void) 
////{
  ////return getch_(1);
////}

////void delete(int n){
	////int i;
	////printf("%c[2K",27);
	////for (i = 0; i < n; i++){
		////printf("\b");
	////}
////}

////void read_line(char* buf, int m){
	////static char inputs[SIZE][SIZE];
	////static int n;
	////char c;
	////int i=0, j=0, k=0;
	////bzero(buf,m);
	
	////n++;
	////n %= SIZE;

	////strncpy(inputs[n],buf,SIZE);

	
	////while((c=getch())!='\n'){
		////if(c=='\033'){
			////delete(k);
			////k=0;
			////bzero(buf,m);
			////i=0;
			////if((c=getch())!=91)
				////continue;
			////if((c=getch())=='A')
				////j=(j-1+SIZE)%SIZE;
			////else if(c=='B')
				////j=(j+1+SIZE)%SIZE;
			////else
				////continue;
			////strncpy(buf,inputs[(n+j+SIZE)%SIZE],SIZE);
			////i=k=strlen(buf);
			////delete(i);
			////printf("%s",buf);
		////}
		////else if(c==127){
			////delete(k);//to jest słabe
			////if(k>0)
				////k--;
			////if(i>0)
				////i--;
			////buf[i]='\0';
			////strncpy(inputs[n],buf,SIZE);
			////printf("%s",buf);
		////}
		////else{
			////k++;
			////printf("%c",c);
			////if(i<m){
				////buf[i++]=c;
				////if(i<m)
					////buf[i]='\0';
			////}
			////strncpy(inputs[n],buf,SIZE);
		////}
	////}
	////strncpy(inputs[n],buf,SIZE);
	////puts("");
////}

////int main ( void ){
	////char buf[8];
	////while(1){
	////read_line(buf,8);
	////printf("%s\n",buf);
	////}
	////return 0;
	////delete(3);
	////char inputs[128][128];
  ////int ch, i = 0, n=0;
//////while ( (ch=getch()) != EOF )printf("%d\n",ch);
  ////do {
////if ((ch=getch() )== '\033') { // if the first value is esc
	//////printf("%d\n",ch);
	////fflush(stdin);
    ////if((ch=getch())!=91){
		//////printf("%d\n",ch);
		////continue; // skip the [
	////}
    //////printf("%d\n",ch);
    ////switch((ch=getch())) { // the real value
        ////case 'A':
            //////printf("A\n");
            //////n++;
            ////if(n-i>=0)
				////i++;
			//////printf("%c[2K", );

            ////printf("%sad%c[2K%c[2Kas",inputs[n-i],27,27);
            ////break;
        ////case 'B':
            //////printf("B\n");
            ////break;
        ////case 'C':
            //////printf("C\n");
            ////break;
        ////case 'D':
            //////printf("D\n");
            ////break;
    ////}
////}else{
      ////sprintf(inputs[n++],"%d\n",ch);
      ////printf("%d\n",ch);
 ////i=0;}   }while ( 1);//( ch = get_code() ) != KEY_ESC 
////printf("%d\n",ch);
////puts("chuj");
  ////return 0;
////}



//int admin_process(int sockfd, int fdmax){
	//char buf[MSG_SIZE];
	//fd_set afds;
	//int afd;//file descriptor admina
	//bool is_authenticated = false;
	
	//if ((afd = TEMP_FAILURE_RETRY(accept(sockfd, NULL, NULL))) == -1)
		//ERR("Cannot accept connection");
	//FD_SET(afd, &afds);
	//puts("odebrano połączenie");
	////przyszło połączenie od admina
	//while(true){//zmienić!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//if (select(fdmax + 1, &afds, NULL, NULL, NULL) == -1){
			//if (errno != EINTR) ERR("Cannot select");
			//else if(stop){
				//close_all(afds, fdmax);
				//ERR("SIGINT");
			//}
		//}//nieeeeeeeeeeeeeeeee jeeeeeeeeeeeeeest dddddddddddddddobrze curfds
		//puts("odebrano wiadomość");
				////słucham państwa
		//if(bulk_read(afd,buf,MSG_SIZE)<0){
			////ERR("read");
			//puts("connection lost");
			//return 1;
		//}
		//puts(buf);
		//trim(buf,strlen(buf));
		//puts(buf);
		//char response[MSG_SIZE];
		//if(!is_authenticated){
			//if((is_authenticated = (auth(buf,OWNER)==0)))
				//strcpy(response,"authentication successful");
			//else
				//strcpy(response,"authentication failed");//coś jest nie tak, bo mimo, że idzie admin, to pisze failed
		//}
		//else
			//admin_handle_message(buf, response, is_authenticated);
		//puts(response);
		//if(bulk_write(afd,response,MSG_SIZE)<0){
			////ERR("write");
			//puts("connection lost");
			//return 1;
		//}
	//}
//}

//void admin_listen(){
	//int sockfd = create_socket(ADMIN_PORT);
	//int fdmax = sockfd + 1;
	//fd_set sfds;
	//FD_ZERO(&sfds);
	//FD_SET(sockfd, &sfds);
	//while (true){
		//puts("czekam...");
		//if (select(fdmax + 1, &sfds, NULL, NULL, NULL) == -1){
			//if (errno != EINTR) ERR("Cannot select");
			//else if(stop){
				//close_all(sfds, fdmax);
				//ERR("SIGINT");
			//}
		//}
		//if (FD_ISSET(sockfd, &sfds))
			//admin_process(sockfd, fdmax);
	//}
//}




//pipe main server




	//int pipefd[2], i, n=16;
	//int *pipein = pipefd + 1;
	//char buf[n];
	//bzero(buf,n);
	//if(pipe(pipefd))
		//ERR("pipe:");
	
	//pthread_t thread;

	//if(pthread_create( &thread, NULL, thread_handler, (void*) pipefd))ERR("pthread:");
	
	//for( i = 0; ; i++ ){
		////sleep(1);
		//sprintf(buf,"%d\n",i);
		//if(bulk_write(*pipein,buf,16)<0)ERR("write:");
	//}
	
	//pthread_join( thread, NULL);
	
	//if(TEMP_FAILURE_RETRY(close(*pipein))<0)
		//ERR("close:");
	
	//return 0;
	
	
	
	
	
	
	
	
	//server
	
	
	
	
	
	
	
	
//#include "header.h"
//#include "client.h"
//#include "db.h"
//#include "service.h"
//#include "wrapped_message.h"
//#include "set.h"
//#include <pthread.h>
//volatile sig_atomic_t stop = 0;


//void admin_listen();
//void user_listen();
//void connect_to_services();

//typedef enum role{
	//USER,
	//OWNER
//}role;

//bool auth(char* input){
	//char owner_name[NAME_LENGTH] = "admin";
	//if(strncmp(input, owner_name, strlen(owner_name))==0)
		//return true;
	//return false;
//}
//void handle_client(char* name){
	//int i,  n = db_count_clients();
	//client c[n];
	//db_select_clients(c,n);
	//char buf[CLIENT_DISPLAY_SIZE];
	//for (i = 0; i < n; i++){
		//display_client(&c[i],buf,ALL);
		//puts(buf);
	//}
	
//}


////void *thread_handler( void *ptr ){
	////int *pipeout = (int*)ptr, n = 16;
	
	////char buf[n];
	////bzero(buf,n);
	
	////fd_set curfds, fds;
	
	////FD_ZERO(&fds);
	////FD_SET(*pipeout,&fds);
	
	////while(true){
		////curfds = fds;
		////if (select(*(pipeout + 1), &curfds, NULL, NULL, NULL)<0)
			////ERR("select:");
		////if(FD_ISSET(*pipeout,&curfds)){
			////if(bulk_read(*pipeout,buf,16)<0)
				////ERR("read:");
			////printf("%s\n",buf);
		////}
	////}
	////if(TEMP_FAILURE_RETRY(close(*pipeout))<0)
		////ERR("close:");
	////return NULL;
////}
////main!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
//int main(int argc, char** argv){
	//if(sethandler(SIG_IGN,SIGPIPE))
	    //ERR("Setting SIGPIPE:");
	//user_listen();
	//return 0;
	////admin_listen();
	////return 0;
	////char in[NAME_LENGTH];
	////bzero(in,NAME_LENGTH);
	////client *c;
	////fgets(in,NAME_LENGTH, stdin);
	////trim(in,strlen(in));
	////c = new_client(in,true,SUBSCRIPTION,123,22.93);
	////db_insert_client(c);
	////handle_client(in);
	//////delete_client(c);
	////return 0;
	////if(auth(in, USER)==0){
		////puts("OK");
		////handle_client(in);
	////}
	////else
		////puts("failed");
	//exit(EXIT_SUCCESS);
//}


//void close_all(fd_set fds, int fdmax){
	//int i;
	//for (i = 0; i<fdmax + 1; i++)
		//if (FD_ISSET(i, &fds)){
			//shutdown(i,SHUT_RDWR);
			//if(TEMP_FAILURE_RETRY(close(i))<0)ERR("close:");
		//}
//}

//int admin_info(char* response){
	//puts("admin_info");
	//strcat(response,"\ninfo\treturns the server functionality\n");
	//strcat(response,"list_clients\treturns the list of all clients\n");
	//strcat(response,"add_client <>\tdo zmiany\n");
	//strcat(response,"delete_client <name>\tremoves client from the list\n");
	//strcat(response,"get_data_counters\treturns counter values for all clients\n");
	//strcat(response,"boost_prepaid <name> <amount>\tincreases the prepaid amount for specific client\n");
	//strcat(response,"start_new_sub_period\treturns amounts and resets them for all clients with subscription\n");
	//strcat(response,"lock_client <name>\tlocks/unlocks the client\n");
	//strcat(response,"delete_service\tremoves the service from the list\n");
	//strcat(response,"add_service\tdo zmiany\n");
	//strcat(response,"list_services\tgets the list of all services\n");
	//return 0;
//}
//int admin_list_clients(char* response){
	//puts("admin_list_clients");//uwaga, bo liczba klientów może przekroczyć pojemmność wiadomości
	//char buf[CLIENT_DISPLAY_SIZE];
	
	//int i,  n = db_count_clients();
	//client clist[n];
	//db_select_clients(clist,n);
	
	
	//bzero(buf,CLIENT_DISPLAY_SIZE);
	//for (i = 0; i < n; i++){
		//display_client(&clist[i],buf,ALL);
		//strncat(response,buf,CLIENT_DISPLAY_SIZE);
		//bzero(buf,CLIENT_DISPLAY_SIZE);
	//}
	//return 0;
//}
//int admin_add_client(char* name,/* char* plan, char* capacity, */char* response){
	//puts("admin_add_client");
	//return 0;
//}
//int admin_delete_client(char* arg, char* response){
	//puts("admin_delete_client");
	//printf("arg: %s\n\n",arg);
	//if(db_delete_client(arg)==0){
		//strcat(response,"client removed\n");
		//return 0;
	//}
	//else{
		//strcat(response,"delete failed\n");
		//return 1;
	//}
//}
//int admin_get_data_counters(char* response){
	//puts("admin_get_data_counters");//uwaga, bo liczba klientów może przekroczyć pojemmność wiadomości
	//char buf[CLIENT_DISPLAY_SIZE];
	
	//int i,  n = db_count_clients();
	//client clist[n];
	//db_select_clients(clist,n);
	
	
	//bzero(buf,CLIENT_DISPLAY_SIZE);
	//for (i = 0; i < n; i++){
		//display_client(&clist[i],buf, COUNTERS);
		//strncat(response,buf,CLIENT_DISPLAY_SIZE);
		//bzero(buf,CLIENT_DISPLAY_SIZE);
	//}
	//return 0;
//}
//int admin_start_new_sub_period(char* response){
	//puts("admin_start_new_sub_period");
	//char buf[CLIENT_DISPLAY_SIZE];
	
	//int i,  n = db_count_clients();
	//client clist[n];
	//db_select_clients(clist,n);
	
	
	//bzero(buf,CLIENT_DISPLAY_SIZE);
	//for (i = 0; i < n; i++){
		//if(clist[i].tariff_plan==SUBSCRIPTION){
			//display_client(&clist[i],buf, COUNTERS);
			//clist[i].amount = 0;
			//db_update_client(clist[i].name,&clist[i]);
			//strncat(response,buf,CLIENT_DISPLAY_SIZE);
			//bzero(buf,CLIENT_DISPLAY_SIZE);
		//}
	//}
	//return 0;
//}
//int admin_boost_prepaid(char* name, char* amount , char* response){
	//puts("admin_boost_prepaid");
	//client c;
	//db_select_client(name,&c);
	//c.amount+=atof(amount);
	//db_update_client(c.name,&c);
	//return 0;
//}
//int admin_lock_client(char* name, char* response){
	//puts("admin_lock_client");
	//client c;
	//db_select_client(name,&c);
	//c.is_active=!c.is_active;
	//db_update_client(c.name,&c);
	//return 0;
//}
//int admin_delete_service(char* name, char* response){
	//puts("admin_delete_service");
	//return 0;
//}
//int admin_add_service(char* name, char* host, char* port, char* response){
	//puts("admin_add_service");
	//return 0;
//}
//int admin_list_services(char* response){
	//puts("admin_list_services");
	//return 0;
//}
//int admin_handle_message(char* msg, char* response, bool is_authenticated){
	//int result = 0;
	//bzero(response,MSG_SIZE);
	//char *cmd;
	//char** args;
	//puts("admin_handle_message");
	//if(is_authenticated)
		//puts("authenticated");
	
	//args = str_split(msg, ' ');//to nie jest dobrze bo nie sprawdzam, czy nie przyszedł syf
	//if(args==NULL){
		//puts("unhandled message");
		//strcpy(response,"unhandled message");
		//return 1;
	//}
	//cmd = args[0];
	
	//if(!is_authenticated){
		//puts("authentication failed");
		//strcpy(response,"authentication failed");
		//result = 1;
	//}else if(cmd==NULL){
		//puts("wrong syntax");
		//strcpy(response,"wrong syntax");
		//result = 1;
	//}
	//else if(strcmp(cmd,"info")==0)
		//result = admin_info(response);
	//else if(strcmp(cmd,"list_clients")==0)
		//result = admin_list_clients(response);
	//else if(strcmp(cmd,"add_client")==0)
		//result = admin_add_client(args[1], response);
	//else if(strcmp(cmd,"delete_client")==0)
		//result = admin_delete_client(args[1], response);
	//else if(strcmp(cmd,"get_data_counters")==0)
		//result = admin_get_data_counters(response);
	//else if(strcmp(cmd,"start_new_sub_period")==0)
		//result = admin_start_new_sub_period(response);
	//else if(strcmp(cmd,"boost_prepaid")==0)
		//result = admin_boost_prepaid(args[1], args[2], response);
	//else if(strcmp(cmd,"lock_client")==0)
		//result = admin_lock_client(args[1], response);
	//else if(strcmp(cmd,"list_services")==0)
		//result = admin_list_services(response);
	//else if(strcmp(cmd,"delete_service")==0)
		//result = admin_delete_service(args[1],response);
	//else if(strcmp(cmd,"add_service")==0)
		//result = admin_add_service(args[1], args[2], args[3],response);
	//else{
		//puts("unhandled message");
		//strcpy(response,"unhandled message");
		//result = 1;
	//}
	//free(args);
	//return result;
//}

//typedef struct thread_data{
	//char client_listen_socket;
	//char service_name[SERVICE_NAME_LENGTH];
	//char *client_name;
	//int *fdmax;
//}thread_data;

//void user_listen(){
	//char message[MSG_SIZE];
	//int clifd = create_socket(USER_PORT), fdmax, i, j, k;
	//int admin_listen_socket = create_socket(ADMIN_PORT), admfd = -1;
	//fd_set curfds, allfds;
	//bool is_authenticated = false;
	//fdmax = admin_listen_socket;
	
	//FD_ZERO(&allfds);
	
	//FD_SET(clifd,&allfds);
	//FD_SET(admin_listen_socket,&allfds);

	//int m = 128;
	//int n = 128;
	//client_set cset;
	//client_set_init(&cset,m);
	
	//int select_all_number;
	
	////struct timeval tt = {0,0};
	//while(true){//BRAKUJE OBSŁUGI PRZEPEŁNIENIA SETÓW I ROZŁĄCZENIA SIĘ SERWISU I JAK SERWIS NIE DZIAŁA, A KLIENT CHCE SIĘ POŁĄCZYĆ
//start:
		//puts("czekam");
		//curfds = allfds;
		//if ((select_all_number = select(fdmax + 1, &curfds, NULL, NULL, NULL))<0) perror("select");//jest coś ale nie wiadomo co
		//printf("dostałem %d rzeczy\n",select_all_number);
		//puts("fd:");
		//for (i = 0; i < fdmax+1; i++)
		//{
			//if(FD_ISSET(i,&curfds)){
				//printf("%d\n",i);
			//}
		//}
		//sleep(1);
		////getchar();
		////curfds = clifds;
		////if (select(fdmax + 1, &curfds, NULL, NULL, NULL)<0) perror("select");//sprawdzam sockety klienckie
		
		
		
		
		
		
		//if(FD_ISSET(admin_listen_socket,&curfds)){
			//select_all_number--;
			//printf("%s\n","admin chce się połączyć");
			//admfd = addnewfd(admin_listen_socket,&allfds,&fdmax);
			//printf("przydzielam adminowi: %d\n",admfd);
			//disconnect(admin_listen_socket,&allfds);
		//}
		//if(select_all_number<=0) goto start;
		
		
		//if(FD_ISSET(admfd, &curfds)){
			//select_all_number--;
			//bzero(message,MSG_SIZE);
			//if(bulk_read(admfd,message,MSG_SIZE)<0){
				//disconnect(admfd,&allfds);
				//admin_listen_socket = create_socket(ADMIN_PORT);
				//addnewfd_listen(admin_listen_socket,&allfds,&fdmax);
				//is_authenticated = false;
				//goto start;
			//}
			//puts("otrzymano");
			//puts(message);
			//trim(message,strlen(message));
			//puts(message);
			//char response[MSG_SIZE];
			//if(!is_authenticated){
				//if((is_authenticated = (auth(message))))
					//strcpy(response,"authentication successful");
				//else
					//strcpy(response,"authentication failed");//coś jest nie tak, bo mimo, że idzie admin, to pisze failed
			//}
			//else
				//admin_handle_message(message, response, is_authenticated);
			//puts(response);
			//puts("wysyłam do admina");
			//if(bulk_write(admfd,response,MSG_SIZE)<0){
				//disconnect(admfd,&allfds);
				//admin_listen_socket = create_socket(ADMIN_PORT);
				//addnewfd_listen(admin_listen_socket,&allfds,&fdmax);
				//is_authenticated = false;
				//goto start;
			//}
		//}
		//if(select_all_number<=0) goto start;
		//if(FD_ISSET(clifd,&curfds)){
			//select_all_number--;
			//printf("%s\n","nowy klient");
			
			//int fd = addnewfd(clifd,&allfds,&fdmax);
			//client_set_add_by_fd(&cset,fd, n);
			//printf("przydzialem: %d\n",fd);
		//}
		//if(select_all_number<=0) goto start;
		//for (i = 0; i < cset.n; i++){//dla każdego zarejestrowanego klienta
			//if(cset.arr[i].is_empty)
				//continue;
			//client_set_elem *celem = &cset.arr[i];
			//printf("sprawdzam klienta %d o fd %d\n",i,cset.arr[i].fd);
			//if(FD_ISSET(cset.arr[i].fd,&curfds)){
				//puts("znalazłem fd\n");
				////trzeba sprawdzić, czy klient jest w bazie
				//select_all_number--;
				//if( bulk_read(celem->fd, message, MSG_SIZE) < 0){
					//puts("błąd odczytu");
					//for (k = 0; k < celem->sset.n; k++){
						//if(celem->sset.arr[k].is_empty)
							//continue;
						//disconnect(celem->sset.arr[k].fd, &allfds);
						//service_set_remove_by_fd(&celem->sset,celem->sset.arr[k].fd);
					//}
					//disconnect(celem->fd, &allfds);
					//client_set_remove_by_fd(&cset,celem->fd);
					//goto start;//po disconnectcie trzeba wyjść z ifa
				//}
				//puts("otrzymuję1");
				//printf("%s\n",message);

				//wrapped_message msg_from_client;
				//str_to_wrapped_message(message,&msg_from_client,MSG_SIZE);
				//if(msg_from_client.status==DEREGISTER)
					//puts("\n\nDEREGISTER\n\n");
				
				//printf("\n%s\n",msg_from_client.service_name);
				//printf("%s\n",msg_from_client.client_name);
				//printf("%s\n",msg_from_client.content);
				//printf("%d\n\n",msg_from_client.status);
				
				//puts("rejestracja");
				//client c;
				//if(db_select_client(msg_from_client.client_name,&c)!=0){puts("nie ma klienta w bazie");}//nie ma w bazie
				////porównywanie w BAZIE POPRAWIĆ BO BIERZE DŁUGOŚĆ TYLKO TEGO OK
				//else{
					//puts("klient jest w bazie. n=0 więc rejestruję serwis");
					//strcpy(celem->name,c.name);
					//bool is_found = false;
					////TRZEBA SPRAWDZIĆ CZY W BAZIE MAMY W OGÓLE TAKI SERWIS
					//for (j = 0; j < celem->sset.n; j++){//POPRAWIC
						//if(celem->sset.arr[j].is_empty)
							//continue;
						//service_set_elem *selem = &celem->sset.arr[j];
						//if(strcmp(selem->name, msg_from_client.service_name)==0){
							//is_found = true;
							//printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
							//if( bulk_write(selem->fd, msg_from_client.content, MSG_CONTENT_SIZE) < 0){
								//puts("błąd wysyłania");
								//disconnect(selem->fd, &allfds);
								//client_set_remove_by_fd(&cset,selem->fd);
								//goto start;
							//}
							//puts("wysyłam1");
							//printf("%s\n",msg_from_client.content);
						//}
					//}
					//if(!is_found){
						//printf("%s\n",message);
						//service s;
						////trzeba sprawdzić, czy taki serwis jest w ogóle w bazie
						//if(db_select_service(msg_from_client.service_name,&s)!=0){puts("nie ma serwisu w bazie");}//nie znaleziono - trzeba zamknąć fd
						//else{
							//puts("rejestruje serwis");
							//int fd = create_socket_client(s.host_name,atoi(s.port_number));
							
							//service_set_add_by_fd(&celem->sset, fd);
							
							//service_set_update_by_fd(&celem->sset, fd, s.name);
							
							//addnewfd_listen(fd,&allfds,&fdmax);
							//printf("\n\nwysyłam do serwisu: %s\n\n",msg_from_client.content);
							//if( bulk_write(fd, msg_from_client.content, MSG_CONTENT_SIZE) < 0){
								//puts("write");
								//disconnect(fd,&allfds);//tu i trochę wyżej ogarnąć rozłączenie się serwisu
								//client_set_remove_by_fd(&cset,fd);
								//goto start;
							//}
							//puts("wysłałem");
						//}
					//}
				//}
			//}
			//if(select_all_number<=0) goto start;
			//for (j = 0; j < cset.arr[i].sset.n; j++){//dla każdego serwisu
				//if(cset.arr[i].sset.arr[j].is_empty)
					//continue;
				//service_set_elem *selem = &cset.arr[i].sset.arr[j];
				//printf("sprawdzam serwis %d\to fd %d\n",j,selem->fd);
				//if(FD_ISSET(selem->fd,&curfds)){//mamy coś od serwisu
					//select_all_number--;
					//if( bulk_read(selem->fd, message, MSG_SIZE) < 0){
						//puts("błąd odczytu");
						//disconnect(selem->fd,&allfds);
						//client_set_remove_by_fd(&cset,selem->fd);
						//goto start;
					//}
					//puts("otrzymuję2");
					//printf("%s\n",message);
					//wrapped_message msg_from_service;
					//bzero(msg_from_service.content,MSG_CONTENT_SIZE);
					//strcpy(msg_from_service.content,message);
					
					//bzero(msg_from_service.service_name,SERVICE_NAME_LENGTH);
					//strcpy(msg_from_service.service_name,selem->name);
					
					//bzero(msg_from_service.client_name,NAME_LENGTH);
					//strcpy(msg_from_service.client_name,celem->name);
					//msg_from_service.status = REGULAR;
					//bzero(message,MSG_SIZE);
					//wrapped_message_to_str(message,msg_from_service,MSG_SIZE);
					//puts("wysyłam2");
					//printf("%s\n",message);
					//if( bulk_write(celem->fd, message, MSG_SIZE) < 0){
						//puts("błąd wysyłania");
						//for (k = 0; k < celem->sset.n; k++){
							//if(celem->sset.arr[k].is_empty)
								//continue;
							//printf("odłączam serwis. k=%d\n",k);
							//disconnect(celem->sset.arr[k].fd, &allfds);
							//service_set_remove_by_fd(&celem->sset,celem->sset.arr[k].fd);
						//}
						//puts("odłączam klienta");
						//disconnect(celem->fd,&allfds);
						//client_set_remove_by_fd(&cset,celem->fd);
						//goto start;
					//}
				//}
				//if(select_all_number<=0) goto start;
			//}
			//if(select_all_number<=0) goto start;
		//}
	//}

//}
