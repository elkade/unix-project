typedef enum message_status{
	REGULAR,
	DEREGISTER
}message_status;

typedef struct wrapped_message{
	char service_name[SERVICE_NAME_LENGTH];
	char client_name[NAME_LENGTH];
	char app_name[APP_NAME_LENGTH];
	char content[MSG_SIZE];
	message_status status; 
}wrapped_message;

void str_to_wrapped_message(char* message, wrapped_message *msg, int n){
	
	int offset=0;
	char buf[MSG_SIZE];
	//printf("%d\n",offset);
	bzero(buf,MSG_SIZE);
	
	bzero(msg->service_name,SERVICE_NAME_LENGTH);
	strncpy(buf,message + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(msg->service_name,message+offset,atoi(buf));
	offset+=atoi(buf);
	//printf("%d\n",offset);
	
	bzero(msg->client_name,NAME_LENGTH);
	bzero(buf,MSG_SIZE);
	strncpy(buf,message + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(msg->client_name,message+offset,atoi(buf));
	offset+=atoi(buf);
	//printf("%d\n",offset);
	
	bzero(msg->content,MSG_CONTENT_SIZE);
	bzero(buf,MSG_SIZE);
	strncpy(buf,message + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	strncpy(msg->content,message+offset,atoi(buf));
	offset+=atoi(buf);
	
	
	bzero(buf,MSG_SIZE);
	strncpy(buf,message + offset,INT_LENGTH);
	offset += INT_LENGTH;
	//printf("%d\n",atoi(buf));
	msg->status = atoi(buf);
}

void wrapped_message_to_str(char* buf, wrapped_message msg, int n){
	char buf2[MSG_SIZE];
	bzero(buf2,MSG_SIZE);

	bzero(buf,MSG_SIZE);

	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.service_name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, SERVICE_NAME_LENGTH+1, "%s", msg.service_name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.client_name));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, NAME_LENGTH+1, "%s", msg.client_name);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%.*ld", INT_LENGTH , strlen(msg.content));
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	snprintf(buf2, MSG_CONTENT_SIZE+1, "%s", msg.content);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	
	snprintf(buf2, INT_LENGTH+1, "%0*d",INT_LENGTH, msg.status);
	strncat(buf,buf2,strlen(buf2));
	bzero(buf2,MSG_SIZE);
	}
