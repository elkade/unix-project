#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <stdbool.h>
#include <float.h>

#define NAME_LENGTH 17
#define FLOAT_LENGTH 49
#define INT_LENGTH 17
#define ENUM_LENGTH 5
#define MAX_DB_LINE_LENGTH 128
static const char CLIENTS_DB_NAME[] = "clients.txt";
static const char CLIENTS_DB_NAME_BUF[] = "clients_buf.txt";
static const char SERVICES_DB_NAME[] = "services.txt";
static const char SERVICES_DB_NAME_BUF[] = "services_buf.txt";

typedef enum role{
	USER,
	OWNER
}role;

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

client* new_client(char* name, bool is_active, tariff tariff_plan,
		int capacity, float amount){
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

int db_row_to_client(char* db_row, client* c ){//powinno zwracać int
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
		printf("line: %s\t%ld\n", buf, strlen(buf));
			db_row_to_client(buf, c);
			printf("name:%s\t%ld", c->name, strlen(c->name));
			if(strcmp(name,c->name)==0){
				printf("znaleziono");
			if (c == NULL)
				return 1;
			break;
		}else{
		}
	}
	if (ferror(f)) {
		fprintf(stderr,"Oops, error reading stdin\n");
		abort();
	}
	fclose(f);
	return 0;
}

int db_delete_client(char* name){
	FILE *oldTodoFile;
	client c;

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

void display_client(client *c){
	if(c==NULL)
		return;
	printf("name:  %s",c->name);
	printf("is active: %d",c->is_active);
	printf("tariff plan: %d",c->tariff_plan);
	printf("capacity: %d",c->capacity);
	printf("amount: %f",c->amount);
}

void handle_client(char* name){
	client c;
	if(db_select_client(name,&c)==1) return;
	display_client(&c);
}

int main(int argc, char** argv){
	char in[NAME_LENGTH];
	bzero(in,NAME_LENGTH);
	client *c;
	fgets(in,NAME_LENGTH, stdin);
	c = new_client(in,true,SUBSCRIPTION,123,22.93);
	db_insert_client(c);
	handle_client(in);
	delete_client(c);
	return 0;
	if(auth(in, USER)==0){
		puts("OK");
		handle_client(in);
	}
	else
		puts("failed");
	exit(EXIT_SUCCESS);
}











//#include <stdio.h> 
//#include <termios.h>
//#include <stdio.h>
//#include <string.h>

//#define SIZE 8

//static struct termios old, new;

///* Initialize new terminal i/o settings */
//void initTermios(int echo) 
//{
  //tcgetattr(0, &old); /* grab old terminal i/o settings */
  //new = old; /* make new settings same as old settings */
  //new.c_lflag &= ~ICANON; /* disable buffered i/o */
  //new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  //tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
//}

///* Restore old terminal i/o settings */
//void resetTermios(void) 
//{
  //tcsetattr(0, TCSANOW, &old);
//}

///* Read 1 character - echo defines echo mode */
//char getch_(int echo) 
//{
  //char ch;
  //initTermios(echo);
  //ch = getchar();
  //resetTermios();
  //return ch;
//}

///* Read 1 character without echo */
//char getch(void) 
//{
  //return getch_(0);
//}

///* Read 1 character with echo */
//char getche(void) 
//{
  //return getch_(1);
//}

//void delete(int n){
	//int i;
	//printf("%c[2K",27);
	//for (i = 0; i < n; i++){
		//printf("\b");
	//}
//}

//void read_line(char* buf, int m){
	//static char inputs[SIZE][SIZE];
	//static int n;
	//char c;
	//int i=0, j=0, k=0;
	//bzero(buf,m);
	
	//n++;
	//n %= SIZE;

	//strncpy(inputs[n],buf,SIZE);

	
	//while((c=getch())!='\n'){
		//if(c=='\033'){
			//delete(k);
			//k=0;
			//bzero(buf,m);
			//i=0;
			//if((c=getch())!=91)
				//continue;
			//if((c=getch())=='A')
				//j=(j-1+SIZE)%SIZE;
			//else if(c=='B')
				//j=(j+1+SIZE)%SIZE;
			//else
				//continue;
			//strncpy(buf,inputs[(n+j+SIZE)%SIZE],SIZE);
			//i=k=strlen(buf);
			//delete(i);
			//printf("%s",buf);
		//}
		//else if(c==127){
			//delete(k);//to jest słabe
			//if(k>0)
				//k--;
			//if(i>0)
				//i--;
			//buf[i]='\0';
			//strncpy(inputs[n],buf,SIZE);
			//printf("%s",buf);
		//}
		//else{
			//k++;
			//printf("%c",c);
			//if(i<m){
				//buf[i++]=c;
				//if(i<m)
					//buf[i]='\0';
			//}
			//strncpy(inputs[n],buf,SIZE);
		//}
	//}
	//strncpy(inputs[n],buf,SIZE);
	//puts("");
//}

//int main ( void ){
	//char buf[8];
	//while(1){
	//read_line(buf,8);
	//printf("%s\n",buf);
	//}
	//return 0;
	//delete(3);
	//char inputs[128][128];
  //int ch, i = 0, n=0;
////while ( (ch=getch()) != EOF )printf("%d\n",ch);
  //do {
//if ((ch=getch() )== '\033') { // if the first value is esc
	////printf("%d\n",ch);
	//fflush(stdin);
    //if((ch=getch())!=91){
		////printf("%d\n",ch);
		//continue; // skip the [
	//}
    ////printf("%d\n",ch);
    //switch((ch=getch())) { // the real value
        //case 'A':
            ////printf("A\n");
            ////n++;
            //if(n-i>=0)
				//i++;
			////printf("%c[2K", );

            //printf("%sad%c[2K%c[2Kas",inputs[n-i],27,27);
            //break;
        //case 'B':
            ////printf("B\n");
            //break;
        //case 'C':
            ////printf("C\n");
            //break;
        //case 'D':
            ////printf("D\n");
            //break;
    //}
//}else{
      //sprintf(inputs[n++],"%d\n",ch);
      //printf("%d\n",ch);
 //i=0;}   }while ( 1);//( ch = get_code() ) != KEY_ESC 
//printf("%d\n",ch);
//puts("chuj");
  //return 0;
//}


