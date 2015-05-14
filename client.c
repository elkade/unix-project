#include <stdio.h> 
#include <termios.h>
#include <stdio.h>
#include <string.h>

#define SIZE 8

static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche(void) 
{
  return getch_(1);
}

void delete(int n){
	int i;
	printf("%c[2K",27);
	for (i = 0; i < n; i++){
		printf("\b");
	}
}

void read_line(char* buf, int m){
	static char inputs[SIZE][SIZE];
	static int n;
	char c;
	int i=0, j=0, k=0;
	bzero(buf,m);
	
	n++;
	n %= SIZE;

	strncpy(inputs[n],buf,SIZE);

	
	while((c=getch())!='\n'){
		if(c=='\033'){
			delete(k);
			k=0;
			bzero(buf,m);
			i=0;
			if((c=getch())!=91)
				continue;
			if((c=getch())=='A')
				j=(j-1+SIZE)%SIZE;
			else if(c=='B')
				j=(j+1+SIZE)%SIZE;
			else
				continue;
			strncpy(buf,inputs[(n+j+SIZE)%SIZE],SIZE);
			i=k=strlen(buf);
			delete(i);
			printf("%s",buf);
		}
		else if(c==127){
			delete(k);//to jest słabe
			if(k>0)
				k--;
			if(i>0)
				i--;
			buf[i]='\0';
			strncpy(inputs[n],buf,SIZE);
			printf("%s",buf);
		}
		else{
			k++;
			printf("%c",c);
			if(i<m){
				buf[i++]=c;
				if(i<m)
					buf[i]='\0';
			}
			strncpy(inputs[n],buf,SIZE);
		}
	}
	strncpy(inputs[n],buf,SIZE);
	puts("");
}

int main ( void ){
	char buf[8];
	while(1){
	read_line(buf,8);
	printf("%s\n",buf);
	}
	return 0;
	delete(3);
	char inputs[128][128];
  int ch, i = 0, n=0;
//while ( (ch=getch()) != EOF )printf("%d\n",ch);
  do {
if ((ch=getch() )== '\033') { // if the first value is esc
	//printf("%d\n",ch);
	fflush(stdin);
    if((ch=getch())!=91){
		//printf("%d\n",ch);
		continue; // skip the [
	}
    //printf("%d\n",ch);
    switch((ch=getch())) { // the real value
        case 'A':
            //printf("A\n");
            //n++;
            if(n-i>=0)
				i++;
			//printf("%c[2K", );

            printf("%sad%c[2K%c[2Kas",inputs[n-i],27,27);
            break;
        case 'B':
            //printf("B\n");
            break;
        case 'C':
            //printf("C\n");
            break;
        case 'D':
            //printf("D\n");
            break;
    }
}else{
      sprintf(inputs[n++],"%d\n",ch);
      printf("%d\n",ch);
 i=0;}   }while ( 1);//( ch = get_code() ) != KEY_ESC 
printf("%d\n",ch);
puts("chuj");
  return 0;
}


