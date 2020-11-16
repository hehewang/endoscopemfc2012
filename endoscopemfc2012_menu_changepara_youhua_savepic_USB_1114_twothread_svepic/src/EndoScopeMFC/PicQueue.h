#ifndef __PIC_QUEUE_H__
#define __PIC_QUEUE_H__


void InitPicQueue();
void WritePicQueue(unsigned char *buffer,int length);
int ReadPicQueue(unsigned char *buffer,int *length);
//void UinitMyQueue();
void resetPicQueue();
#endif
