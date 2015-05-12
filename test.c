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

int insert_client(client* c){
	FILE *f = fopen(CLIENTS_DB_NAME, "a");
	char buf[MAX_DB_LINE_LENGTH];
	client_to_db_row(c, buf);
		printf("%s",buf);

	fwrite(buf,1,strlen(buf),f);
	fclose(f);
	return 0;
}

int select_client(char* name, client* c){//zmienić na inta
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

int delete_client(char* name){
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

int update_client(char* name, client* c){
	if(delete_client(name)!=0) return 1;
	if(insert_client(c)!=0) return 1;
	return 0;
}

int auth(char* input, role who){
	char owner_name[NAME_LENGTH] = "admin";
	client c;
	switch (who)
	{
		case USER:
			//sprawdzić, czy input jest na liście klientów
			if(select_client(input, &c)==0){
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
	client c;
	if(select_client(name,&c)==1) return;
	printf("\nhello %s",c.name);
	printf("\n%d",c.is_active);
	printf("\n%d",c.tariff_plan);
	printf("\n%d",c.capacity);
	printf("\n%f",c.amount);
}

int main(int argc, char** argv){
	char in[NAME_LENGTH];
	bzero(in,NAME_LENGTH);
	client c;
	fgets(in,NAME_LENGTH, stdin);
	strncpy(c.name,in,NAME_LENGTH);
	c.amount=1.6;
	c.capacity=45654;
	c.tariff_plan = SUBSCRIPTION;
	c.is_active=true;
		printf("%s\t%ld\n",c.name,strlen(c.name));

	//update_client("agfsaf",&c);
	handle_client(in);
	return 0;
	if(auth(in, USER)==0){
		puts("OK");
		handle_client(in);
	}
	else
		puts("failed");
	exit(EXIT_SUCCESS);
}
/*
 * 0000000000000006agfsaf000100010000000000045654000000000000000000000000000000000000000001.60
0000000000000003qwe000100010000000000045654000000000000000000000000000000000000000001.60
0000000000000004erte000100010000000000045654000000000000000000000000000000000000000001.60
 * */
