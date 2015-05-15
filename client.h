#define CLIENT_DISPLAY_SIZE 128

typedef enum client_display_mode{
	ALL,
	COUNTERS
}client_display_mode;
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

