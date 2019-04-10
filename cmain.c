#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
//���̸�ʽ fat12 ����1.44m 
//�����ʽ��  writetoimage xxxfile xxx.img ��file�ļ��������� 

void changetoFat(short one,short two,unsigned char *s);
void findFilename(char* filename,int length,char* result);
void findFiletype(char* filename,int length,char* filetype);
int fatfilesize(int filesize,char *p);
int getarraysize(char *str);

int main(int argc, char *argv[]) {
	
	FILE *file;
	FILE *img; 
		
    int filesize; 		//��Ҫд����ļ���С 
    struct stat buf;
    int secs;           //�ļ�ռ�������� 
    int cu;
    int startcu=2;      //FAT���Ŀ�ʼ�غ� 
    
	if(argc==3){
		//��һ������argv[0]ָ��exe�ļ��ľ��Ե�ַ 
		//������������������������Ĳ��� 
		file=fopen(argv[2],"rb");      //ֻ�� 
		if(NULL==file){
			printf("����%s�ļ�������",argv[2]);
    		return 0;
		}
		img=fopen(argv[1],"rb+");      //�ɶ�д 
		if(NULL==img){
			printf("����%s�ļ�������",argv[1]);
    		return 0;
		}
	} else{
		printf("���������ʽ����");
	} 
	 
    stat(argv[2], &buf);           //�õ��ļ��ṹ������ 
    filesize = buf.st_size; 
    long modify_time=buf.st_mtime;    //����޸�ʱ�� (seconds passed from 01/01/00:00:00 1970 UTC)
    secs=filesize%512==0?(filesize/512):(filesize/512+1); 
    unsigned short fat[secs];    //FAT�� 
	
	
	/*д�������� */
	//��������Ϊ���̵ĵ�0���� 
	
	
	/*дFAT��*/ 
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
	unsigned char temp2[j];  //Ҫд���fat��            
	unsigned char *p=temp2;
	for(i=0;i<secs_temp/2;i++){
		changetoFat(fat[2*i],fat[2*i+1],p);
		p=p+3;
	}	
	
	fseek(img,512+3,0);           //ǰ�������ֽ�(����0,1��)�ǹ̶��� f0 ff ff 
	fwrite(temp2,j,1,img);        
	fseek(img,10*512+3,0);        //������һ����FAT����ռ9����������һ���ڵ�1�������ڶ����ڵ�10���� 
	fwrite(temp2,j,1,img);
	
	
	
	/*дĿ¼*/ 
	unsigned char filename[8];
	unsigned char filetype[3];
	unsigned char property=0;           //�ļ����ԣ����ﶼĬ��Ϊ����ͨ�ɶ�д�ļ� 
	unsigned char firstcu[2]={2,0};     //�״غţ���һ���ļ����Ӵغ�2��ʼ����˴洢 
	unsigned char filesize_in[4]={0,0,0,0};       //�ļ���С����λ�ֽڣ���˴洢 

	int length=getarraysize(argv[2]);
	findFilename(argv[2],length,filename);
	findFiletype(argv[2],length,filetype);
	fatfilesize(filesize,filesize_in);
	
	fseek(img,19*512,0); 
	fwrite(filename,8,1,img);  //�ļ��� 
	fseek(img,19*512+8,0);
	fwrite(filetype,3,1,img);  //�ļ����� 
	fseek(img,19*512+11,0);
	fputc(property,img);  //�ļ����� 
	fseek(img,19*512+26,0);
	fwrite(firstcu,2,1,img);  //�״غ� 
	fseek(img,19*512+28,0);
	fwrite(filesize_in,4,1,img);  //�ļ���С 
	//ʱ�����ڴ������� 

	

	/*д�ļ�����*/
	fseek(img,33*512,0);
	fseek(file,0,0);
	int buffsize=256;
	unsigned char *buffer[buffsize];    //��ȡ����������ȡʱʹ���޷��� 
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
			printf("���󣺷�����δ֪���󣡣���"); 
			fclose(img);
			fclose(file);
			return 1; 
		}	
	}

	fclose(img);
	fclose(file);
	return 0;   
}


//FAT�����ת�� 
void changetoFat(short one,short two,unsigned char *s){
	char *p=s;
	*s=one&0x0ff;
	*(s+1)=((two<<4)&0xf0)|((one>>8)&0x0f); 
	*(s+2)=(two&0xff0)>>4;
}

//��ʽ���ļ��� 
void findFilename(char* filename,int length,char* result){
	
	char *p=filename;

	int point=length-1;
	for(;point>=0;point--){
		if(p[point]=='.'){
			break;
		}
	}
	int i=0;
	if(point<0){       //�ļ���û��С����,Ĭ��ǰ8���ֽ�Ϊ�ļ��� 
		point=length; 
	}
	for(;i<point;i++){
		if(i==8){      //�ļ������ֻ��Ϊ8�ֽ� 
			break;
		}
		if(filename[i]>=97&&filename[i]<=122){   //�ļ���ֻ���Ǵ�д��ĸ��Сдʱ����ת�� 
			result[i]=filename[i]-32; 
		}else{
			result[i]=filename[i];
		} 
	}
	if(i<=7){
		for(;i<=7;i++){  //�ļ�������8���ֽڵĲ����ÿո�0x20������ 
			result[i]=0x20;
		}
	}
}

//��ʽ���ļ����� 
void findFiletype(char* filename,int length,char* filetype){
	
	char *p=filename;
	int point=length-1;
	for(;point>=0;point--){
		if(p[point]=='.'){
			break;
		}
	}
	if(point<0){       //�ļ���û��С����,Ĭ��Ϊ�� 
		*filetype=0x20; *(filetype+1)=0x20; *(filetype+2)=0x20;
		return ;
	} 
	int i=point+1;
	int k=0;
	for(;i<length;i++){
		if(k>3){
			return;
		}else{
			if(filename[i]>=97&&filename[i]<=122){   //�ļ�����Ҳֻ���Ǵ�д��ĸ��Сдʱ����ת�� 
				filetype[k]=filename[i]-32; 
			}else{
				filetype[k]=filename[i];
			} 
			k++;
		}
	}
	if(k<3){
		for(;k<3;i++,k++){  //�ļ����Ͳ���3���ֽڵĲ����ÿո�0x20������ 
			filetype[k]=0x20;
		}
	} 
	
}

//�ļ���Сת����4���ֽڱ�ʾ 
int fatfilesize(int filesize,char *p){
	int i=0;
	while(filesize){     //��˴洢��ԭ�����Դ�0��ʼ 
		p[i]=filesize%256;
		filesize/=256;
		i++;
	}
	if(i<5){
		return 1;       
	}else{
		return 0;       //�ļ����� 
	}
} 

//ͨ��ָ��õ��ַ������鳤�� 
int getarraysize(char *str){
	int i=0;
	while(str[i]!='\0'){
		i++;
	}
	return i;
}
 
