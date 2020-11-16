#ifndef __PACK_QUEUE_H__
#define __PACK_QUEUE_H__

void WritePackQueue(unsigned char *buffer,int length);
int ReadPackQueue(unsigned char *buffer,int *length);
#endif
