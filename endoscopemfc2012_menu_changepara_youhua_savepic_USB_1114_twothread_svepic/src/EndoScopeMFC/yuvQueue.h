#ifndef __YUV_QUEUE_H__
#define __YUV_QUEUE_H__

void WriteYUVQueue(unsigned char *buffer,int length);
int ReadYUVQueue(unsigned char *buffer,int *length);
#endif
