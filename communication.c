//~ #include "header.h"
//~ #include "wrapped_message.h"
//~ 
//~ #define MSG_CONTENT_MAX_SIZE 8192
//~ #define MSG_HEADER_SIZE (SERVICE_NAME_LENGTH + NAME_LENGTH + APP_NAME_LENGTH + INT_LENGTH)
//~ #define INT_LENGTH 16
//~ 
//~ //jeżeli zwraca < 0 to należy odłączyć fd 
//~ int comm_send(int fd, wrapped_message *msg){
	//~ return 0;
//~ }
//~ 
//~ int comm_recv(int fd, wrapped_message *msg){
	//~ int count = 0;
	//~ char header_len_buf[INT_LENGTH];
	//~ char header_buf[MSG_HEADER_SIZE];
	//~ 
	//~ if((count += bulk_read(fd, header_len_buf, INT_LENGTH)) <= 0)
		//~ return -1;
	//~ int header_len = atoi(header_len_buf);
	//~ if((count += bulk_read(fd, header_buf, header_len)) <= 0)
		//~ return -1;
	//~ str_to_header(header_buf, msg, MSG_SIZE);//w tym niech będzie bzero na content
	//~ if((count += bulk_read(fd, msg->content, msg->content_length)) <= 0)
		//~ return -1;
	//~ return count;
//~ }
