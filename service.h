typedef struct service{
	char name[SERVICE_NAME_LENGTH];
	char host_name[HOST_NAME_LENGTH];
	char port_number[PORT_NUMBER_LENGTH];
}service;


void db_row_to_service(char* db_row, service* s ){
	int offset=0;
	char buf[MAX_DB_LINE_LENGTH];
	//printf("%d\n",offset);
	
	bzero(s->name,SERVICE_NAME_LENGTH);
	bzero(buf,MAX_DB_LINE_LENGTH);
	strncpy(buf,db_row + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(s->name,db_row+offset,atoi(buf));
	offset+=atoi(buf);
	//printf("%d\n",offset);
	
	bzero(s->host_name,HOST_NAME_LENGTH);
	bzero(buf,MAX_DB_LINE_LENGTH);
	strncpy(buf,db_row + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(s->host_name,db_row+offset,atoi(buf));
	offset+=atoi(buf);
	//printf("%d\n",offset);
	
	bzero(s->port_number,PORT_NUMBER_LENGTH);
	bzero(buf,MAX_DB_LINE_LENGTH);
	strncpy(buf,db_row + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(s->port_number,db_row+offset,atoi(buf));
}
void service_to_db_row(service* s, char* buf){
	char buf2[MSG_SIZE];
	bzero(buf2,MSG_SIZE);

	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(s->name));
	strncat(buf,buf2,INT_LENGTH+1);
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, SERVICE_NAME_LENGTH+1, "%s", s->name);
	strncat(buf,buf2,SERVICE_NAME_LENGTH+1);
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(s->host_name));
	strncat(buf,buf2,INT_LENGTH+1);
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, HOST_NAME_LENGTH+1, "%s", s->host_name);
	strncat(buf,buf2,HOST_NAME_LENGTH+1);
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(s->port_number));
	strncat(buf,buf2,INT_LENGTH+1);
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, PORT_NUMBER_LENGTH+1, "%s", s->port_number);
	strncat(buf,buf2,PORT_NUMBER_LENGTH+1);
	bzero(buf2,MSG_SIZE);
}


void display_service(service s, char* buf,int n){
	char buf2[n];
	bzero(buf,n);
	bzero(buf2,n);
	snprintf(buf2,n,"name: %s\t",s.name);
	strncat(buf,buf2,n);
	
	bzero(buf2,n);
	snprintf(buf2,n,"host name: %s\t",s.host_name);
	strncat(buf,buf2,n);
	
	bzero(buf2,n);
	snprintf(buf2,n,"port number: %s\t",s.port_number);
	strncat(buf,buf2,n);
	
}



int db_insert_service(service* s){
	FILE *f = fopen(SERVICES_DB_NAME, "a");
	char buf[MAX_DB_LINE_LENGTH];
	service_to_db_row(s, buf);
		printf("%s",buf);

	fwrite(buf,1,strlen(buf),f);
	fclose(f);
	return 0;
}
int db_select_service(char* name, service* s){
	FILE *f;
	printf("name: %s\t%ld\n", name, strlen(name));
	f = fopen(SERVICES_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}
	puts("Plik otwarty pomyslnie!");

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		//printf("line: %s\t%ld\n", buf, strlen(buf));
		db_row_to_service(buf, s);
		//printf("name:%s\t%ld\n", c->name, strlen(c->name));
		if(strcmp(name,s->name)==0){
			printf("znaleziono");
			if (s == NULL){
				fclose(f);
				return 1;
			}
			fclose(f);
			return 0;
		}
		bzero(s->name,NAME_LENGTH);
	}
	puts("nie znaleziono");
	fclose(f);
	return 1;
}
int db_count_services(){
	FILE *f;
	int count = 0;
	f = fopen(SERVICES_DB_NAME, "r");
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
int db_select_services(service* slist, int n){//po tej funkcji trzeba zwalniać. zwraca liczbę elementów
	FILE *f;
	int i = 0;
	f = fopen(SERVICES_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}
	puts("Plik otwarty pomyslnie!");

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		bzero(slist[i].name,NAME_LENGTH);
		db_row_to_service(buf, &slist[i++]);
		bzero(buf,MAX_DB_LINE_LENGTH);
		if(i>=n)
			break;
	}
	fclose(f);
	return 0;
}
int db_delete_service(char* name){
	FILE *oldTodoFile;
	service s;
	if(name==NULL)//poprawić, bo przy nullu segmentation fault
		return 1;
	oldTodoFile = fopen(SERVICES_DB_NAME, "r");

	FILE *todoFile;
	todoFile = fopen(SERVICES_DB_NAME_BUF, "w");
	int lineNumber = 0;
	int len;
	char line[MAX_DB_LINE_LENGTH];

	if (oldTodoFile != NULL) {
		while (fgets(line, sizeof line, oldTodoFile)) {
			len = strlen(line);
			if (len && (line[len - 1] != '\n')) {} else {
				lineNumber++;
				db_row_to_service(line,&s);
				if(strcmp(name,s.name)==0) {
					// Do nothing   
				} else {
					fputs(line, todoFile);
				}
						bzero(s.name,NAME_LENGTH);

			}
		}
	} else {
		printf("ERROR");
	}
	remove(SERVICES_DB_NAME);
	rename(SERVICES_DB_NAME_BUF, SERVICES_DB_NAME);
	fclose(oldTodoFile);
	fclose(todoFile);
return 0;
}
int db_update_service(char* name, service* s){
	if(db_delete_service(name)!=0) return 1;
	if(db_insert_service(s)!=0) return 1;
	return 0;
}






//void service_test()
//{
		//service s,s2,s3;
	//strcpy(s.name,"multicast");
	//strcpy(s.host_name,"127.0.0.1");
	//strcpy(s.port_number,"9998");
	//char buf3[2048];
	//bzero(buf3,2048);
	//service_to_db_row(&s,buf3);
	//printf("%s\n",buf3);
	
	//db_row_to_service(buf3,&s2);
	//bzero(buf3,SERVICE_DISPLAY_SIZE);
	//display_service(s2, buf3,SERVICE_DISPLAY_SIZE);
	//printf("%s\n",buf3);
	
	//printf("%ld\n",strlen(s2.port_number));
	//printf("%ld\n",strlen(s2.host_name));
	
	//db_insert_service(&s);
	//db_delete_service("multicast");
	//bzero(buf3,SERVICE_DISPLAY_SIZE);
	//db_select_service("multicast",&s3);
	//display_service(s3,buf3,SERVICE_DISPLAY_SIZE);
	//puts(buf3);
	
	//return 0;
//}
