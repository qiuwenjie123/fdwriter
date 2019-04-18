#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fat12.h"

uchar canuseclu[Max_CluSum+2]={0};          //��2��Ϊ0��1�ز���ʹ�� 

/*	FAT����ѹ��
*  clu:Ҫд��Ĵغ�   s��Ҫд��λ��ԭ��������  cluu:Ҫд������� 
*/
void changetoFat(ushort cluu,ushort clu,uchar* s){
	uchar temp[2]={*s,*(s+1)};		//���ԭ�������� 
	if(clu&0x01){
		*s=(temp[0]&0x0f)+((cluu&0x00f)<<4);
		*(s+1)=(cluu&0xff0)>>4;
	}else{
		*s=cluu&0x0ff;
		*(s+1)=(temp[1]&0xf0)+((cluu&0xf00)>>8); 
	}
	
}
/*	FAT�����ѹ�� 
* s��δ��ѹ��3���ֽ�    fat:��ѹ�����������غ� 
*/ 
void getFat(uchar* s,ushort* fat){
	*fat=((s[1]&0x0f)<<8)+s[0];
	*(fat+1)=(s[2]<<4)+(s[1]>>4);
}

/*��ʽ���ļ��� */ 
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

/*��ʽ���ļ����� */ 
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

/*�ļ���Сת����4���ֽڱ�ʾ (���߽���ת������)*/ 
int getcatafilesize(int filesize,uchar *p){
	int i=0;
	while(filesize){     //��˴洢��ԭ�����Դ�0��ʼ,4���ֽ�Ĭ�϶�Ϊ0�����������̳�ʼ�������ļ�ɾ���� 
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

/*ͨ��ָ��õ��ַ������鳤�� */ 
int getarraysize(char *str){
	int i=0;
	while(str[i]!='\0'){
		i++;
	}
	return i;
}


/*�ж����̿ռ��Ƿ��㹻���ҵ�Ŀ¼��Ӧ�ò����λ��*/ 
int changefromFat(ushort clu,int *s){
	if(clu&0x01){				//ͨ���غŵõ��غ����ڵ��ֽڣ���ص��ֽ�����������ʼλ�� 
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
	
	int filesum=0;   //�����Ѵ����ļ��� 
	int cata_position=-1;  //��¼����Ŀ¼��Ҫ�����λ�� 
	uchar buffer[32];
	Catalog *node=head;

	int i=-1;
	fseek(img,CATALOG_START,0);   //��ת��Ŀ¼����ʼλ�� 
	while(true){
		i++;
		fread(buffer,sizeof(uchar),32,img);
		if(buffer[0]==0||i==RootEntCnt){  //Ŀ¼���Ѿ�����
			if(cata_position==-1){
				cata_position=i;
			} 
			node->next=NULL;
			break;
		}else if(buffer[0]!=0xe5){  //���ڵ��ļ�
			filesum++;
			Catalog *c=(Catalog *)malloc(sizeof(Catalog));
			c->startclu=(buffer[DIR_FstClus+1]<<8)+buffer[DIR_FstClus];   //��¼�ļ�����ʼ�غţ���˴洢��
			if(filesum==1){
				head->next=c;
				node=c;
			}else{
				node->next=c;
				node=c;
			}
		}else if(buffer[0]==0xe5){  //�Ѿ���ɾ�����ļ��������ڴ˴����� 
			cata_position=i;
		}
		fseek(img,(i+1)*CATALOG_SIZE,CATALOG_START);  //�������²�ѯ��һ��Ŀ¼��		
	} 
	
	//Ѱ�ҿ��õĴ� 
	uchar fat2[2]; //�غ�
    ushort clu; 
    int clusize=0;
	uchar addr;    //�غŶ�Ӧ������ƫ���� 
	node=head->next; 
	while(node!=NULL){
		clu=node->startclu;
		clusize++; 
		canuseclu[clu]=1;      //��¼�Ѿ���ʹ�õĴ�
		int addr;
		uchar temp[3]; 
		ushort temp2[2];
		while(clu<0x0ff8){     //�����ļ��Ĵ��� 
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
		node=node->next;       //������һ�������ļ� 
	}
	
	if(secs>(Max_CluSum-clusize)){    //ʣ�µĿռ䲻���Ա�������ļ� 
		return -1;
	}	
	return cata_position;
} 

/*ͨ���غ�д������ һ����Ϊ512�ֽ�*/
void writebyClu(uchar *buffer,int buffersize,ushort clu,FILE *img){
	int offset=DATA_START+(clu-2)*BytsPerSec; 
	fseek(img,offset,0);
	fwrite(buffer,sizeof(uchar),buffersize,img);
	if(buffersize!=BytsPerSec){			//�������ݲ���512�ֽڣ����油0 
		int size=BytsPerSec-buffersize;
		for(;size>0;size--){
			fputc(0,img);
		}
	}
} 


