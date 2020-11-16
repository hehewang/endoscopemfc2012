#ifndef __FRAME_QUEUE_H__
#define __FRAME_QUEUE_H__


void InitQueue();
void WriteQueue(unsigned char *buffer,int length);
int ReadQueue(unsigned char *buffer,int *length);
//void UinitMyQueue();
void resetQueue();
#endif
