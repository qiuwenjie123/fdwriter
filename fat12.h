#ifndef _FAT12_H_
#define _FAT12_H_

typedef unsigned short ushort;
typedef unsigned char uchar;

//已经存在的目录项(链表结构) 
typedef struct _catbuffer { 
	int startclu;   //开始簇号
	struct _catbuffer *next; //指向下一个 
}Catalog;


//1.44m最大保存能保存的文件的簇数量 
#define Max_CluSum 			0xB1F
#define BytsPerSec			0x200	//每扇区字节数 

/* 
*	目录表 
*/ 
//目录项 各属性偏移量 
#define CATALOG_START 		0x2600  //开始地址为第19扇区 512*19
#define DIR_Name 			0		//文件名
#define DIR_LName			0x8		//后缀名 
#define DIR_Attr 			0xB		//文件类型 
#define DIR_WrtTime 		0x16	//写入时间 
#define DIR_Date 			0x18 	//写入日期 
#define DIR_FstClus 		0x1A 	//开始簇号 
#define DIR_FileSize 		0x1C	//文件大小	
//目录项 各属性长度 
#define DIR_Name_Len 		8
#define DIR_LName_Len		3 
#define DIR_Attr_Len 		1 
#define DIR_WrtTime_Len 	2
#define DIR_Date_Len 		2
#define DIR_FstClus_Len 	2
#define DIR_FileSize_Len 	4

#define CATALOG_SIZE		32    //目录项占32字节
#define RootEntCnt			0xE0  //根目录文件数最大值


/*
*	FAT表 
*/
#define FAT_START			0x0200   //FAT表从第一扇区开始 
#define FAT2_START			0x1400	 //另一份一样的从第10扇区开始 10*512 


/*
*	数据区 
*/
#define DATA_START			0x4200	//数据区起始地址为33扇区 


/*
*	方法声明 
*/
void changetoFat(ushort cluu,ushort clu,uchar* s);
void findFilename(char* filename,int length,char* result);
void findFiletype(char* filename,int length,char* filetype);
int getcatafilesize(int filesize,uchar *p);
int getarraysize(char *str);
int dealcat(int secs,FILE* img,Catalog* head); 
#endif
