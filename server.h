#ifndef SEVER_H
#define SEVER_H


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

using namespace std;

#define DATA_SIZE 512

#define MAXSIZE 516
#define ERR_SIZE 100
#define TRUE 1
#define FALSE 0

#define RRQ 1
#define DATA 3
#define ACK 4
#define ERROR 5
#define uint16 unsigned short int



struct info_client
{
	bool is_last;
	FILE *fp, *fp_end;
	char file_name[20];
	uint16  block_number;
	struct sockaddr_in client_addr;	
	struct timespec record_time;
	int len;
};

struct tftp_message
{	
	char file_name[20]; 
	uint16 opcode, block_number, error_info;
			
};

int read_message(tftp_message* received_data,char* rec_buf);
int send_message_rrq(int sd, tftp_message *received_data, map<int, info_client *> &list);
int send_message_ack(int sd, tftp_message *received_data, map<int, info_client *> &list);
void info_client_input(char* file, struct sockaddr_in addr,info_client *new_client);
void make_tftp_message(int len, FILE **fp,char *&send_buf,uint16 opcode,uint16 block_number);
#endif	



		
	

