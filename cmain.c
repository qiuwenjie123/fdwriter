#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
//软盘格式 fat12 容量1.44m 
//输入格式：  writetoimage xxxfile xxx.img 将file文件加入软盘 

void changetoFat(short one,short two,unsigned char *s);
void findFilename(char* filename,int length,char* result);
void findFiletype(char* filename,int length,char* filetype);
int fatfilesize(int filesize,char *p);
int getarraysize(char *str);

int main(int argc, char *argv[]) {
	
	FILE *file;
	FILE *img; 
		
    int filesize; 		//需要写入的文件大小 
    struct stat buf;
    int secs;           //文件占用扇区数 
    int cu;
    int startcu=2;      //FAT区的开始簇号 
    
	if(argc==3){
		//第一个参数argv[0]指向exe文件的绝对地址 
		//其他参数就是命令行中输入的参数 
		file=fopen(argv[2],"rb");      //只读 
		if(NULL==file){
			printf("错误：%s文件不存在",argv[2]);
    		return 0;
		}
		img=fopen(argv[1],"rb+");      //可读写 
		if(NULL==img){
			printf("错误：%s文件不存在",argv[1]);
    		return 0;
		}
	} else{
		printf("错误：输入格式错误");
	} 
	 
    stat(argv[2], &buf);           //得到文件结构描述符 
    filesize = buf.st_size; 
    long modify_time=buf.st_mtime;    //最近修改时间 (seconds passed from 01/01/00:00:00 1970 UTC)
    secs=filesize%512==0?(filesize/512):(filesize/512+1); 
    unsigned short fat[secs];    //FAT项 
	
	
	/*写引导扇区 */
	//引导扇区为磁盘的第0扇区 
	
	
	/*写FAT表*/ 
	int i; 
	for(i=0;i<secs;i++){
	    if(i==secs-1){
	    	fat[i]=0xfff;
		}else{
			fat[i]=i+startcu+1;
		}		
	}
	int secs_temp; 
	secs_temp=secs;
	if(secs&0x01==1){
		secs_temp=secs+1;	
	}
	int j=secs_temp/2*3;
	unsigned char temp2[j];  //要写入的fat表            
	unsigned char *p=temp2;
	for(i=0;i<secs_temp/2;i++){
		changetoFat(fat[2*i],fat[2*i+1],p);
		p=p+3;
	}	
	
	fseek(img,512+3,0);           //前面三个字节(即第0,1簇)是固定的 f0 ff ff 
	fwrite(temp2,j,1,img);        
	fseek(img,10*512+3,0);        //有两个一样的FAT表，都占9个扇区，第一个在第1扇区，第二个在第10扇区 
	fwrite(temp2,j,1,img);
	
	
	
	/*写目录*/ 
	unsigned char filename[8];
	unsigned char filetype[3];
	unsigned char property=0;           //文件属性，这里都默认为是普通可读写文件 
	unsigned char firstcu[2]={2,0};     //首簇号，第一个文件都从簇号2开始，大端存储 
	unsigned char filesize_in[4]={0,0,0,0};       //文件大小，单位字节，大端存储 

	int length=getarraysize(argv[2]);
	findFilename(argv[2],length,filename);
	findFiletype(argv[2],length,filetype);
	fatfilesize(filesize,filesize_in);
	
	fseek(img,19*512,0); 
	fwrite(filename,8,1,img);  //文件名 
	fseek(img,19*512+8,0);
	fwrite(filetype,3,1,img);  //文件类型 
	fseek(img,19*512+11,0);
	fputc(property,img);  //文件属性 
	fseek(img,19*512+26,0);
	fwrite(firstcu,2,1,img);  //首簇号 
	fseek(img,19*512+28,0);
	fwrite(filesize_in,4,1,img);  //文件大小 
	//时间日期处理。。。 

	

	/*写文件数据*/
	fseek(img,33*512,0);
	fseek(file,0,0);
	int buffsize=256;
	unsigned char *buffer[buffsize];    //读取缓冲区，读取时使用无符号 
	int count;
	
	while(1){
		count=fread(buffer,sizeof(unsigned char),buffsize,file);
		if(count!=buffsize){
			fwrite(buffer,sizeof(unsigned char),count,img);
		}else{
			fwrite(buffer,sizeof(unsigned char),buffsize,img);
		}
		
		if(feof(file)){
			break;
		}else if(ferror(file)){
			printf("错误：发生了未知错误！！！"); 
			fclose(img);
			fclose(file);
			return 1; 
		}	
	}

	fclose(img);
	fclose(file);
	return 0;   
}


//FAT表项的转化 
void changetoFat(short one,short two,unsigned char *s){
	char *p=s;
	*s=one&0x0ff;
	*(s+1)=((two<<4)&0xf0)|((one>>8)&0x0f); 
	*(s+2)=(two&0xff0)>>4;
}

//格式化文件名 
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

//格式化文件类型 
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

//文件大小转换成4个字节表示 
int fatfilesize(int filesize,char *p){
	int i=0;
	while(filesize){     //大端存储的原因所以从0开始 
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

//通过指针得到字符串数组长度 
int getarraysize(char *str){
	int i=0;
	while(str[i]!='\0'){
		i++;
	}
	return i;
}
 
