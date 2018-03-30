
// producer_consumer.cpp
//////////////////////////////////////////////////////////////////////
// 有一个生产者在生产产品，这些产品将提供给若干个消费者去消费，为了使生产者和消费者能并发执行，
// 在两者之间设置一个有多个缓冲区的缓冲池，生产者将它生产的产品放入一个缓冲区中，消费者可以从缓
// 冲区中取走产品进行消费，所有生产者和消费者都是异步方式运行的，但它们必须保持同步，即不允许消
// 费者到一个空的缓冲区中取产品，也不允许生产者向一个已经装满产品且尚未被取走的缓冲区中投放产品。
//////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


const int BUFFER_LENGTH = 10;
int buffer[BUFFER_LENGTH];
int front = 0, rear = -1; // 缓冲区的前端和尾端
int size = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg);
void *consumer(void *arg);

int main(int argc, char **argv)
{
    pthread_t producer_id;
    pthread_t consumer_id;

    pthread_create(&producer_id, NULL, producer, NULL);
    
    pthread_create(&consumer_id, NULL, consumer, NULL);
    
	//主线程运行结束，子线程也就运行结束了
	//http://bbs.chinaunix.net/thread-1286290-1-1.html
    sleep(1);

    return 0;
}

void *producer(void *arg)
{    
   //pthread_detach(threadid)函数的功能是使线程ID为threadid的线程处于分离状态，一旦线程处于分离状态，
  //该线程终止时底层资源立即被回收；否则终止子线程的状态会一直保存（占用系统资源）直到主线程调用pthread_join(threadid,NULL)获取线程的退出状态。
  //通常是主线程使用pthread_create()创建子线程以后，一般可以调用pthread_detach(threadid)分离刚刚创建的子线程，
 
  //这里的threadid是指子线程的threadid；如此以来，该子线程止时底层资源立即被回收；
  //被创建的子线程也可以自己分离自己，子线程调用pthread_detach(pthread_self())就是分离自己，
  //因为pthread_self()这个函数返回的就是自己本身的线程ID；
    pthread_detach(pthread_self());
    
    while (true)
    {
        pthread_mutex_lock(&mutex);
        while (size == BUFFER_LENGTH) // 如果缓冲区已满，等待; 否则，添加新产品
        {
            printf("buffer is full. producer is waiting...\n");
            pthread_cond_wait(&cond, &mutex);
        }
        // 往尾端添加一个产品
        rear = (rear + 1) % BUFFER_LENGTH;
        buffer[rear] = rand() % BUFFER_LENGTH;
        printf("producer produces the item %d: %d\n", rear, buffer[rear]);
        ++size;
        if (size == 1) // 如果当前size=1, 说明以前size=0, 消费者在等待，则给消费者发信号
        {
           pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg)
{
    pthread_detach(pthread_self());
    
    while (true)
    {
        pthread_mutex_lock(&mutex);
        while(size == 0) // 如果缓冲区已空，等待; 否则，消费产品
        {
            printf("buffer is empty. consumer is waiting...\n");
            pthread_cond_wait(&cond, &mutex);
        }
        // 从前端消费一个产品
        printf("consumer consumes an item%d: %d\n", front, buffer[front]);
        front = (front + 1) % BUFFER_LENGTH;
        --size;
        if (size == BUFFER_LENGTH-1) // 如果当前size=BUFFER_LENGTH-1，说明以前生产者在等待，则给生产者发信号
        {
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
}


----
运行效果：
buffer is empty. consumer is waiting...
producer produces the item 9: 8
producer produces the item 0: 5
producer produces the item 1: 2
producer produces the item 2: 0
producer produces the item 3: 7
producer produces the item 4: 4
producer produces the item 5: 6
producer produces the item 6: 0
producer produces the item 7: 4
producer produces the item 8: 8
buffer is full. producer is waiting...
consumer consumes an item9: 8
consumer consumes an item0: 5
consumer consumes an item1: 2
consumer consumes an item2: 0
consumer consumes an item3: 7
consumer consumes an item4: 4
consumer consumes an item5: 6
consumer consumes an item6: 0
consumer consumes an item7: 4
consumer consumes an item8: 8
buffer is empty. consumer is waiting...
