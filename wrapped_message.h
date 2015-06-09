#define MSG_CONTENT_MAX_SIZE 8192


typedef enum message_status{
	REGULAR,
	DEREGISTER,
	SERVICE_DISCONNECTED,
	REGISTER
}message_status;

typedef struct wrapped_message{
	char service_name[SERVICE_NAME_LENGTH];
	char client_name[NAME_LENGTH];
	char app_name[APP_NAME_LENGTH];
	int content_length;
	char content[MSG_SIZE];
	message_status status; 
}wrapped_message;

void read_string(char* dest, char* src, int n, int *offset){
	char buf[INT_LENGTH];
	bzero(buf,INT_LENGTH);
	
	bzero(dest, n);
	strncpy(buf, src + *offset, INT_LENGTH);
	*offset += INT_LENGTH;
	strncpy(dest, src + *offset, atoi(buf));
	*offset+=atoi(buf);
}

void read_int(int* dest, char* src, int n, int *offset){
	char buf[INT_LENGTH];
	bzero(buf,INT_LENGTH);
	
	bzero(buf,INT_LENGTH);
	strncpy(buf, src + *offset, INT_LENGTH);
	*offset += INT_LENGTH;
	*dest = atoi(buf);
}
void append_string(char *dest, char* src, int n){
	char buf2[MSG_CONTENT_MAX_SIZE];
	
	printf("%s\n",src);
	
	bzero(buf2,MSG_CONTENT_MAX_SIZE);
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(src));
	strncat(dest,buf2,strlen(buf2));
	
	bzero(buf2,MSG_CONTENT_MAX_SIZE);
	snprintf(buf2, n+1, "%s", src);
	strncat(dest,buf2,strlen(buf2));
}

void append_int(char *dest, int i){
	char buf2[MSG_CONTENT_MAX_SIZE];
	
	bzero(buf2,MSG_CONTENT_MAX_SIZE);
	snprintf(buf2, INT_LENGTH+1, "%0*d",INT_LENGTH, i);
	strncat(dest,buf2,strlen(buf2));
}

void str_to_wrapped_message(char* message, wrapped_message *msg, int n){
	int offset=0;
	
	read_string(msg->client_name, message, NAME_LENGTH, &offset);
	read_string(msg->service_name, message, SERVICE_NAME_LENGTH, &offset);
	read_string(msg->app_name, message, APP_NAME_LENGTH, &offset);
	read_int((int*)&msg->status, message, INT_LENGTH, &offset);
	//read_int(&msg->content_length, message, INT_LENGTH, &offset);
	read_string(msg->content, message, MSG_CONTENT_SIZE, &offset);
}
void str_to_header(char* message, wrapped_message *msg, int n){//to samo tylko bez kontentu
	int offset=0;
	
	read_string(msg->client_name, message, NAME_LENGTH, &offset);
	read_string(msg->service_name, message, SERVICE_NAME_LENGTH, &offset);
	read_string(msg->app_name, message, APP_NAME_LENGTH, &offset);
	read_int((int*)&msg->status, message, INT_LENGTH, &offset);
	read_int(&msg->content_length, message, INT_LENGTH, &offset);
	bzero(msg->content, msg->content_length);
}
void wrapped_message_to_str(char* buf, wrapped_message msg, int n){
	bzero(buf,MSG_SIZE);
	
	append_string(buf, msg.client_name, NAME_LENGTH);
	append_string(buf, msg.service_name, SERVICE_NAME_LENGTH);
	append_string(buf, msg.app_name, APP_NAME_LENGTH);
	append_int(buf, msg.status);
	//append_int(buf, msg.content_length);
	append_string(buf, msg.content, MSG_CONTENT_SIZE);
}
void header_to_str(char* buf, wrapped_message msg, int n){
	bzero(buf,MSG_SIZE);
	
	append_string(buf, msg.client_name, NAME_LENGTH);
	append_string(buf, msg.service_name, SERVICE_NAME_LENGTH);
	append_string(buf, msg.app_name, APP_NAME_LENGTH);
	append_int(buf, msg.status);
	append_int(buf, msg.content_length);
}

#define MSG_CONTENT_MAX_SIZE 8192
#define MSG_HEADER_SIZE (SERVICE_NAME_LENGTH + NAME_LENGTH + APP_NAME_LENGTH + INT_LENGTH)
#define INT_LENGTH 16

//jeżeli zwraca < 0 to należy odłączyć fd 
int comm_send(int fd, wrapped_message *msg){//msg musi być kompletne i spójne
	int count = 0;
	
	char header_len_buf[INT_LENGTH+1];
	bzero(header_len_buf,INT_LENGTH+1);
	char header_buf[MSG_HEADER_SIZE+1];
	bzero(header_len_buf,MSG_HEADER_SIZE+1);

	int header_len = MSG_HEADER_SIZE;
	append_int(header_len_buf, header_len);
	if((count += bulk_write(fd, header_len_buf, INT_LENGTH)) <= 0)
		return -1;
		
	header_to_str(header_buf,*msg, MSG_SIZE);
	
	if((count += bulk_write(fd, header_buf, header_len)) <= 0)
		return -1;

	if((count += bulk_write(fd, msg->content, msg->content_length)) <= 0)
		return -1;
	return count;
}

int comm_recv(int fd, wrapped_message *msg){
	int count = 0;
	char header_len_buf[INT_LENGTH];
	bzero(header_len_buf,INT_LENGTH);
	char header_buf[MSG_HEADER_SIZE];
	bzero(header_len_buf,MSG_HEADER_SIZE);
	
	if((count += bulk_read(fd, header_len_buf, INT_LENGTH)) <= 0)
		return -1;
	int header_len = atoi(header_len_buf);
	if((count += bulk_read(fd, header_buf, header_len)) <= 0)
		return -1;
	str_to_header(header_buf, msg, MSG_SIZE);//w tym niech będzie bzero na content
	if((count += bulk_read(fd, msg->content, msg->content_length)) <= 0)
		return -1;
	return count;
}
