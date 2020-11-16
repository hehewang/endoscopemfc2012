#pragma once
#include "time.h"
/*  
*   函数用于计算彩色图像的白平衡系数，系数用Mat结构存储
*   输入：图像，图像高，图像宽
*   输出：系数
*   耗时：18ms左右
*/

void CalculateLamda_RB(unsigned char *pBuffer, int width, int height, double** lamda_rgb)
{
	//假设输入的图像为BGR
	double*pBuffer_dst =*lamda_rgb;
	int pos = 0;
	for (int i = 0;i < height;++i)
	{
		for (int j = 0;j < width;++j)
		{
			//防止分母是0，取最小值
			pos = i * width * 3 + j * 3;
			if (0 == pBuffer[pos + 0])
				pBuffer[pos + 0] = 1;
			if (0 == pBuffer[pos + 2])
				pBuffer[pos + 2] = 1;

			pBuffer_dst[pos + 0] = (double)pBuffer[pos + 1] / (double)pBuffer[pos+0];//Lamda_B=G/B
			pBuffer_dst[pos + 2] = (double)pBuffer[pos + 1] / (double)pBuffer[pos+2];//Lamda_R=G/R
			pBuffer_dst[pos + 1] = 1.0;//Lamda_G=G/G

		}
	}
}
/*
*	函数用来对图像进行白平衡矫正
*	输入：原图像，系数矩阵，图像宽，图像高
*	输出：目标图像
*   注意：目标图像的内存空间要在外部声明
*/
void AdjWhiteBalance(unsigned char* src,double *lamda,int width,int height,uchar** dst)
{
	int pos = 0;
	uchar *t_dst = *dst;
	for (int i = 0;i < height;++i)
	{
		for (int j = 0;j < width;++j)
		{
			pos = i * width * 3 + j * 3;
			t_dst[pos + 0] = (int)(src[pos + 0] * lamda[pos + 0]);
			t_dst[pos + 1] = (int)(src[pos + 1] * lamda[pos + 1]);
			t_dst[pos + 2] = (int)(src[pos + 2] * lamda[pos + 2]);
		}
	}
}