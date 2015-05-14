#include "applist.h"

/*
应用载入的测试函数
*/
void main(){
	int c,i,j,l;
	c=loadAppList();

	for(i=0;i<c;i++)
	{
		printf("[%d]\n",i+1);
		printf("ID %s\n",applist[i].id);
		printf("HIDDEN %s\n",applist[i].hidden);
		printf("PATH %s\n",applist[i].path);
		//由于和下载有交互，所以编码跟着pc端
		printf("NAME ");
		l=strlen(applist[i].name);
		for(j=0;j<l;j++)
			printf("%02x ",applist[i].name[j]);
		printf("\n\n");
	}
}
