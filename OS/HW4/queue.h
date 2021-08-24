#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct __node_t //노드 정의
{
    char *data;
    struct __node_t *next;
}node_t;
 
 
typedef struct __queue_t //Queue 구조체 정의
{
    node_t *head; //맨 앞(꺼낼 위치)
    node_t *tail; //맨 뒤(보관할 위치)
    int count;//보관 개수
    pthread_mutex_t head_lock, tail_lock;

}queue_t;

void Queue_Init(queue_t *q)
{
    node_t *temp = malloc(sizeof(node_t));
    temp->next = NULL;
    q->head = q->tail = temp;
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
    q->count = 0;
}
 
int IsEmpty(queue_t *queue)
{
    return queue->count == 0;    //보관 개수가 0이면 빈 상태
}
 
void Enqueue(queue_t *q, char *data)
{
    node_t *temp = malloc(sizeof(node_t));
    temp->data = data;
    temp->next = NULL;
    pthread_mutex_lock(&q->tail_lock);
    q->tail->next = temp;
    q->tail = temp;
    q->count++;//보관 개수를 1 증가
    pthread_mutex_unlock(&q->tail_lock);
   
}
 
char* Dequeue(queue_t *q)
{
    pthread_mutex_lock(&q->head_lock);
    node_t *temp = q->head;
    node_t *new_head = temp->next;
    if(new_head == NULL){
        pthread_mutex_unlock(&q->head_lock);
        return NULL;
    }
    char *data = new_head->data;
    q->head = new_head;
    q->count--;
    pthread_mutex_unlock(&q->head_lock);
    free(temp);
    return data;
}