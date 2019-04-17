#ifndef _FAT12_H_
#define _FAT12_H_

typedef unsigned short ushort;
typedef unsigned char uchar;

//�Ѿ����ڵ�Ŀ¼��(����ṹ) 
typedef struct _catbuffer { 
	int startclu;   //��ʼ�غ�
	struct _catbuffer *next; //ָ����һ�� 
}Catalog;


//1.44m��󱣴��ܱ�����ļ��Ĵ����� 
#define Max_CluSum 			0xB1F
#define BytsPerSec			0x200	//ÿ�����ֽ��� 

/* 
*	Ŀ¼�� 
*/ 
//Ŀ¼�� ������ƫ���� 
#define CATALOG_START 		0x2600  //��ʼ��ַΪ��19���� 512*19
#define DIR_Name 			0		//�ļ���
#define DIR_LName			0x8		//��׺�� 
#define DIR_Attr 			0xB		//�ļ����� 
#define DIR_WrtTime 		0x16	//д��ʱ�� 
#define DIR_Date 			0x18 	//д������ 
#define DIR_FstClus 		0x1A 	//��ʼ�غ� 
#define DIR_FileSize 		0x1C	//�ļ���С	
//Ŀ¼�� �����Գ��� 
#define DIR_Name_Len 		8
#define DIR_LName_Len		3 
#define DIR_Attr_Len 		1 
#define DIR_WrtTime_Len 	2
#define DIR_Date_Len 		2
#define DIR_FstClus_Len 	2
#define DIR_FileSize_Len 	4

#define CATALOG_SIZE		32    //Ŀ¼��ռ32�ֽ�
#define RootEntCnt			0xE0  //��Ŀ¼�ļ������ֵ


/*
*	FAT�� 
*/
#define FAT_START			0x0200   //FAT��ӵ�һ������ʼ 
#define FAT2_START			0x1400	 //��һ��һ���Ĵӵ�10������ʼ 10*512 


/*
*	������ 
*/
#define DATA_START			0x4200	//��������ʼ��ַΪ33���� 


/*
*	�������� 
*/
void changetoFat(ushort cluu,ushort clu,uchar* s);
void findFilename(char* filename,int length,char* result);
void findFiletype(char* filename,int length,char* filetype);
int getcatafilesize(int filesize,uchar *p);
int getarraysize(char *str);
int dealcat(int secs,FILE* img,Catalog* head); 
#endif
