#define SET_MAX 128

typedef struct single_set_elem{
	int fd;
	char name[SERVICE_NAME_LENGTH];
	bool is_empty;
}single_set_elem;

typedef struct single_set{
	single_set_elem arr[SET_MAX];
	int n;//size
}single_set;



typedef struct double_set_elem{
	//int fd;
	char name[NAME_LENGTH];
	bool is_empty;
	single_set sset;
}double_set_elem;

typedef struct double_set{
	double_set_elem arr[SET_MAX];
	int n;//size
}double_set;







void single_set_init(single_set *ss, int n){
	int i;
	if(n>SET_MAX)
		ERR("set:");
	ss->n = n;
	for (i = 0; i < ss->n; i++){
		ss->arr[i].is_empty = true;
	}
}
int single_set_select_by_name(single_set *ss, char* name, single_set_elem **sse){
	int i;
	for (i = 0; i < ss->n; i++){
		if(!ss->arr[i].is_empty && strcmp(ss->arr[i].name,name)==0){
			*sse = &ss->arr[i];
			return 0;
		}
	}
	return 1;
}
int single_set_add_by_name(single_set *ss, char* name){
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
int single_set_add_by_fd(single_set *ss, int fd){
	int i;
	for (i = 0; i < ss->n; i++){
		if(ss->arr[i].is_empty){
			ss->arr[i].fd = fd;
			ss->arr[i].is_empty = false;
			return 0;
		}
	}
	return 1;
}
int single_set_update_by_name(single_set *ss, char* name, int fd){
	int i;
	for (i = 0; i < ss->n; i++){
		if(!ss->arr[i].is_empty && strcmp(ss->arr[i].name,name)==0){
			ss->arr[i].fd = fd;
			return 0;
		}
	}
	return 1;
}
int single_set_update_by_fd(single_set *ss, int fd, char* name){
	int i;
	for (i = 0; i < ss->n; i++){
		if(!ss->arr[i].is_empty && ss->arr[i].fd == fd){
			bzero(ss->arr[i].name,NAME_LENGTH);
			strcpy(ss->arr[i].name,name);
			return 0;
		}
	}
	return 1;
}
/*
 * Removes all elements from s equal to val.
 * Returns number of removed elements.
 */
int single_set_remove_by_name(single_set *ss, char* name){
	int i, c = 0;
	for (i = 0; i < ss->n; i++){
		if(strcmp(ss->arr[i].name, name)==0 && !ss->arr[i].is_empty){
			ss->arr[i].is_empty = true;
			c++;
		}
	}
	return c;
}
int single_set_remove_by_fd(single_set *ss, int fd){
	int i, c = 0;
	for (i = 0; i < ss->n; i++){
		if(ss->arr[i].fd==fd && !ss->arr[i].is_empty){
			ss->arr[i].is_empty = true;
			c++;
		}
	}
	return c;
}

int single_set_count(single_set *ss){
	int i, c = 0;
	for (i = 0; i < ss->n; i++)
		if(ss->arr[i].is_empty == true)
			c++;
	return c;
}





void double_set_init(double_set *cs, int n){
	int i;
	if(n>SET_MAX)
		ERR("set:");
	cs->n = n;
	for (i = 0; i < cs->n; i++){
		cs->arr[i].is_empty = true;
	}
}

int double_set_add_by_name(double_set *cs, char* name, int n){
	int i;
	for (i = 0; i < cs->n; i++){
		if(cs->arr[i].is_empty){
			bzero(cs->arr[i].name,NAME_LENGTH);
			strcpy(cs->arr[i].name,name);
			cs->arr[i].is_empty = false;
			single_set_init(&cs->arr[i].sset, n);
			return 0;
		}
	}
	return 1;
}
int double_set_add_by_fd(double_set *cs, int fd, int n){
	int i;
	for (i = 0; i < cs->n; i++){
		if(cs->arr[i].is_empty){
			//cs->arr[i].fd = fd;
			cs->arr[i].is_empty = false;
			single_set_init(&cs->arr[i].sset, n);
			return 0;
		}
	}
	return 1;
}
//~ int double_set_update_by_name(double_set *cs, char* name, int fd){
	//~ int i;
	//~ for (i = 0; i < cs->n; i++){
		//~ if(!cs->arr[i].is_empty && strcmp(cs->arr[i].name,name)==0){
			//~ cs->arr[i].fd = fd;
			//~ return 0;
		//~ }
	//~ }
	//~ return 1;
//~ }
int double_set_select_by_name(double_set *cs, char* name, double_set_elem** dse){
	int i;
	for (i = 0; i < cs->n; i++){
		if(!cs->arr[i].is_empty && strcmp(cs->arr[i].name,name)==0){
			*dse = &cs->arr[i];
			return 0;
		}
	}
	return 1;
}
//~ int double_set_update_by_fd(double_set *cs, int fd, char* name){
	//~ int i;
	//~ for (i = 0; i < cs->n; i++){
		//~ if(!cs->arr[i].is_empty && cs->arr[i].fd == fd){
			//~ bzero(cs->arr[i].name,NAME_LENGTH);
			//~ strcpy(cs->arr[i].name,name);
			//~ return 0;
		//~ }
	//~ }
	//~ return 1;
//~ }
/*
 * Removes all elements from s equal to val.
 * Returns number of removed elements.
 */
int double_set_remove_by_name(double_set *cs, char* name){
	int i, c = 0;
	for (i = 0; i < cs->n; i++){
		if(strcmp(cs->arr[i].name, name)==0 && !cs->arr[i].is_empty){
			cs->arr[i].is_empty = true;
			c++;
		}
	}
	return c;
}
//~ int double_set_remove_by_fd(double_set *cs, int fd){
	//~ int i, c = 0;
	//~ for (i = 0; i < cs->n; i++){
		//~ if(cs->arr[i].fd==fd && !cs->arr[i].is_empty){
			//~ cs->arr[i].is_empty = true;
			//~ c++;
		//~ }
	//~ }
	//~ return c;
//~ }
int double_set_count(double_set *cs){
	int i, c = 0;
	for (i = 0; i < cs->n; i++)
		if(cs->arr[i].is_empty == true)
			c++;
	return c;
}
