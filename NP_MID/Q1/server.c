#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define BUF_SIZE 102400

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

void sig_chld(int signo)
{
	pid_t 	pid;
	int   	stat;

	while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
	    //printf("child %d terminated\n",pid);
	}
	return;
}

int main (int argc, char **argv) {
    pid_t		childpid;
    struct sockaddr_in  server_addr;
    socklen_t           len;
    fd_set              active_fd_set;
    int                 sock_fd;
    int                 max_fd;
    int			i,j;
    int                 flag = 1;
    char                buff[BUF_SIZE];
    char		*client;
    //int		c_dirname;

    void		sig_chld(int);

    unsigned seed;
    seed = (unsigned)time(NULL); // 取得時間序列
    srand(seed); // 以時間序列當亂數種子

    client = NULL;
    //c_dirname = -1;

    /* Create endpoint */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        return -1;
    } else {
        printf("sock_fd=[%d]\n", sock_fd);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    len = sizeof(struct sockaddr_in);
 
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
 
    //FD_ZERO(&active_fd_set);
    //FD_SET(sock_fd, &active_fd_set);
    //max_fd = sock_fd;

    signal(SIGCHLD,sig_chld);
 
    while (1) {
        //int             ret;
        //struct timeval  tv;
        //fd_set          read_fds;
 
        /* Set timeout */
        //tv.tv_sec = 2;
        //tv.tv_usec = 0;
 
        /* Copy fd set */
        //read_fds = active_fd_set;
        //ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
	/*ret = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select()");
            return -1;
        } else if (ret == 0) {
            printf("select timeout\n");
            continue;
        } else {
            int i;*/
 
            /* Service all sockets */
            //for (i = 0; i < FD_SETSIZE; i++) {
                //if (FD_ISSET(i, &read_fds)) {
                    //if (i == sock_fd) {
                        /* Connection request on original socket. */
                        struct sockaddr_in  client_addr;
                        int                 new_fd;
 
                        /* Accept */
			memset(&client_addr, 0, sizeof(client_addr));
			len = sizeof(client_addr);
                        new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);
                        if (new_fd == -1) {
                            perror("accept()");
                            return -1;
                        } 
			else {
				//if( (childpid=fork()) == 0){

					//close(sock_fd);
				
					char str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET,&client_addr.sin_addr,str,INET_ADDRSTRLEN);

					//int pid = getpid();
					//printf("###%d###\n",pid);

					/*c_dirname=pid;
					char path[] = "./";
					char dirname[5];itoa(c_dirname,dirname);reverse_string(dirname);
					strcat(path,dirname);
					mkdir(path, 0700);*/

		                	/* Add to fd set */
		                        //FD_SET(new_fd, &active_fd_set);
		                    	//if (new_fd > max_fd)max_fd = new_fd;

					for(;;){

						/* Data arriving on an already-connected socket */
						int recv_len;
			 
						/* Receive */
						memset(&buff, 0, sizeof(buff));
						recv_len = read(new_fd, buff, sizeof(buff));
						if (recv_len == -1) {
						    perror("recv()");
						    return -1;
						}
						else if (recv_len == 0) {
						    printf("Client disconnect\n");

							char *fname = strtok(client," \n");
							if(fname!=NULL)
							{
								/*char dirname[5];itoa(c_dirname,dirname);reverse_string(dirname);
								char path[] = "./"; 
								strcat(path,dirname);strcat(path,"/");strcat(path,fname);
								unlink(path);*/
								unlink(fname);
							}
							while(fname!=NULL)
							{
							    fname = strtok(NULL," \n");
							    if(fname!=NULL)
							    {
								/*char dirname[5];itoa(c_dirname,dirname);reverse_string(dirname);
								char path[] = "./"; 
								strcat(path,dirname);strcat(path,"/");strcat(path,fname);
								unlink(path);*/
								unlink(fname);	
							    }
							}
							client=NULL;
							//rmdir(dirname);
							//c_dirname=-1;
						    
						    /* Clean up */
						    close(new_fd);
						    break;
						}
						else {
							printf("Receive: len=[%d] msg=[%s]\n", recv_len, buff);
			 
							/* Send (In fact we should determine when it can be written)*/

							char *func,*sfd,*dfd;
							func = strtok(buff," \n");
							if(!strcmp(func,"GET")){

								//printf("11111\n");
								sfd = strtok(NULL," \n");
								dfd = strtok(NULL," \n");
					
								/*char sfd_path[] = "./";
								char dirname[5];itoa(c_dirname,dirname);reverse_string(dirname);
								strcat(sfd_path,dirname);
								strcat(sfd_path,"/");
								strcat(sfd_path,sfd);*/
								//printf("*****%s,%d*****\n",sfd_path,c_dirname);

								int fd = open(sfd, O_RDONLY);
				    				if (fd == -1)
									perror("open src file");


								    struct sockaddr_in  servaddr,cliaddr;
								    socklen_t           length;
								    int                 s_temp_fd,c_temp_fd;
								    char		send_t[BUF_SIZE];

								    int port_t = rand() % 10000;
								    if(port_t < 1000)port_t=port_t+1000;
								    char temp_port[4];itoa(port_t,temp_port);reverse_string(temp_port);
								    write(new_fd, temp_port, strlen(temp_port));

								    /* Create endpoint */
								    if ((s_temp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
									perror("server temp socket()");
									return -1;
								    } else {
									//printf("s_temp_fd=[%d]\n", s_temp_fd);
								    }

								    memset(&servaddr, 0, sizeof(servaddr));
								    servaddr.sin_family = AF_INET;
								    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
								    servaddr.sin_port = htons(port_t);
								    length = sizeof(struct sockaddr_in);
								 
								    /* Bind */
								    if (bind(s_temp_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
									perror("server temp bind()");
									return -1;
								    } else {
									//printf("server temp bind [%s:%u] success\n",
									    //inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
								    }
								 
								    /* Listen */
								    if (listen(s_temp_fd, 128) == -1) {
									perror("server temp listen()");
									return -1;
								    }


									memset(&cliaddr, 0, sizeof(cliaddr));
									length = sizeof(cliaddr);
									c_temp_fd = accept(s_temp_fd, (struct sockaddr *)&cliaddr, &length);
									if (c_temp_fd == -1) {
									    perror("server temp accept()");
									    return -1;
									} 
									else {
										char s[INET_ADDRSTRLEN];
										inet_ntop(AF_INET,&cliaddr.sin_addr,s,INET_ADDRSTRLEN);

										    memset( send_t, 0, sizeof( send_t ) );
										    while( read(fd, send_t, sizeof(send_t)) != 0 )
										    {
											write( c_temp_fd, send_t, strlen( send_t ) );
										    }

										close(s_temp_fd);
										close(c_temp_fd);
										close(fd);
									}

							}
							else if(!strcmp(func,"PUT")){

								//printf("22222\n");
								sfd = strtok(NULL," \n");
								dfd = strtok(NULL," \n");

					
								/*char dfd_path[] = "./";
								char dirname[5];itoa(c_dirname,dirname);reverse_string(dirname);
								strcat(dfd_path,dirname);
								strcat(dfd_path,"/");
								strcat(dfd_path,dfd);*/
								//printf("*****%s,%d*****\n",dfd_path,c_dirname);

								int fd = open(dfd, O_RDWR | O_CREAT, 0644);
				    				if (fd == -1)
									perror("open dest file");

								if(client==NULL){client=strdup(dfd);}
								else {strcat(client," ");strcat(client,strdup(dfd));}
								//printf("*****%s*****\n",client);

								    struct sockaddr_in  servaddr,cliaddr;
								    socklen_t           length;
								    int                 s_temp_fd,c_temp_fd;
								    char		buff_t[BUF_SIZE];

								    int port_t = rand() % 10000;
								    if(port_t < 1000)port_t=port_t+1000;
								    char temp_port[4];itoa(port_t,temp_port);reverse_string(temp_port);
								    write(new_fd, temp_port, strlen(temp_port));

								    /* Create endpoint */
								    if ((s_temp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
									perror("server temp socket()");
									return -1;
								    } else {
									//printf("s_temp_fd=[%d]\n", s_temp_fd);
								    }

								    memset(&servaddr, 0, sizeof(servaddr));
								    servaddr.sin_family = AF_INET;
								    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
								    servaddr.sin_port = htons(port_t);
								    length = sizeof(struct sockaddr_in);
								 
								    /* Bind */
								    if (bind(s_temp_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
									perror("server temp bind()");
									return -1;
								    } else {
									//printf("server temp bind [%s:%u] success\n",
									    //inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
								    }
								 
								    /* Listen */
								    if (listen(s_temp_fd, 128) == -1) {
									perror("server temp listen()");
									return -1;
								    }


									memset(&cliaddr, 0, sizeof(cliaddr));
									length = sizeof(cliaddr);
									c_temp_fd = accept(s_temp_fd, (struct sockaddr *)&cliaddr, &length);
									if (c_temp_fd == -1) {
									    perror("server temp accept()");
									    return -1;
									} 
									else {
										char s[INET_ADDRSTRLEN];
										inet_ntop(AF_INET,&cliaddr.sin_addr,s,INET_ADDRSTRLEN);

										int recv_len_t;
										for(;;){

											memset(&buff_t, 0, sizeof(buff_t));
											recv_len_t = read(c_temp_fd, buff_t, sizeof(buff_t));
											if (recv_len_t == -1) {
											    perror("server temp recv()");
											    return -1;
											}
											else if (recv_len_t == 0) {
											    //printf("server temp client disconnect\n");
											    
											    /* Clean up */
											    close(s_temp_fd);
											    close(c_temp_fd);
											    close(fd);
								
											    break;
											}
											else {
									//printf("server temp receive: len=[%d] msg=[%s]\n", recv_len_t, buff_t);
												write( fd, buff_t, recv_len_t );
											}
										}
									}

							}
							else if(!strcmp(func,"LIST")){

								if(client==NULL){

									write( new_fd, "LIST suceeded\n", strlen("LIST suceeded\n") );					
									continue;
								}

								int fname_len = strlen(client);
								char og_fname[fname_len];
								strcpy(og_fname,client);

								char *fname = strtok(og_fname," \n");
								if(fname!=NULL){
									//printf("%s\n",fname);
									int fname_len_t = strlen(client);
									char fname_t[fname_len];
									strcpy(fname_t,fname);
									strcat(fname_t,"\n");
									write( new_fd, fname_t, strlen(fname_t) );
								}
								while(fname!=NULL)
								{
								    fname = strtok(NULL," \n");
								    if(fname!=NULL){
									//printf("%s\n",fname);
									int fname_len_t = strlen(client);
									char fname_t[fname_len];
									strcpy(fname_t,fname);
									strcat(fname_t,"\n");
									write( new_fd, fname_t, strlen(fname_t) );
								    }		
								}

								write( new_fd, "LIST suceeded\n", strlen("LIST suceeded\n") );

							}
							else{
					
							}

						}
					
					}

					//exit(0);

				//}
				//close(new_fd);

                        }
                    //}
		    //else {

                    //}
                //} // end of if
            //} //end of for
        //} // end of if
    } // end of while
     
    return 0;
}

