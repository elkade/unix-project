

static const char CLIENTS_DB_NAME[] = "clients.txt";
static const char CLIENTS_DB_NAME_BUF[] = "clients_buf.txt";
static const char SERVICES_DB_NAME[] = "services.txt";
static const char SERVICES_DB_NAME_BUF[] = "services_buf.txt";

int db_row_to_client(char* db_row, client* c ){
	char buf[MAX_DB_LINE_LENGTH];
	bzero(buf,MAX_DB_LINE_LENGTH);
	puts("DESERIALIZACJA\n");
	
	strncpy(buf,db_row,INT_LENGTH);
	//	printf("buf: %s\n",buf);

	int offset = atoi(buf);
	//printf("offset: %d\n",offset);
	strncpy(c->name,db_row+INT_LENGTH,offset);
			printf("name: %s\n",c->name);
	offset+=INT_LENGTH;
	//	printf("offset: %d\n",offset);
	bzero(buf,MAX_DB_LINE_LENGTH);
	strncpy(buf,db_row+offset,ENUM_LENGTH);
	//		printf("buf2: %s\n",buf);

	c->is_active = atoi(buf);
	offset+=ENUM_LENGTH;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,ENUM_LENGTH);
	//		printf("buf3: %s\n",buf);

	c->tariff_plan = atoi(buf);
	offset+=ENUM_LENGTH;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,INT_LENGTH);
	//		printf("buf4: %s\n",buf);

	c->capacity = atoi(buf);
	offset+=INT_LENGTH;
		bzero(buf,MAX_DB_LINE_LENGTH);

	strncpy(buf,db_row+offset,FLOAT_LENGTH);
	//		printf("buf5: %s\n",buf);

	c->amount = atof(buf);
	//	puts("\nKONIEC\n");

	return 0;
}
int client_to_db_row(client* c, char* buf){
	char buf2[MAX_DB_LINE_LENGTH];
	bzero(buf,MAX_DB_LINE_LENGTH);
	bzero(buf2,MAX_DB_LINE_LENGTH);
		puts("SERIALIZACJA\n");

	//printf("%s\t%ld\n",c->name,strlen(c->name));
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(c->name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, NAME_LENGTH+1, "%s", c->name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, ENUM_LENGTH+1, "%.*d", ENUM_LENGTH, c->is_active);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, ENUM_LENGTH+1, "%.*d", ENUM_LENGTH, c->tariff_plan);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, INT_LENGTH+1, "%.*d", INT_LENGTH, c->capacity);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MAX_DB_LINE_LENGTH);
	snprintf(buf2, FLOAT_LENGTH + 2, "%0*.*f\n", FLOAT_LENGTH -1-2-1,2, c->amount);
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
	printf("szukam klienta: %s\t%ld\n", name, strlen(name));
	f = fopen(CLIENTS_DB_NAME, "r");
	if (f == NULL){
		perror("Nie udalo sie otworzyc pliku notatki.txt");
		return 1;
	}
	//puts("Plik otwarty pomyslnie!");

	char buf[MAX_DB_LINE_LENGTH];
	while (fgets (buf, sizeof(buf), f)) {
		//printf("line: %s\t%ld\n", buf, strlen(buf));
		db_row_to_client(buf, c);
		//printf("name:%s\t%ld\n", c->name, strlen(c->name));
		if(strcmp(name,c->name)==0){
			puts("znaleziono");
			if (c == NULL){
				fclose(f);
				puts("nie znaleziono");
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

