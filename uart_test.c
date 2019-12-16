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
#include <sys/types.h>
#include <asm/types.h>
//该头文件需要放在netlink.h前面防止编译出现__kernel_sa_family未定义
#include <sys/socket.h>  
#include <linux/netlink.h>
 
void print_hex(uint8_t *str, uint8_t len)
{
    while (len > 0)
    {
        printf(" %x,", *str);
        str++;
        len--;
    }
    printf("\r\n");
}

void MonitorNetlinkUevent()
{
    int sockfd;
    struct sockaddr_nl sa;
    int len;
    char buf[4096];
    struct iovec iov;
    struct msghdr msg;
    int i;

    memset(&sa,0,sizeof(sa));
    sa.nl_family=AF_NETLINK;
    sa.nl_groups=NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;//getpid(); both is ok
    memset(&msg,0,sizeof(msg));
    iov.iov_base=(void *)buf;
    iov.iov_len=sizeof(buf);
    msg.msg_name=(void *)&sa;
    msg.msg_namelen=sizeof(sa);
    msg.msg_iov=&iov;
    msg.msg_iovlen=1;

    sockfd=socket(AF_NETLINK,SOCK_RAW,NETLINK_KOBJECT_UEVENT);
    if(sockfd==-1)
        printf("socket creating failed:%s\n",strerror(errno));
    if(bind(sockfd,(struct sockaddr *)&sa,sizeof(sa))==-1)
        printf("bind error:%s\n",strerror(errno));

    len=recvmsg(sockfd,&msg,0);
    if(len<0)
        printf("receive error\n");
    else if(len<32||len>sizeof(buf))
        printf("invalid message");
    for(i=0;i<len;i++)
        if(*(buf+i)=='\0')
            buf[i]='\n';
    printf("received %d bytes\n%s\n",len,buf);
}


int wait_usb_plugin(){
	int fd = -1;

    MonitorNetlinkUevent();

	while ( (fd = UART0_Open(fd,"/dev/ttyUSB0") ) < 0){
		sleep(1);
	}

	printf("usb device found \r\n");

	UART0_Init(fd,115200,0,8,1,'N'); 

	printf("matching device's puf ... \r\n");
	return fd;
	
}

uint8_t *get_puf_response(int fd){

	uint8_t  buf[256];
	uint8_t *resp = (uint8_t *)calloc(5,sizeof(uint8_t));
    uint8_t puf[] = {0x0a,0xaa,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x88};
    uint8_t err_times = 0;

	while (1)
	{
		
		UART0_Send(fd,(char *)puf,11);
		printf("send challenge cmd :");
		print_hex(puf,11);
		if ( UART0_Recv(fd, (char *)buf, 10) != buf[0]+1 ){
            if(err_times++ > 3){
                return resp;
            }
			continue;
		}
		printf("receive puf response : ");
		print_hex(buf,buf[0]+1);
		break;
	}
	
	memcpy(resp,buf+2,4);
	return resp;
	
	
}

uint8_t calc_hamming_dist_between_two_bytes(uint8_t byte_a,uint8_t byte_b){
	uint8_t hamming_dist_one_byte = 0;
	for(uint8_t i = 0;i < 8;i++){
		if ( ((byte_a>>i)&0x1) ^ ((byte_b>>i)&0x1) ){
			hamming_dist_one_byte++;
		}
	}
	return hamming_dist_one_byte;
}

uint8_t calc_hamming_dist(const uint8_t *std_resp,const uint8_t *unsure_resp,uint8_t nbytes){
	
	uint8_t hamming_dist = 0;
	for (uint8_t i = 0; i < nbytes; i++)
	{
		hamming_dist += calc_hamming_dist_between_two_bytes(std_resp[i],unsure_resp[i]);
	}

    printf("hamming_dist : %d \r\n",hamming_dist);
	return hamming_dist;
	
}

int match_response(uint8_t *resp){
	const uint8_t std_resp[] = {0xc, 0x8d, 0xa, 0x4c};
	if( calc_hamming_dist(std_resp,resp,4) <= 3 ){
		return 1;
	}
	else
	{
		return 0;
	}
	
}

void report_result(uint8_t result){
	if(result == 1){
		printf("usb device was trusted \r\n");
	}
	else{
		printf("usb device was not trusted \r\n");
	}
}


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{

    int    fd, c=0, res;
 
    uint8_t  buf[256];
    uint8_t puf[] = {0x0a,0xaa,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x88};

	int result;
	uint8_t *resp = NULL;

	while (1)
	{
		
		fd = wait_usb_plugin();

		resp = get_puf_response(fd);

		result = match_response(resp);

		report_result(result);

		close(fd);

        MonitorNetlinkUevent();
	}
 
    return 0;
}

