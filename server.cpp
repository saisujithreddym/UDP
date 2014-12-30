#include "server.h"	



int main(int argc,char *argv[])
{
	int sockfd ,new_sd,addr_len,i,k,rv,fdmax;
	char rec_buf[MAXSIZE];
    short int temp_port;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in server_addr,their_addr;
	fd_set read_fds, master_set;  
    map<int, info_client* > client_list;
	//struct timespec current_time;//current time ns
    //struct timeval time_out;//timeout time us
	int HaveSent[100];
 		
	
	if(argc !=3)
	{
		printf("usage: IP ./server server_port\n");
		return -1;
	}

	// get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;  // use my IP

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "sever: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);

	FD_ZERO(&read_fds);  // clear the master and temp sets
	FD_ZERO(&master_set);
	FD_SET(sockfd, &master_set); // add the liste sockfd to the master set.
	fdmax = sockfd;
	//time_out.tv_sec = 5;
    //time_out.tv_usec = 0;
	for (i=0;i<100;i++)
	HaveSent[i]=FALSE;
    
    // main loop
	
	for(;;)
	{       
		read_fds = master_set;		

		if( select(fdmax+1, &read_fds, NULL, NULL, NULL)==-1  )
		{
			perror("select");
			return -1;
		}
		
		for(i=0; i<= fdmax; i++)
		{
			
		
			if(FD_ISSET(i, &read_fds))   // we get a client 
			{
				
				tftp_message *received_data = new tftp_message;
								
				if(i == sockfd) 		//A new client comes		
				{	
		                 
				addr_len=sizeof(their_addr);
				if(recvfrom(sockfd,rec_buf,MAXSIZE,0,(struct sockaddr *)&their_addr,(socklen_t*)&addr_len)==-1)//put received data into rec_buf
               	   	{
        			 perror("recvfrom");
					 return -1;
					}
                			
					read_message(received_data,rec_buf);//unpack rec_buf and record into read_message 

					
					if(received_data->opcode == RRQ)//we receive RRQ as the opcode
					{
						new_sd=socket(AF_INET,SOCK_DGRAM,0);//Create a new socket for existed client

						if(new_sd<0)
						{
							perror("accept");
							return -1;
						}
				
						FD_SET(new_sd, &master_set);
				        if(new_sd >= fdmax)
							fdmax = new_sd;

                		temp_port=rand()%1001+3000; 						
						memset(&server_addr,0,sizeof(server_addr));
						server_addr.sin_family=AF_INET;
						server_addr.sin_port=htons(temp_port);
						server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	                
                		if(bind(new_sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
						{
							perror("bind");
							return 1;
						}
                        /*record existed client's information and put it into client_list*/
						info_client *new_client = new info_client;
						info_client_input(received_data->file_name, their_addr,new_client);
						client_list[new_sd] = new_client;
						//clock_gettime(CLOCK_REALTIME,&client_list[new_sd]->record_time);
						k=send_message_rrq(new_sd,received_data, client_list);
						HaveSent[new_sd]=TRUE;
						if(k==-1)//send the requested data to client when receive RRQ
						{       
                            printf("Cannot find the file!\n");
							close(new_sd);
							FD_CLR(new_sd, &master_set); 
							HaveSent[new_sd]=FALSE;

						}
													
						
					}
				}
				
				/* received Ack or error from the client*/
				else
				{
					
					addr_len=sizeof(their_addr);
					if(recvfrom(i,rec_buf,DATA_SIZE,0,(struct sockaddr *)&their_addr,(socklen_t*)&addr_len)<=0)//put received data into rec_buf
               		
					{
						perror("recvfrom");
						return -1;
					}
                	read_message(received_data,rec_buf);//unpack rec_buf and record into read_message
					//clock_gettime(CLOCK_REALTIME,&client_list[i]->record_time);
					if(client_list[i]->block_number == received_data->block_number)
					{
						//FILE **fp=&(client_list[i]->fp);
						//printf("\n fclient_list[i]->len- %d", client_list[i]->len);
						//printf("\n ftell(list[i]->fp)- %ld", ftell(client_list[i]->fp));
						fseek(client_list[i]->fp, -client_list[i]->len, SEEK_CUR);
						//printf("\n ftell(list[i]->fp)- %ld", ftell(client_list[i]->fp));
						//client_list[i]->fp=fp;
						/*send_message_again(i,received_data, client_list);*/
						printf("\n retransmit since received same ACK: send  to client data of length- %d", client_list[i]->len);
						if(send_message_ack(i,received_data, client_list)==-1)//send the requested data to client when receive ACK
						{       
                            printf("transmit completed\n");
							HaveSent[i]=FALSE;
							close(i);
							FD_CLR(i, &master_set); 

						}
					}
					else if(send_message_ack(i,received_data, client_list)==-1)//send the requested data to client when receive ACK
						{       
                            printf("transmit completed\n");
							HaveSent[i]=FALSE;
							close(i);
							FD_CLR(i, &master_set); 

						}
					
				}
				
				delete(received_data);
			}
		}//closing for loop		
	}// forever loop
	
	close(sockfd);
	return 0;
}



int send_message_rrq(int sd, tftp_message *received_data, map<int, info_client *> &list)
{
 	//FILE *fp, *fp_end;
	char* buf;
	int len;	
	socklen_t addr_len=sizeof(struct sockaddr_in);
	map<int, info_client *>::iterator it; 
	printf("\n receive RRQ request frome client for file: %s\n",received_data->file_name);
	printf("\n received_data->block_number0)- %d", received_data->block_number);
	list[sd]->fp = fopen(received_data->file_name, "r");
	if (list[sd]->fp==NULL) //file open fail
	{
		printf("Fail to open file\n"); 
		/* creat error message*/
		uint16 opcode=ERROR;
		uint16 errcode=1;
		char errbuf[ERR_SIZE];
		memset(errbuf,'\0',ERR_SIZE);
		opcode = htons(opcode);
		memcpy(errbuf, &opcode,2);
		errcode = htons(errcode);
		memcpy(errbuf+2, &errcode,2);
		memcpy(errbuf+4,"File not Found",strlen("File not Found"));

		if(sendto(sd,errbuf,ERR_SIZE,0,(struct sockaddr *)&list[sd]->client_addr, addr_len)!=-1)//send the error message
		{
			printf("send a error to client\n");
		} 
		it= list.find(sd);
  		list.erase (it);
		return -1; 		
	}	
	else
	{
		printf("\n successfully open the file - %s", received_data->file_name);
			/* Create sendout_data tftp_message response of RRQ*/
		tftp_message *sendout_data = new tftp_message;
		sendout_data->block_number = 1;
		sendout_data->opcode=DATA;
		
		list[sd]->fp_end = fopen(received_data->file_name, "r");
		fseek(list[sd]->fp_end, 0, SEEK_END);
		
		len = ftell(list[sd]->fp_end) - ftell(list[sd]->fp);
		
		if(len >= DATA_SIZE)
		{
			len = DATA_SIZE;
		}
		else if(len < DATA_SIZE)
		{
			list[sd]->is_last = TRUE;
		}
		list[sd]->len=len;
		make_tftp_message(len, &(list[sd]->fp),buf,sendout_data->opcode,sendout_data->block_number);
			
		sendto(sd,buf,len+4,0,(struct sockaddr *)&list[sd]->client_addr, addr_len);
		printf("\n send  to client data of length- %d", len);
		delete(sendout_data);
	}						
		
	return 0;
}


int send_message_ack(int sd, tftp_message *received_data, map<int, info_client *> &list)
{
 	char* buf;
	int len;	
	socklen_t addr_len=sizeof(struct sockaddr_in);
	map<int, info_client *>::iterator it; 		
	       
	/*server send the next block data to client in response to receive ACK*/
	printf("\n server recieved ACk for block: %d\n", received_data->block_number);
	list[sd]->block_number = received_data->block_number;
	/* Create sendout_data tftp_message response of ACK*/
	tftp_message *sendout_data = new tftp_message;
	sendout_data->opcode=DATA;
	sendout_data->block_number = list[sd]->block_number +1 ;
	//printf("\n ftell(list[sd]->fp_end)- %ld", ftell(list[sd]->fp_end));
	//printf("\n ftell(list[sd]->fp)- %ld", ftell(list[sd]->fp));
	len = ftell(list[sd]->fp_end) - ftell(list[sd]->fp);
	
	if(len >= DATA_SIZE)
	{
		len = DATA_SIZE;
		
	}
	
	else if(len == 0 && list[sd]->is_last == TRUE)// Received ack block number is the last one*/
	{
		fclose(list[sd]->fp);
		it= list.find(sd);
  		list.erase (it);
		
		return -1;
	}
	
	else if(len < DATA_SIZE)
	{
		list[sd]->is_last = TRUE;
	}
	
    list[sd]->len=len;
	
	make_tftp_message(len, &(list[sd]->fp),buf,sendout_data->opcode,sendout_data->block_number);
	
	sendto(sd,buf,len+4,0,(struct sockaddr *)&list[sd]->client_addr, addr_len);
	
	delete(sendout_data);	
	return 0;
}


//int send_message_again(int sd,tftp_message *received_data, map<int, info_client *> &list)
//{
//	char* buf;
//	printf("\n server recieved ACk for block: %d\n", received_data->block_number);
//	list[sd]->block_number = received_data->block_number;
//	socklen_t addr_len=sizeof(struct sockaddr_in);
//	uint16 opcode=DATA;
//	printf("list[sd]->block_number  %d",list[sd]->block_number);
//	uint16 block_number=list[sd]->block_number+1;
//	make_tftp_message(list[sd]->len, &(list[sd]->fp),buf,opcode,block_number);
//	sendto(sd,buf,list[sd]->len+4,0,(struct sockaddr *)&list[sd]->client_addr, addr_len);
//	
//	return 0;
//}

int read_message(tftp_message* received_data,char* rec_buf)
{
	uint16 * temp,len =0, i=0, j=0; //unit16 is unsigned short;	
	char c[2];

	memset(c,'\0',2);
	memcpy(c,rec_buf,2);
	temp = (uint16 *)c;
	received_data->opcode = ntohs(*temp);
	len+=2;	
	/* Read ACK Request message*/
	if(received_data->opcode == ACK)
	{
		memset(c,'\0',2);
		memcpy(c,rec_buf+len,2);
		temp = (uint16 *)c;
		received_data->block_number = ntohs(*temp);
		len+=2;
		
	}
	
	/* Read RRQ Request message*/
	else if(received_data->opcode == RRQ)
	{
		for(i=2, j=0; rec_buf[i]!='\0'; i++, j++)
		received_data->file_name[j] = rec_buf[i];
		received_data->file_name[j] = '\0';   

		printf("%s\n",received_data->file_name);
			
		
	}
		
	/* Read Error message*/
	else if(received_data->opcode == ERROR)
	{
		memset(c,'\0',2);
		memcpy(c,rec_buf+len,2);
		temp = (uint16 *)c;
		received_data->error_info = ntohs(*temp);
		len+=2;
		
		
			
	}
	return 1;
}

void info_client_input(char* file, struct sockaddr_in addr,info_client *new_client)
{
		strcpy(new_client->file_name, file);
		new_client->block_number =0;
		new_client->client_addr = addr;
		new_client->is_last = FALSE;
}


void make_tftp_message(int len, FILE **fp,char *&send_buf,uint16 opcode,uint16 block_number)
		{
			uint16 size=4;
			size += len;
			send_buf = new char[size];
			memset(send_buf,'\0',size);				
			opcode = htons(opcode);
			memcpy(send_buf, &opcode,2);			
			block_number = htons(block_number);
			memcpy(send_buf+2, &block_number,2);			

			fread (send_buf+4,1,len,*fp); // copy the requested file into buf to send		

		}







	

