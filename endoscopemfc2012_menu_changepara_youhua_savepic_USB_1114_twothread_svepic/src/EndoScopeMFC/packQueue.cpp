#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packQueue.h"
#include "pch.h"

#ifndef SEND_BUF_MAX_NUM 
#define SEND_BUF_MAX_NUM	2
#endif

#ifndef CMD_LENGTH 
#define CMD_LENGTH			1280*720*3
#endif

static HANDLE current_pack_lock = INVALID_HANDLE_VALUE ;

//Circular queue
struct _pack_queue{ 
    int font;
	 int rear;
    int num;
    int len[SEND_BUF_MAX_NUM];
	unsigned char packQueBuf[SEND_BUF_MAX_NUM][CMD_LENGTH];
};  

static struct _pack_queue packQueue;
static int _have_init_pack = 0;
//Init queue
void InitPackQueue()
{
	//printf("init queque!*************************************\n");
	current_pack_lock = CreateMutex(NULL, FALSE, "pack lock");
	DWORD dwRet = GetLastError();
	if(current_pack_lock){
		if (ERROR_ALREADY_EXISTS == dwRet){
			CloseHandle(current_pack_lock);
			current_pack_lock = 0 ;
		}
	}
	packQueue.font = 0;
	packQueue.rear = 0;
	packQueue.num = 0;
	_have_init_pack = 1;
}

//Write Queue
void WritePackQueue(unsigned char *buffer,int length)
{	
	if(_have_init_pack == 0){
		_have_init_pack = 1;
		InitPackQueue();
	}
	if(current_pack_lock) WaitForSingleObject(current_pack_lock, INFINITE);
	packQueue.num++;
	if(packQueue.num>SEND_BUF_MAX_NUM)
	{
		TRACE("fifo is full*******************************8!\n");
		
		packQueue.font = 0;
		packQueue.rear = 0;
		packQueue.num = 1;
		
	}
	packQueue.len[packQueue.rear]=length;
	memcpy(packQueue.packQueBuf[packQueue.rear],buffer,length);
	packQueue.rear++;
	if(packQueue.rear>=SEND_BUF_MAX_NUM)
	{
		packQueue.rear=0;
	}
	if(current_pack_lock) ReleaseMutex(current_pack_lock);
}

int ReadPackQueue(unsigned char *buffer,int *length)
{
	if(_have_init_pack == 0){
		_have_init_pack = 1;
		InitPackQueue();
	}
	*length = 0;
	if(current_pack_lock) WaitForSingleObject(current_pack_lock, INFINITE);
	if(packQueue.num<=0){
		if(current_pack_lock) ReleaseMutex(current_pack_lock);
		return -1;
	}

	*length=packQueue.len[packQueue.font];
	memcpy(buffer,packQueue.packQueBuf[packQueue.font],*length);

	packQueue.font++;	
	if(packQueue.font>=SEND_BUF_MAX_NUM)
	{
		packQueue.font = 0;
	}
	packQueue.num--;
	if(current_pack_lock) ReleaseMutex(current_pack_lock);
	return 0;
}

void ResetPackQueue(){
	if(current_pack_lock) WaitForSingleObject(current_pack_lock, INFINITE);
	packQueue.font = 0;
    packQueue.rear = 0;
    packQueue.num = 0;
	if(current_pack_lock) ReleaseMutex(current_pack_lock);
}
