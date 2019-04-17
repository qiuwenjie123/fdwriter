#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "fat12.h"
//软盘格式 fat12 容量1.44m
//输入格式：  writetoimage xxxfile xxx.img 将file文件加入软盘 

int main(int argc, char *argv[]) {
	
	extern uchar canuseclu[Max_CluSum];
	
	FILE *file;
	FILE *img; 
	FILE *ipl; 
		
    int filesize; 			 //需要写入的文件大小 
    struct stat f_struct;    //文件描述符结构体 
    int secs;                //文件占用扇区数 
    int startcu=2;           //FAT区的开始簇号 
    int buffsize=512;        //读写缓冲区大小 
    unsigned char buffer[buffsize];    //读取缓冲区，读取时使用无符号
    int cataposition;
	int cata_addr;		 //目录项应该插入的位置 
    Catalog head;
    
	if(argc==3){
		//第一个参数argv[0]指向exe文件的绝对地址 
		//其他参数就是命令行中输入的参数 
		file=fopen(argv[2],"rb");                 //只读 
		if(NULL==file){
			printf("错误：%s文件不存在",argv[2]);
    		return 0;
		}
		img=fopen(argv[1],"rb+");                 //可读写 
		if(NULL==img){
			printf("错误：%s文件不存在",argv[1]);
    		return 0;
		}
	} else{
		printf("错误：输入格式错误");
	} 
	
	
	
	
	
	/*写引导扇区 */
	//引导扇区为磁盘的第0扇区 
	fseek(img,510,0);
	uchar sign[2];
	fread(sign,sizeof(uchar),sizeof(sign),img);
	ipl=fopen("ipl.bin","rb");
	if(sign[0]!=0xaa||sign[1]!=0x55){
		if(ipl==NULL){
			printf("错误：引导扇区文件ipl丢失！！！"); 
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
	
	
	
	
	 	 
    /*写FAT表*/
    stat(argv[2], &f_struct);           	//得到文件结构描述符 
    filesize = f_struct.st_size; 
    long modify_time=f_struct.st_mtime; 	//最近修改时间 (seconds passed from 01/01/00:00:00 1970 UTC)
    secs=filesize%512==0?(filesize/512):(filesize/512+1);
    ushort fat[secs];    			//FAT项
	ushort fat_num[secs]; 			//FAT项顺序 
    
	cataposition=dealcat(secs,img,&head); //目录项插入的位置 
	if(cataposition==-1){
		printf("警告：软盘剩余空间不足");
		fclose(img);
		fclose(file);
		return 0;   
	}
     
	int i=0,j=2; 
	while(i<secs){
		if(canuseclu[j]==0){
			fat_num[i++]=j++;		//寻找可用的簇 
		}else{
			j++;
		}
	}
	for(i=0;i<secs;i++){			//得到簇链 
		if(i==secs-1){
			fat[i]=0xfff;	
		}else{
			fat[i]=fat_num[i+1];
		}
	}
	//簇号逐个压缩并写入fat项 
	for(i=0;i<secs;i++){
		int fatstart;							//簇号在fat表的对应位置 
		int *fatstartp=&fatstart;						 
		uchar temp[2];
		changefromFat(fat_num[i],fatstartp);
		int position=FAT_START+fatstart;
		int other=FAT2_START+fatstart;         //有两个完全相同的fat表 
		fseek(img,position,0);
		fread(temp,sizeof(uchar),2,img);
		changetoFat(fat[i],fat_num[i],temp);	
		fseek(img,position,0);
		fwrite(temp,sizeof(uchar),2,img);
		fseek(img,other,0);
		fwrite(temp,sizeof(uchar),2,img);			
	}
	buffer[0]=0xf0; buffer[1]=0xff; buffer[2]=0xff;  //下面写入固定的字节 
	fseek(img,512,0); 
	fwrite(buffer,sizeof(uchar),3,img);
	fseek(img,10*512,0);
	fwrite(buffer,sizeof(uchar),3,img); 
	

	
	
	
	/*写目录*/ 
	uchar filename[8];
	uchar filetype[3];
	uchar property=0;           //文件属性，这里都默认为是普通可读写文件 
	uchar firstcu[2]={0,0};     //首簇号，大端存储 
	uchar filesize_in[4]={0,0,0,0};       //文件大小，单位字节，大端存储 

	getcatafilesize(fat_num[0],firstcu); 

	int length=getarraysize(argv[2]);
	findFilename(argv[2],length,filename);
	findFiletype(argv[2],length,filetype);
	getcatafilesize(filesize,filesize_in);
	
	cata_addr=CATALOG_START+CATALOG_SIZE*cataposition;
	
	fseek(img,cata_addr+DIR_Name,0); 
	fwrite(filename,sizeof(uchar),DIR_Name_Len,img);    	 //文件名
	 
	fseek(img,cata_addr+DIR_LName,0);
	fwrite(filetype,sizeof(uchar),DIR_LName_Len,img);   	 //文件类型(后缀) 
	
	fseek(img,cata_addr+DIR_Attr,0);
	fputc(property,img);  									 //文件属性 
	
	fseek(img,cata_addr+DIR_FstClus,0);
	fwrite(firstcu,sizeof(uchar),DIR_FstClus_Len,img);  	 //首簇号
	 
	fseek(img,cata_addr+DIR_FileSize,0);
	fwrite(filesize_in,sizeof(uchar),DIR_FileSize_Len,img);  //文件大小 
	//时间日期处理。。。 

	

	/*写文件数据*/
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
	
	
	printf("文件加入成功\n"); 
	fclose(img);
	fclose(file);
	return 0;   
}



