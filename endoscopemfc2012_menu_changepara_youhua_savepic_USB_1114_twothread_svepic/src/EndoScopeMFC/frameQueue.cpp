#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WIN32__) || defined(_WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "frameQueue.h"
#include "pch.h"

#ifndef MAX_BUF_NUM
#define MAX_BUF_NUM			1
#endif

#ifndef MAX_BUF_LEN 
#define MAX_BUF_LEN			1280*720*3
#endif

#ifndef MAX_CHANNEL_NUM 
#define MAX_CHANNEL_NUM			3
#endif

//Circular queue
struct _queue{ 
	unsigned char queueBuf[MAX_BUF_NUM][MAX_BUF_LEN];
	int font, rear;
	int num;
	int flag;
	int len[MAX_BUF_NUM];
}; 

typedef struct _multi_queue
{
	struct _queue channelQueue[MAX_CHANNEL_NUM];
	int channel_num;
}multi_queue;

#if defined(__WIN32__) || defined(_WIN32)
static HANDLE queueLock[MAX_CHANNEL_NUM]={NULL,NULL};
#else
static pthread_mutex_t queueLock;
#endif

static multi_queue dataQueue;
static int _have_init_fifo = 0;

void InitQueue()
{
	int i=0;
	if (_have_init_fifo>0)return;
	for(i=0;i<MAX_CHANNEL_NUM;i++)
	{
		dataQueue.channelQueue[i].font = 0;
		dataQueue.channelQueue[i].rear = 0;
		dataQueue.channelQueue[i].num = 0;
#if defined(__WIN32__) || defined(_WIN32)
		queueLock[i] = CreateMutex(NULL,FALSE,"frame lock");
		if(queueLock[i]==NULL){
			TRACE("==== %d %d===\n",__LINE__,i);
		}
		Sleep(10);
#else
		pthread_mutex_init(&queueLock,NULL);
#endif
	}
	_have_init_fifo = 1;
}

void resetQueue()
{
	int i=0;
	for(i=0;i<MAX_CHANNEL_NUM;i++)
	{
		dataQueue.channelQueue[i].font = 0;
		dataQueue.channelQueue[i].rear = 0;
		dataQueue.channelQueue[i].num = 0;
	}
}

void WriteQueue(unsigned char *buffer,int length)
{	
	if(!_have_init_fifo)InitQueue();
	int i =0;
	if(length>MAX_BUF_LEN)length=MAX_BUF_LEN;
#if defined(__WIN32__) || defined(_WIN32)
	WaitForSingleObject(queueLock[i], INFINITE);
#else
	pthread_mutex_lock(&queueLock[i]);
#endif
	dataQueue.channelQueue[i].num++;
	if(dataQueue.channelQueue[i].num>MAX_BUF_NUM)
	{
		TRACE("***** write queue is full!****\n");
		dataQueue.channelQueue[i].font = 0;
		dataQueue.channelQueue[i].rear = 0;
		dataQueue.channelQueue[i].num = 1;
	}
	dataQueue.channelQueue[i].len[dataQueue.channelQueue[i].rear]=length;
	memset(dataQueue.channelQueue[i].queueBuf[dataQueue.channelQueue[i].rear],0,MAX_BUF_LEN);
	memcpy(dataQueue.channelQueue[i].queueBuf[dataQueue.channelQueue[i].rear],buffer,length);
	dataQueue.channelQueue[i].rear++;

	if(dataQueue.channelQueue[i].rear>=MAX_BUF_NUM)
	{
		dataQueue.channelQueue[i].rear=0;
	}
#if defined(__WIN32__) || defined(_WIN32)
	ReleaseMutex(queueLock[i]);
#else
	pthread_mutex_unlock(&queueLock[i]);
#endif
}

int ReadQueue(unsigned char *buffer,int *length)
{
	if(!_have_init_fifo)InitQueue();
	int i = 0;
#if defined(__WIN32__) || defined(_WIN32)
	WaitForSingleObject(queueLock[i], INFINITE);
#else
	pthread_mutex_lock(&queueLock[i]);
#endif
	if(dataQueue.channelQueue[i].num<=0){
#if defined(__WIN32__) || defined(_WIN32)
		ReleaseMutex(queueLock[i]);
#else
		pthread_mutex_unlock(&queueLock[i]);
#endif
		return -1;
	}
	*length=dataQueue.channelQueue[i].len[dataQueue.channelQueue[i].font];
	memcpy(buffer,dataQueue.channelQueue[i].queueBuf[dataQueue.channelQueue[i].font],*length);
	dataQueue.channelQueue[i].font++;	
	if(dataQueue.channelQueue[i].font>=MAX_BUF_NUM)
	{
		dataQueue.channelQueue[i].font = 0;
	}
	dataQueue.channelQueue[i].num--;
#if defined(__WIN32__) || defined(_WIN32)
	ReleaseMutex(queueLock[i]);
#else
	pthread_mutex_unlock(&queueLock[i]);
#endif
	return 0;
}
