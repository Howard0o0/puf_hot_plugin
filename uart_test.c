/*********************************************************************************
 *      Copyright:  (C) 2018 Yujie
 *                  All rights reserved.
 *
 *       Filename:  usart_test.c
 *    Description:  串口测试
 *                 
 *        Version:  1.0.0(08/27/2018)
 *         Author:  yanhuan <yanhuanmini@foxmail.com>
 *      ChangeLog:  1, Release initial version on "08/23/2018 17:28:51 PM"
 *                 
 ********************************************************************************/
 
#include "usart.h"
#include<stdio.h>      /*标准输入输出定义*/    
#include<stdlib.h>     /*标准函数库定义*/    
#include<unistd.h>     /*Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*文件控制定义*/    
#include<termios.h>    /*PPSIX 终端控制定义*/    
#include<errno.h>      /*错误号定义*/    
#include<string.h>
#include <stdint.h>
 
void print_hex(char *str, uint8_t len)
{
    while (len > 0)
    {
        printf(" %x,", *str);
        str++;
        len--;
    }
    printf("\r\n");
}

int main(int argc, char **argv)    
{
	int fd = -1;           //文件描述符，先定义一个与程序无关的值，防止fd为任意值导致程序出bug    
    int err;               //返回调用函数的状态    
    int len;                            
    int i;    
    char rcv_buf[256];             
    char send_buf[256];
    char challenge[] = {0x0a,0xaa,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x88};

    

    fd = UART0_Open(fd,"/dev/ttyUSB0");
    UART0_Init(fd,115200,0,8,1,'N');   
    while (1)
    {
        
        len = UART0_Send(fd,challenge,11);
        printf("send [%d] : ",len);
        print_hex(challenge,len);

        len = UART0_Recv(fd, rcv_buf,sizeof(rcv_buf));   
        printf("rcv [%d] : ",len);
        print_hex(rcv_buf,len);

        sleep(1); 
    }


    if(argc != 3)    
    {    
        printf("Usage: %s /dev/ttySn 1(send data)/1 (receive data) \n",argv[0]);
        printf("open failure : %s\n", strerror(errno));
    
        return FALSE;    
    }    
     fd = UART0_Open(fd,argv[1]); //打开串口，返回文件描述符   
     // fd=open("dev/ttyS1", O_RDWR);
    //printf("fd= \n",fd);
     do  
    {    
        err = UART0_Init(fd,115200,0,8,1,'N');    
        printf("Set Port Exactly!\n"); 
        sleep(1);   
    }while(FALSE == err || FALSE == fd);    
       
    if(0 == strcmp(argv[2],"0"))    //开发板向pc发送数据的模式
    {   
        fgets(send_buf,256,stdin);   //输入内容，最大不超过40字节，fgets能吸收回车符，这样pc收到的数据就能自动换行     
        for(i = 0;i < 10;i++)    
        {    
            len = UART0_Send(fd,send_buf,40);    
            if(len > 0)    
                printf(" %d time send %d data successful\n",i,len);    
            else    
                printf("send data failed!\n");    
                              
            sleep(1);    
        }    
        UART0_Close(fd);                 
    }    
    else                            //开发板收到pc发送的数据的模式                 
    {                                          
        while (1) //循环读取数据    
        {   
            len = UART0_Recv(fd, rcv_buf,sizeof(rcv_buf));    
            if(len > 0)    
            {    
                rcv_buf[len] = '\0';    
                printf("receive data is %s\n",rcv_buf);    
            }    
            else    
            {    
                printf("cannot receive data\n");    
            }    
            sleep(1);    
        }                
        UART0_Close(fd);     
    }    
}    