#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fat12.h"

uchar canuseclu[Max_CluSum+2]={0};          //加2因为0和1簇不能使用 

/*	FAT表项压缩
*  clu:要写入的簇号   s：要写入位置原来的数据  cluu:要写入的内容 
*/
void changetoFat(ushort cluu,ushort clu,uchar* s){
	uchar temp[2]={*s,*(s+1)};		//获得原来的数据 
	if(clu&0x01){
		*s=(temp[0]&0x0f)+((cluu&0x00f)<<4);
		*(s+1)=(cluu&0xff0)>>4;
	}else{
		*s=cluu&0x0ff;
		*(s+1)=(temp[1]&0xf0)+((cluu&0xf00)>>8); 
	}
	
}
/*	FAT表项解压缩 
* s：未解压的3个字节    fat:解压出来的两个簇号 
*/ 
void getFat(uchar* s,ushort* fat){
	*fat=((s[1]&0x0f)<<8)+s[0];
	*(fat+1)=(s[2]<<4)+(s[1]>>4);
}

/*格式化文件名 */ 
void findFilename(char* filename,int length,char* result){
	
	char *p=filename;

	int point=length-1;
	for(;point>=0;point--){
		if(p[point]=='.'){
			break;
		}
	}
	int i=0;
	if(point<0){       //文件名没有小数点,默认前8个字节为文件名 
		point=length; 
	}
	for(;i<point;i++){
		if(i==8){      //文件名最多只能为8字节 
			break;
		}
		if(filename[i]>=97&&filename[i]<=122){   //文件名只能是大写字母，小写时进行转换 
			result[i]=filename[i]-32; 
		}else{
			result[i]=filename[i];
		} 
	}
	if(i<=7){
		for(;i<=7;i++){  //文件名不够8个字节的部分用空格（0x20）补齐 
			result[i]=0x20;
		}
	}
}

/*格式化文件类型 */ 
void findFiletype(char* filename,int length,char* filetype){
	
	char *p=filename;
	int point=length-1;
	for(;point>=0;point--){
		if(p[point]=='.'){
			break;
		}
	}
	if(point<0){       //文件名没有小数点,默认为空 
		*filetype=0x20; *(filetype+1)=0x20; *(filetype+2)=0x20;
		return ;
	} 
	int i=point+1;
	int k=0;
	for(;i<length;i++){
		if(k>3){
			return;
		}else{
			if(filename[i]>=97&&filename[i]<=122){   //文件类型也只能是大写字母，小写时进行转换 
				filetype[k]=filename[i]-32; 
			}else{
				filetype[k]=filename[i];
			} 
			k++;
		}
	}
	if(k<3){
		for(;k<3;i++,k++){  //文件类型不够3个字节的部分用空格（0x20）补齐 
			filetype[k]=0x20;
		}
	} 
	
}

/*文件大小转换成4个字节表示 (或者进制转换作用)*/ 
int getcatafilesize(int filesize,uchar *p){
	int i=0;
	while(filesize){     //大端存储的原因所以从0开始,4个字节默认都为0（无论是软盘初始化还是文件删除） 
		p[i]=filesize%256;
		filesize/=256;
		i++;
	}
	if(i<5){
		return 1;       
	}else{
		return 0;       //文件过大 
	}
} 

/*通过指针得到字符串数组长度 */ 
int getarraysize(char *str){
	int i=0;
	while(str[i]!='\0'){
		i++;
	}
	return i;
}


/*判断软盘空间是否足够和找到目录项应该插入的位置*/ 
int changefromFat(ushort clu,int *s){
	if(clu&0x01){				//通过簇号得到簇号所在的字节（相关的字节有两个）起始位置 
		clu--;
		if(s!=NULL){
			*s=((clu*3)>>1)+1;
		}
	}else if(s!=NULL){
		*s=(clu*3)>>1;
	}
	return (clu*3)>>1;
} 
int dealcat(int secs,FILE* img,Catalog* head){
	
	int filesum=0;   //软盘已存在文件数 
	int cata_position=-1;  //记录待会目录项要插入的位置 
	uchar buffer[32];
	Catalog *node=head;

	int i=-1;
	fseek(img,CATALOG_START,0);   //跳转到目录项起始位置 
	while(true){
		i++;
		fread(buffer,sizeof(uchar),32,img);
		if(buffer[0]==0||i==RootEntCnt){  //目录项已经结束
			if(cata_position==-1){
				cata_position=i;
			} 
			node->next=NULL;
			break;
		}else if(buffer[0]!=0xe5){  //存在的文件
			filesum++;
			Catalog *c=(Catalog *)malloc(sizeof(Catalog));
			c->startclu=(buffer[DIR_FstClus+1]<<8)+buffer[DIR_FstClus];   //记录文件的起始簇号（大端存储）
			if(filesum==1){
				head->next=c;
				node=c;
			}else{
				node->next=c;
				node=c;
			}
		}else if(buffer[0]==0xe5){  //已经被删除的文件，可以在此处插入 
			cata_position=i;
		}
		fseek(img,(i+1)*CATALOG_SIZE,CATALOG_START);  //继续往下查询下一个目录项		
	} 
	
	//寻找可用的簇 
	uchar fat2[2]; //簇号
    ushort clu; 
    int clusize=0;
	uchar addr;    //簇号对应的物理偏移量 
	node=head->next; 
	while(node!=NULL){
		clu=node->startclu;
		clusize++; 
		canuseclu[clu]=1;      //记录已经被使用的簇
		int addr;
		uchar temp[3]; 
		ushort temp2[2];
		while(clu<0x0ff8){     //保存文件的簇链 
			addr=changefromFat(clu,NULL);
			fseek(img,FAT_START+addr,0);	
			fread(temp,sizeof(uchar),3,img);
			getFat(temp,temp2);
			if(clu&0x01){
				clu=temp2[1];
			}else{
				clu=temp2[0];
			}
			clusize++;
			canuseclu[clu]=1;
		}
		node=node->next;       //查找下一个存在文件 
	}
	
	if(secs>(Max_CluSum-clusize)){    //剩下的空间不足以保存这个文件 
		return -1;
	}	
	return cata_position;
} 

/*通过簇号写入数据 一个簇为512字节*/
void writebyClu(uchar *buffer,int buffersize,ushort clu,FILE *img){
	int offset=DATA_START+(clu-2)*BytsPerSec; 
	fseek(img,offset,0);
	fwrite(buffer,sizeof(uchar),buffersize,img);
	if(buffersize!=BytsPerSec){			//假如数据不够512字节，后面补0 
		int size=BytsPerSec-buffersize;
		for(;size>0;size--){
			fputc(0,img);
		}
	}
} 


