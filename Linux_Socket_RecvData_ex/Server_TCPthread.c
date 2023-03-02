
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>

const int Port=7525;
const int buffer_Size=1300;
static int recvcount=0;
static int semaphore=0;     //傳送存取 號誌
static char* buf[1300];



static char* Savedata1[1600];
static char* Savedata2[1600];
static char* Savedata3[1600];
static char* Savedata4[1600];
void split(char *src,const char *separator,char **dest,int *num) 
{
	/*
		src 源字串的首地址(buf的地址) 
		separator 指定的分割字元
		dest 接收子字串的陣列
		num 分割後子字串的個數
	*/
     char *pNext;
     int count = 0;
     if (src == NULL || strlen(src) == 0) //如果傳入的地址為空或長度為0，直接終止 
        return;
     if (separator == NULL || strlen(separator) == 0) //如未指定分割的字串，直接終止 
        return;
     pNext = (char *)strtok(src,separator); //必須使用(char *)進行強制型別轉換(雖然不寫有的編譯器中不會出現指標錯誤)
     while(pNext != NULL) 
     {
          *dest++ = pNext;
          ++count;
         pNext = (char *)strtok(NULL,separator);  //必須使用(char *)進行強制型別轉換
     }  
    *num = count;
} 
char *string_concat(char *str1, char *str2) 
{  
   // 計算所需的陣列長度  
   int length=strlen(str1)+strlen(str2)+1;     
   // 產生新的陣列空間  
   char *result = (char*)malloc(sizeof(char) * length);      
   // 複製第一個字串至新的陣列空間  
   strcpy(result, str1);  
   // 串接第二個字串至新的陣列空間  
   strcat(result, str2);  
     
   return result;  
} 
void* SocketRecive(void *fd)
{     
  int sockfd = *(int *)fd;
  while(1)
  {
    while(semaphore!=0)
    {
      //waiting write
    }    
    memset(buf,0,buffer_Size);
    int socketerror = recv(sockfd,buf,buffer_Size,0);
    if(socketerror<0)
    printf("recv error\n");    
    //printf("%s\n",realdatabuf[0]);
    if(buf!=NULL)
    recvcount+=1;
    if(recvcount %  1000 ==0)
    printf("已經接收%d千筆資料\n",recvcount/1000);
    semaphore=1;
  }
      
}
        
void* Writefile(void *argu)
{
  FILE *pFile;
  while(1)
  {     
        time_t tt = time(NULL);
        struct tm *stm = localtime(&tt);
        char Txt[]=".txt";
        while(semaphore!=1)
        {
          //waiting recv
        }
        //抓取時間資料 並且加上txt
        char tmp[32];
        sprintf(tmp,"ID01//%04d-%02d-%2d-%2d-%2d-%02d.txt", 1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour,stm->tm_min, stm->tm_sec);
        //printf("%s\n",tmp );

        //FLIE SYSTEM        
        //char *filename=string_concat(tmp,Txt);                                 
        pFile = fopen(tmp,"ab+");
        if(pFile==NULL)
        {
          printf("openfail create the file.txt");       
        }
        else
        { //printf("Write\n");

          fwrite(buf,sizeof(buf),sizeof(buf),pFile);
        }
        fclose(pFile);  
        semaphore=0;  

  }


}
int main(int argc , char *argv[])
{   
    //Share memory 參數
	  void *shmaddr = NULL;
    int shmid;
    long page_size = sysconf(_SC_PAGESIZE);


    //socket的建立參數//  
    int sockfd,ret ,newSocket;
    struct  sockaddr_in serverAddr; 
    struct  sockaddr_in newAddr;    
    socklen_t addr_size;    
    char buffer[1320];						//socket接收進來的buffer
    //////////////////

    char *realdatabuf[1320]={};
    //File System參數///  
    

    ///////////////////

    //多執行序參數
    pthread_t Rcv_t1,Writefile_t2;

    sockfd =socket(AF_INET,SOCK_STREAM,0);  //Socket 建立
    if(sockfd<0)
    {
        printf("Error in connection \n.");
        exit(1);
    }
    printf("(sever socket is created)\n" );
    memset(&serverAddr,'\0',sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(Port);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.11.161");
    ret =bind(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
  	if(ret<0)
  	{
  		printf("Binding錯誤 \n.");
        exit(1);
  	}
  	printf("(bind to Port%d)\n",Port);
  	if(listen(sockfd,10)==0)
  	{
  		printf("(正在監聽中)\n" );
  	}
  	else
  	{
  		printf("監聽錯誤\n");
  	}
    newSocket = accept(sockfd,(struct sockaddr*)&newAddr,&addr_size);    
    if(newSocket < 0)
    {
      exit(1);
    }     
    printf("connection accepted form %s:%d\n",inet_ntoa(newAddr.sin_addr),ntohs(newAddr.sin_port));
    
    //多執行緒建立執行
    pthread_create(&Rcv_t1,NULL,SocketRecive,&newSocket);  
    pthread_create(&Writefile_t2,NULL,Writefile,NULL);  
  	
      while(1)
    	{
    	}
    	close(newSocket);
    
    return 0;
}
