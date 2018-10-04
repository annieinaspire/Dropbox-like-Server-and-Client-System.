#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SVR_IP                          "127.0.0.1"
#define SVR_PORT                        1128
#define BUF_SIZE                        1024

void reverse_string(char *str)
{
    /* skip null */
    if (str == 0)
    {
    	return;
    }

    /* skip empty string */
    if (*str == 0)
    {
    	return;
    }

    /* get range */
    char *start = str;
    char *end = start + strlen(str) - 1; /* -1 for \0 */
    char temp;

    /* reverse */
    while (end > start)
    {
    	/* swap */
    	temp = *start;
    	*start = *end;
    	*end = temp;

    	/* move */
    	++start;
    	--end;
    }
}

void itoa (int n,char s[])
{
    int i,j,sign;
    if((sign=n)<0)//记录符号
    	n=-n;//使n成为正数
    i=0;
    do{
       s[i++]=n%10+'0';//取下一个数字
    }while ((n/=10)>0);//删除该数字
    
    if(sign<0)
	s[i++]='-';
    s[i]='\0';
    //for(j=i;j>=0;j--)//生成的数字是逆序的，所以要逆序输出
       //printf("%c",s[j]);
}


int main (int argc, char **argv) {
    struct sockaddr_in  server_addr;
    socklen_t           len;
    fd_set              active_fd_set;
    int                 sock_fd;
    int                 max_fd;
    int			i;
    int                 flag = 1;
    char                buff[BUF_SIZE];
    char		*client[FD_SETSIZE];
    char		*c_addr[FD_SETSIZE];
    char		*c_port[FD_SETSIZE];

    for(i=0;i<FD_SETSIZE;i++){client[i]=NULL;c_addr[i]=NULL;c_port[i]=NULL;}
     
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SVR_IP);
    server_addr.sin_port = htons(SVR_PORT);
    len = sizeof(struct sockaddr_in);
 
    /* Create endpoint */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    } else {
        printf("sock_fd=[%d]\n", sock_fd);
    }
 
    /* Set socket option */
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) {
        perror("setsockopt()");
        return -1;
    }
 
    /* Bind */
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        return -1;
    } else {
        printf("bind [%s:%u] success\n",
            inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    }
 
    /* Listen */
    if (listen(sock_fd, 128) == -1) {
        perror("listen()");
        return -1;
    }
 
    FD_ZERO(&active_fd_set);
    FD_SET(sock_fd, &active_fd_set);
    max_fd = sock_fd;
 
    while (1) {
        int             ret;
        struct timeval  tv;
        fd_set          read_fds;
 
        /* Set timeout */
        //tv.tv_sec = 2;
        //tv.tv_usec = 0;
 
        /* Copy fd set */
        read_fds = active_fd_set;
        //ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
	ret = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select()");
            return -1;
        } else if (ret == 0) {
            printf("select timeout\n");
            continue;
        } else {
            int i;
 
            /* Service all sockets */
            for (i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(i, &read_fds)) {
                    if (i == sock_fd) {
                        /* Connection request on original socket. */
                        struct sockaddr_in  client_addr;
                        int                 new_fd;
 
                        /* Accept */
                        new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);
                        if (new_fd == -1) {
                            perror("accept()");
                            return -1;
                        } else {
                            printf("Accpet client come from [%s:%u] by fd [%d]\n",
                                inet_ntoa(client_addr.sin_addr),
                                ntohs(client_addr.sin_port), new_fd);

                            /* Add to fd set */
                            FD_SET(new_fd, &active_fd_set);
                            if (new_fd > max_fd)
                                max_fd = new_fd;

			    int j;
			    for(j=4;j<=max_fd;j++){
				if(j!=new_fd)write(j, "[Server] Someone is coming!\n", strlen("[Server] Someone is coming!\n"));
			    }

			    client[new_fd]="anonymous";
			    c_addr[new_fd]=strdup(inet_ntoa(client_addr.sin_addr));
			    int value = (int)ntohs(client_addr.sin_port);
			    char temp_str[5];itoa(value,temp_str);reverse_string(temp_str);
			    c_port[new_fd]=strdup(temp_str);

			    char send_str0[] = "[Server] Hello, anonymous! From: ";
			    strcat(send_str0,c_addr[new_fd]);
			    strcat(send_str0,"/");
				
			    //strcat(send_str1,temp_str);
			    int len = strlen(temp_str);
				char send_str1[len];
				strcpy(send_str1,temp_str);
				strcat(send_str1,"\n");

			    //strcat(send_str0,"<");
			    //strcat(send_str0,temp_str);
			    //strcat(send_str0,">\n");

			    write(new_fd, send_str0, strlen(send_str0)); //bad file descriptor if string is too long
			    write(new_fd, send_str1, strlen(send_str1));

			    
                        }
                    } else {
                        /* Data arriving on an already-connected socket */
                        int recv_len;
 
                        /* Receive */
                        memset(buff, 0, sizeof(buff));
                        recv_len = recv(i, buff, sizeof(buff), 0);
                        if (recv_len == -1) {
                            perror("recv()");
                            return -1;
                        } else if (recv_len == 0) {
                            printf("Client disconnect\n");

			    

			    int j;
			    char send_str[]="[Server] ";
			    strcat(send_str,client[i]);
			    strcat(send_str," is offline.\n");
			    client[i]=NULL;c_addr[i]=NULL;c_port[i]=NULL;
			    for(j=4;j<=max_fd;j++){
				if(client[j]!=NULL)write(j, send_str, strlen(send_str));
			    }

			    
			    
			    /* Clean up */
                            close(i);
                            FD_CLR(i, &active_fd_set);
                        } else {
                            //printf("Receive: len=[%d] msg=[%s]\n", recv_len, buff);
 
                            /* Send (In fact we should determine when it can be written)*/

			    if(!strcmp(buff,"who")){
				int j;
			    	
			    	for(j=4;j<=max_fd;j++){
				    if(client[j]!=NULL){
					char send_str[]="[Server] ";
					strcat(send_str,client[j]);
			    	strcat(send_str," ");
					strcat(send_str,c_addr[j]);
					strcat(send_str,"/");
					strcat(send_str,c_port[j]);
						if(j==i){
							strcat(send_str," ->me\n");
							write(i, send_str, strlen(send_str));
						}
						else{
							strcat(send_str,"\n");
							write(i, send_str, strlen(send_str));
						}
				    }
					
			    	}
			    }
                else{

					char *pch;
					pch = strtok(buff," ");

					if(!strcmp(pch,"yell")){
						pch = strtok(NULL,"\0");
						if(pch==NULL)continue;
						char sender_name[] = "[server] ";
						strcat(sender_name,client[i]);
						strcat(sender_name," yell ");
						strcat(sender_name,pch);
						strcat(sender_name,"\n");
						int j;
				    		for(j=4;j<=max_fd;j++){
							if(client[j]!=NULL)write(j, sender_name, strlen(sender_name));
				    		}
					}
					else if(!strcmp(pch,"name")){
						pch = strtok(NULL,"\0");
						if(pch==NULL)continue;
						int j,ascii,fit=1,len=strlen(pch);

						for(j=0;j<len;j++){
							ascii = (int)pch[j];
							if((ascii>=65 && ascii<=90)||(ascii>=97 && ascii<=122)){}
							else{//printf("11111\n");
								fit=0;
								write(i, "[Server] ERROR: Username can only consists of 2~12 English letters.\n", 
									strlen("[Server] ERROR: Username can only consists of 2~12 English letters.\n"));
								break;
							}
						}

						if(fit){

							if(len < 2 || len > 12){//printf("22222\n");
								fit=0;
								write(i, "[Server] ERROR: Username can only consists of 2~12 English letters.\n", 
									strlen("[Server] ERROR: Username can only consists of 2~12 English letters.\n"));
							}
							else if(!strcmp(pch,"anonymous")){//printf("33333\n");
								fit=0;
								write(i, "[Server] ERROR: Username cannot be anonymous.\n", 
									strlen("[Server] ERROR: Username cannot be anonymous.\n"));
							}
							else{
						    		for(j=4;j<=max_fd;j++){
									if(client[j]!=NULL){
										if(!strcmp(pch,client[j])){
											if(j==i){break;}
											else{//printf("44444\n");
												fit=0;
												char send_str[] = "[Server] ERROR: ";
												strcat(send_str,pch);
												strcat(send_str," has been used by others.\n");
												write(i, send_str, strlen(send_str));
												break;
											}
										}
									}
								}

								if(fit){//printf("55555\n");
									for(j=4;j<=max_fd;j++){
										if(client[j]==NULL){}
										else if(j==i){
											char send_str2[] = "[server] You're now known as ";
											strcat(send_str2,pch);
											strcat(send_str2,".\n");
											write(i, send_str2, strlen(send_str2));
										}
										else{
											char send_str3[]= "[server] ";
											strcat(send_str3,client[i]);
											strcat(send_str3," is now known as ");
											strcat(send_str3,pch);
											strcat(send_str3,".\n");
											write(j, send_str3, strlen(send_str3));
										}
									}

									client[i]=strdup(pch);
								}
							}
						}
					}
					else if(!strcmp(pch,"tell")){
						pch = strtok(NULL," ");
						if(pch==NULL)continue;
						char *msg = strtok(NULL,"\0");
						if(msg==NULL)continue;
						int j,flag=1;

						if(!strcmp(client[i],"anonymous")){
							write(i, "[Server] ERROR: You are anonymous.\n", strlen("[Server] ERROR: You are anonymous.\n"));
						}
						else if(!strcmp(pch,"anonymous")){
							write(i, "[Server] ERROR: The client to which you sent is anonymous.\n", 
								strlen("[Server] ERROR: The client to which you sent is anonymous.\n"));
						}
						else{
							for(j=4;j<=max_fd;j++){
								if(client[j]==NULL){}
								else if(!strcmp(client[j],pch)){
									flag=0;
									char send_str4[] = "[server] ";
									strcat(send_str4,client[i]);
									strcat(send_str4," tell you ");
									strcat(send_str4,msg);
									strcat(send_str4,"\n");
									write(j, send_str4, strlen(send_str4));
									write(i, "[Server] SUCCESS: Your message has been sent.\n", 
										strlen("[Server] SUCCESS: Your message has been sent.\n"));
									break;
								}
							}

							if(flag){
								write(i, "[Server] ERROR: The receiver doesn't exist.\n", 
									strlen("[Server] ERROR: The receiver doesn't exist.\n"));
							}

						}
					}
					else{write(i, "[Server] ERROR: Error command.\n", strlen("[Server] ERROR: Error command.\n"));}
			    }
                        }
 
                        /* Clean up */
                        //close(i);
                        //FD_CLR(i, &active_fd_set);
                    }
 
                } // end of if
            } //end of for
        } // end of if
    } // end of while
     
    return 0;
}
