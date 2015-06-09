//~ #include <stdio.h>
//~ #include <stdlib.h>
//~ #include <string.h>
//~ 
//~ #define MSG_SIZE 111

#define QUEUE_MAX 2048
typedef struct queue_node{
    char info[MSG_SIZE];
    struct queue_node *ptr;
}queue_node;

typedef struct queue{
	queue_node *front;
	queue_node *rear;
	int count;
}queue;

int queue_peek(queue *q, char* buf);
void queue_enq(queue *q, char* buf);
int queue_deq(queue *q, char* buf);
void queue_empty(queue *q);
void queue_display(queue *q);
void queue_create(queue *q);
int queue_size(queue *q);
 
//~ int main()
//~ {
	//~ char buf[MSG_SIZE];
    //~ int ch, e;
	//~ queue q;
    //~ printf("\n 1 - Enque");
    //~ printf("\n 2 - Deque");
    //~ printf("\n 3 - Front element");
    //~ printf("\n 4 - Empty");
    //~ printf("\n 5 - Exit");
    //~ printf("\n 6 - Display");
    //~ printf("\n 7 - Queue size");
    //~ queue_create(&q);
    //~ while (1)
    //~ {
        //~ printf("\n Enter choice : ");
        //~ scanf("%d", &ch);
        //~ switch (ch)
        //~ {
        //~ case 1:
            //~ printf("Enter data : ");
            //~ scanf("%s", buf);
            //~ queue_enq(&q, buf);
            //~ break;
        //~ case 2:
            //~ e = queue_deq(&q, buf);
            //~ puts(buf);
            //~ if (e == 0)
                //~ printf("Front element : %s", buf);
            //~ else
                //~ printf("\n No front element in Queue as queue is empty");
            //~ break;
        //~ case 3:
            //~ e = queue_peek(&q, buf);
            //~ puts(buf);
            //~ if (e == 0)
                //~ printf("Front element : %s", buf);
            //~ else
                //~ printf("\n No front element in Queue as queue is empty");
            //~ break;
        //~ case 4:
            //~ queue_empty(&q);
            //~ break;
        //~ case 5:
            //~ exit(0);
        //~ case 6:
            //~ queue_display(&q);
            //~ break;
        //~ case 7:
            //~ queue_size(&q);
            //~ break;
        //~ default:
            //~ printf("Wrong choice, Please enter correct choice  ");
            //~ break;
        //~ }
    //~ }
    //~ return 0;
//~ }
 
/* Create an empty queue */
void queue_create(queue *q)
{
    q->front = q->rear = NULL;
    q->count = 0;
}
 
/* Returns queue size */
int queue_size(queue *q)
{
    return q->count;
}
 
/* Enqueing the queue */
void queue_enq(queue *q, char* data)
{
    if (q->rear == NULL)
    {
        q->rear = (queue_node *)malloc(1*sizeof(queue_node));
        q->rear->ptr = NULL;
        strncpy(q->rear->info, data, MSG_SIZE);
        q->front = q->rear;
    }
    else
    {
		queue_node *temp;
        temp=(queue_node *)malloc(1*sizeof(queue_node));
        q->rear->ptr = temp;
        strncpy(temp->info, data, MSG_SIZE);
        temp->ptr = NULL;
 
        q->rear = temp;
    }
    q->count+=strlen(data);
    while(q->count>QUEUE_MAX)
		queue_deq(q,data);
}
 
/* Displaying the queue elements */
void queue_display(queue *q)
{
    queue_node *front1 = q->front;
 
    if ((front1 == NULL) && (q->rear == NULL))
    {
        printf("Queue is empty");
        return;
    }
    while (front1 != q->rear)
    {
        printf("%s ", front1->info);
        front1 = front1->ptr;
    }
    if (front1 == q->rear)
        printf("%s", front1->info);
}
 
/* Dequeing the queue */
int queue_deq(queue *q, char* buf)
{
	bzero(buf,MSG_SIZE);
    queue_node *front1 = q->front;
 
    if (front1 == NULL)
    {
        printf("\n Error: Trying to display elements from empty queue");
        return 1;
    }
    else
        if (front1->ptr != NULL)
        {
            front1 = front1->ptr;
            strncpy(buf, q->front->info, MSG_SIZE);
            free(q->front);
            q->front = front1;
        }
        else
        {
            strncpy(buf, q->front->info, MSG_SIZE);
            free(q->front);
            q->front = NULL;
            q->rear = NULL;
        }
        q->count-=strlen(buf);
    return 0;
}
 
/* Returns the front element of queue */
int queue_peek(queue *q, char* buf)
{
	bzero(buf,MSG_SIZE);
    if ((q->front != NULL) && (q->rear != NULL)){
        strncpy(buf, q->front->info, MSG_SIZE);
        return 0;
	}
    else
        return 1;
}
 
/* Display if queue is empty or not */
void queue_empty(queue *q)
{
     if ((q->front == NULL) && (q->rear == NULL))
        printf("\n Queue empty");
    else
       printf("Queue not empty");
}
