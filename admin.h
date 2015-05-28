
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
	if(args==NULL){
		puts("unhandled message");
		strcpy(response,"unhandled message");
		return 1;
	}
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
