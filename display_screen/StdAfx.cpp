// stdafx.cpp : source file that includes just the standard includes
//	BGS��λ��ʱ�豸.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

void write_log(char *file_name,const char *fmt,...)
{
	FILE *fp = NULL;
//	fp = fopen("speed_message.log","a+");
	fp = fopen(file_name,"a+");
	if(fp == NULL)
		return;
	va_list ap;
	va_start(ap,fmt);
	vfprintf(fp,fmt,ap);//ת������ʽ�����ݣ�����ʵ��������ַ���
	va_end(ap);

	fclose(fp);
	fp = NULL;
}



