#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WIN32__) || defined(_WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "yuvQueue.h"
#include "pch.h"

#ifndef MAX_YUV_BUF_NUM
#define MAX_YUV_BUF_NUM			2
#endif

#ifndef MAX_YUV_BUF_LEN 
#define MAX_YUV_BUF_LEN			(1280*720*3)
#endif

#ifndef MAX_YUV_CH_NUM 
#define MAX_YUV_CH_NUM			3
#endif

//Circular queue
struct _queue{ 
	unsigned char yuvQueBuf[MAX_YUV_BUF_NUM][MAX_YUV_BUF_LEN];
	int font, rear;
	int num;
	int flag;
	int len[MAX_YUV_BUF_NUM];
}; 

typedef struct _yuv_queue
{
	struct _queue channelQueue[MAX_YUV_CH_NUM];
	int channel_num;
}yuv_queue;

#if defined(__WIN32__) || defined(_WIN32)
static HANDLE yuvQueueLock[MAX_YUV_CH_NUM]={NULL,NULL};
#else
static pthread_mutex_t yuvQueueLock;
#endif

static yuv_queue yuvQueue;
static int _have_init_yuv_queue = 0;

void InitYUVQueue()
{
	int i=0;
	if (_have_init_yuv_queue>0)return;
	for(i=0;i<MAX_YUV_CH_NUM;i++)
	{
		yuvQueue.channelQueue[i].font = 0;
		yuvQueue.channelQueue[i].rear = 0;
		yuvQueue.channelQueue[i].num = 0;
#if defined(__WIN32__) || defined(_WIN32)
		yuvQueueLock[i] = CreateMutex(NULL,FALSE,"frame lock");
		if(yuvQueueLock[i]==NULL){
			TRACE("==== %d %d===\n",__LINE__,i);
		}
		Sleep(10);
#else
		pthread_mutex_init(&yuvQueueLock,NULL);
#endif
	}
	_have_init_yuv_queue = 1;
}

void resetYUVQueue()
{
	int i=0;
	for(i=0;i<MAX_YUV_CH_NUM;i++)
	{
		yuvQueue.channelQueue[i].font = 0;
		yuvQueue.channelQueue[i].rear = 0;
		yuvQueue.channelQueue[i].num = 0;
	}
}

void WriteYUVQueue(unsigned char *buffer,int length)
{	
	if(!_have_init_yuv_queue)InitYUVQueue();
	int i =0;
	if(length>MAX_YUV_BUF_LEN)length=MAX_YUV_BUF_LEN;
#if defined(__WIN32__) || defined(_WIN32)
	WaitForSingleObject(yuvQueueLock[i], INFINITE);
#else
	pthread_mutex_lock(&yuvQueueLock[i]);
#endif
	yuvQueue.channelQueue[i].num++;
	if(yuvQueue.channelQueue[i].num>MAX_YUV_BUF_NUM)
	{
		TRACE("***** write %d queue is full!****\n",i);
		yuvQueue.channelQueue[i].font = 0;
		yuvQueue.channelQueue[i].rear = 0;
		yuvQueue.channelQueue[i].num = 1;
	}
	yuvQueue.channelQueue[i].len[yuvQueue.channelQueue[i].rear]=length;
	memset(yuvQueue.channelQueue[i].yuvQueBuf[yuvQueue.channelQueue[i].rear],0,MAX_YUV_BUF_LEN);
	memcpy(yuvQueue.channelQueue[i].yuvQueBuf[yuvQueue.channelQueue[i].rear],buffer,length);
	yuvQueue.channelQueue[i].rear++;

	if(yuvQueue.channelQueue[i].rear>=MAX_YUV_BUF_NUM)
	{
		yuvQueue.channelQueue[i].rear=0;
	}
#if defined(__WIN32__) || defined(_WIN32)
	ReleaseMutex(yuvQueueLock[i]);
#else
	pthread_mutex_unlock(&yuvQueueLock[i]);
#endif
}

int ReadYUVQueue(unsigned char *buffer,int *length)
{
	if(!_have_init_yuv_queue)InitYUVQueue();
	int i = 0;
#if defined(__WIN32__) || defined(_WIN32)
	WaitForSingleObject(yuvQueueLock[i], INFINITE);
#else
	pthread_mutex_lock(&yuvQueueLock[i]);
#endif
	if(yuvQueue.channelQueue[i].num<=0){
#if defined(__WIN32__) || defined(_WIN32)
		ReleaseMutex(yuvQueueLock[i]);
#else
		pthread_mutex_unlock(&yuvQueueLock[i]);
#endif
		return -1;
	}
	*length=yuvQueue.channelQueue[i].len[yuvQueue.channelQueue[i].font];
	memcpy(buffer,yuvQueue.channelQueue[i].yuvQueBuf[yuvQueue.channelQueue[i].font],*length);
	yuvQueue.channelQueue[i].font++;	
	if(yuvQueue.channelQueue[i].font>=MAX_YUV_BUF_NUM)
	{
		yuvQueue.channelQueue[i].font = 0;
	}
	yuvQueue.channelQueue[i].num--;
#if defined(__WIN32__) || defined(_WIN32)
	ReleaseMutex(yuvQueueLock[i]);
#else
	pthread_mutex_unlock(&yuvQueueLock[i]);
#endif
	return 0;
}
