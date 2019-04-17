#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "fat12.h"
//���̸�ʽ fat12 ����1.44m
//�����ʽ��  writetoimage xxxfile xxx.img ��file�ļ��������� 

int main(int argc, char *argv[]) {
	
	extern uchar canuseclu[Max_CluSum];
	
	FILE *file;
	FILE *img; 
	FILE *ipl; 
		
    int filesize; 			 //��Ҫд����ļ���С 
    struct stat f_struct;    //�ļ��������ṹ�� 
    int secs;                //�ļ�ռ�������� 
    int startcu=2;           //FAT���Ŀ�ʼ�غ� 
    int buffsize=512;        //��д��������С 
    unsigned char buffer[buffsize];    //��ȡ����������ȡʱʹ���޷���
    int cataposition;
	int cata_addr;		 //Ŀ¼��Ӧ�ò����λ�� 
    Catalog head;
    
	if(argc==3){
		//��һ������argv[0]ָ��exe�ļ��ľ��Ե�ַ 
		//������������������������Ĳ��� 
		file=fopen(argv[2],"rb");                 //ֻ�� 
		if(NULL==file){
			printf("����%s�ļ�������",argv[2]);
    		return 0;
		}
		img=fopen(argv[1],"rb+");                 //�ɶ�д 
		if(NULL==img){
			printf("����%s�ļ�������",argv[1]);
    		return 0;
		}
	} else{
		printf("���������ʽ����");
	} 
	
	
	
	
	
	/*д�������� */
	//��������Ϊ���̵ĵ�0���� 
	fseek(img,510,0);
	uchar sign[2];
	fread(sign,sizeof(uchar),sizeof(sign),img);
	ipl=fopen("ipl.bin","rb");
	if(sign[0]!=0xaa||sign[1]!=0x55){
		if(ipl==NULL){
			printf("�������������ļ�ipl��ʧ������"); 
			fclose(img);
			fclose(file);
			return 1; 
		}
		ipl=fopen("ipl.bin","rb");
		fseek(img,0,0);
		fread(buffer,sizeof(unsigned char),sizeof(buffer),ipl);
		fwrite(buffer,sizeof(unsigned char),sizeof(buffer),img);
		fclose(ipl);
	} 
	
	
	
	
	 	 
    /*дFAT��*/
    stat(argv[2], &f_struct);           	//�õ��ļ��ṹ������ 
    filesize = f_struct.st_size; 
    long modify_time=f_struct.st_mtime; 	//����޸�ʱ�� (seconds passed from 01/01/00:00:00 1970 UTC)
    secs=filesize%512==0?(filesize/512):(filesize/512+1);
    ushort fat[secs];    			//FAT��
	ushort fat_num[secs]; 			//FAT��˳�� 
    
	cataposition=dealcat(secs,img,&head); //Ŀ¼������λ�� 
	if(cataposition==-1){
		printf("���棺����ʣ��ռ䲻��");
		fclose(img);
		fclose(file);
		return 0;   
	}
     
	int i=0,j=2; 
	while(i<secs){
		if(canuseclu[j]==0){
			fat_num[i++]=j++;		//Ѱ�ҿ��õĴ� 
		}else{
			j++;
		}
	}
	for(i=0;i<secs;i++){			//�õ����� 
		if(i==secs-1){
			fat[i]=0xfff;	
		}else{
			fat[i]=fat_num[i+1];
		}
	}
	//�غ����ѹ����д��fat�� 
	for(i=0;i<secs;i++){
		int fatstart;							//�غ���fat��Ķ�Ӧλ�� 
		int *fatstartp=&fatstart;						 
		uchar temp[2];
		changefromFat(fat_num[i],fatstartp);
		int position=FAT_START+fatstart;
		int other=FAT2_START+fatstart;         //��������ȫ��ͬ��fat�� 
		fseek(img,position,0);
		fread(temp,sizeof(uchar),2,img);
		changetoFat(fat[i],fat_num[i],temp);	
		fseek(img,position,0);
		fwrite(temp,sizeof(uchar),2,img);
		fseek(img,other,0);
		fwrite(temp,sizeof(uchar),2,img);			
	}
	buffer[0]=0xf0; buffer[1]=0xff; buffer[2]=0xff;  //����д��̶����ֽ� 
	fseek(img,512,0); 
	fwrite(buffer,sizeof(uchar),3,img);
	fseek(img,10*512,0);
	fwrite(buffer,sizeof(uchar),3,img); 
	

	
	
	
	/*дĿ¼*/ 
	uchar filename[8];
	uchar filetype[3];
	uchar property=0;           //�ļ����ԣ����ﶼĬ��Ϊ����ͨ�ɶ�д�ļ� 
	uchar firstcu[2]={0,0};     //�״غţ���˴洢 
	uchar filesize_in[4]={0,0,0,0};       //�ļ���С����λ�ֽڣ���˴洢 

	getcatafilesize(fat_num[0],firstcu); 

	int length=getarraysize(argv[2]);
	findFilename(argv[2],length,filename);
	findFiletype(argv[2],length,filetype);
	getcatafilesize(filesize,filesize_in);
	
	cata_addr=CATALOG_START+CATALOG_SIZE*cataposition;
	
	fseek(img,cata_addr+DIR_Name,0); 
	fwrite(filename,sizeof(uchar),DIR_Name_Len,img);    	 //�ļ���
	 
	fseek(img,cata_addr+DIR_LName,0);
	fwrite(filetype,sizeof(uchar),DIR_LName_Len,img);   	 //�ļ�����(��׺) 
	
	fseek(img,cata_addr+DIR_Attr,0);
	fputc(property,img);  									 //�ļ����� 
	
	fseek(img,cata_addr+DIR_FstClus,0);
	fwrite(firstcu,sizeof(uchar),DIR_FstClus_Len,img);  	 //�״غ�
	 
	fseek(img,cata_addr+DIR_FileSize,0);
	fwrite(filesize_in,sizeof(uchar),DIR_FileSize_Len,img);  //�ļ���С 
	//ʱ�����ڴ������� 

	

	/*д�ļ�����*/
	fseek(file,0,0);
	int count;
	for(i=0;i<secs;i++){
		count=fread(buffer,sizeof(unsigned char),buffsize,file);
		if(count!=buffsize){
			writebyClu(buffer,count,fat_num[i],img);	
		}else{
			writebyClu(buffer,buffsize,fat_num[i],img);
		}		
	}
	
	
	printf("�ļ�����ɹ�\n"); 
	fclose(img);
	fclose(file);
	return 0;   
}



