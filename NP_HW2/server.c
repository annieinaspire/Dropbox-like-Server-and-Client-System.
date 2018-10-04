#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 4096

struct dir{
    char* filelist;
    char* id;
};


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
    int			i,j,k,flag,dirnum=0;
    char                buff[BUF_SIZE];
    char		*c_dirname[FD_SETSIZE];
    struct dir 		exist_dir[FD_SETSIZE];
    int			getport_t[FD_SETSIZE],putport_t[FD_SETSIZE];

    flag=fcntl(sock_fd,F_GETFL,0);
    fcntl(sock_fd,F_SETFL,flag|O_NONBLOCK);

    if( argc != 2 )
    {
        printf("./server <port>\n");
        return 0;
    }

    unsigned seed;
    seed = (unsigned)time(NULL); // 取得時間序列
    srand(seed); // 以時間序列當亂數種子

    for(i=0;i < FD_SETSIZE; i++){
	exist_dir[i].filelist=NULL;
	exist_dir[i].id=NULL;
	c_dirname[i]=NULL;
	getport_t[i]=0;
	putport_t[i]=0;
    }

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
						memset(&client_addr, 0, sizeof(client_addr));
						len = sizeof(client_addr);
                        new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);
                        if (new_fd == -1) {
                            perror("accept()");
                            return -1;
                        } 
						else {
							char str[INET_ADDRSTRLEN];
							inet_ntop(AF_INET,&client_addr.sin_addr,str,INET_ADDRSTRLEN);

							flag=fcntl(new_fd,F_GETFL,0);
							fcntl(new_fd,F_SETFL,flag|O_NONBLOCK);

                        	/* Add to fd set */
                            FD_SET(new_fd, &active_fd_set);
                            if (new_fd > max_fd)max_fd = new_fd;
                        }
                    }
					else {
                        /* Data arriving on an already-connected socket */
                        int recv_len;
 
                        /* Receive */
                        memset(&buff, 0, sizeof(buff));
                        recv_len = read(i, buff, sizeof(buff));
                        if (recv_len == -1) {
                            perror("recv()");
                            return -1;
                        }
						else if (recv_len == 0) {
                            //printf("Client disconnect\n");
				
							for (j = 0; j < FD_SETSIZE; j++){ 
								if( exist_dir[j].id!=NULL && !strcmp(exist_dir[j].id,c_dirname[i]) ){ break; } 
							}
							free(c_dirname[i]);
							c_dirname[i]=NULL;
							getport_t[i]=0;
							putport_t[i]=0;
							
							/* Clean up */
                            close(i);
                            FD_CLR(i, &active_fd_set);
                        }
						else {
                        	//printf("Receive: len=[%d] msg=[%s]\n", recv_len, buff);
 
	                        /* Send (In fact we should determine when it can be written)*/


				char *func,*sfd,*dfd,*id;
				func = strtok(buff," \n");
				if(!strcmp(func,"CLIENTID:")){
					id = strtok(NULL," \n");

					c_dirname[i] = malloc(sizeof(char) * 1024); /* buffer for 1024 chars */
					snprintf(c_dirname[i], 1024, "%s", id);

					int flag=1;
					for (j = 0; j < FD_SETSIZE; j++){ 
						if( exist_dir[j].id!=NULL && !strcmp(exist_dir[j].id,id) ){
							flag=0;
							break;
						} 
					}

					if(flag){

						char path[] = "./";
						strcat(path,id);
						mkdir(path, 0700);

						exist_dir[dirnum].id = malloc(sizeof(char) * 1024); /* buffer for 1024 chars */
						snprintf(exist_dir[dirnum].id, 1024, "%s", id);
						dirnum++;

					}
					//printf("%s,%s,%d^^\n",exist_dir[j].id,exist_dir[j].filelist,j);
					
					char welcome[] = "Welcome to the dropbox-like server! : ";strcat(welcome,id);strcat(welcome,"\n");
					write( i, welcome, strlen(welcome) );

					char port_t[] = "PORT ";
					getport_t[i] = rand() % 10000;
					if(getport_t[i] < 1000)getport_t[i]=getport_t[i]+1000;
					char temp_getport[4];itoa(getport_t[i],temp_getport);reverse_string(temp_getport);
					strcat(port_t,temp_getport);strcat(port_t," ");
					putport_t[i] = rand() % 10000;
					if(putport_t[i] < 1000)putport_t[i]=putport_t[i]+1000;
					char temp_putport[4];itoa(putport_t[i],temp_putport);reverse_string(temp_putport);
					strcat(port_t,temp_putport);
					
					//printf("%d,%s$$$\n",j,exist_dir[j].filelist);
					if(exist_dir[j].filelist!=NULL){

						strcat(port_t," ");strcat(port_t,exist_dir[j].filelist);strcat(port_t,"\n");
					}

					//printf("%d,%d###\n",getport_t[i],putport_t[i]);
					write(i, port_t, strlen(port_t));
				}
				else if(!strcmp(func,"GET")){

					dfd = sfd = strtok(NULL," \n");
					//dfd = strtok(NULL," \n");
					
					char sfd_path[] = "./";
					strcat(sfd_path,c_dirname[i]);
					strcat(sfd_path,"/");
					strcat(sfd_path,sfd);
					//printf("*****%s,%d*****\n",sfd_path,c_dirname[i]);

					int fd = open(sfd_path, O_RDONLY);
	    				if (fd == -1)
						perror("open src file");

					    struct sockaddr_in  servaddr,cliaddr;
					    socklen_t           length;
					    int                 s_temp_fd,c_temp_fd;
					    char		send_t[BUF_SIZE];

					    //flag=fcntl(s_temp_fd,F_GETFL,0);
					    //fcntl(s_temp_fd,F_SETFL,flag|O_NONBLOCK);

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
					    servaddr.sin_port = htons(getport_t[i]++);
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

						    //flag=fcntl(c_temp_fd,F_GETFL,0);
						    //fcntl(c_temp_fd,F_SETFL,flag|O_NONBLOCK);

						    struct stat st;//file size
						    stat(sfd_path, &st);
						    char size[25];
						    sprintf(size,"%llds",st.st_size);
						    write(c_temp_fd,size,strlen(size));
				
						    int big,send_len_t,num;
						    big=atoi(size);

						    do
						    {
							memset(&send_t, 0, sizeof(send_t));
							if(big < sizeof(send_t))num = big;
							else num = sizeof(send_t);

							send_len_t = read(fd, send_t, num);
							if (send_len_t < 0) {
							    perror("server temp send()");
							}
							else if(send_len_t == 0){
							    break;
							}
							else{
							    write( c_temp_fd, send_t, send_len_t );
							}

							num-=send_len_t;

						    }while(1);

							// Clean up
							close(s_temp_fd);
							close(c_temp_fd);
		    				    	close(fd);

						}

				}
				else if(!strcmp(func,"/put")){

					dfd = sfd = strtok(NULL," \n");
					//dfd = strtok(NULL," \n");

					char getfile[] = "GET ";
					strcat(getfile,dfd);strcat(getfile," ");strcat(getfile,dfd);strcat(getfile," ");
					
					char dfd_path[] = "./";
					strcat(dfd_path,c_dirname[i]);
					strcat(dfd_path,"/");
					strcat(dfd_path,dfd);
					//printf("*****%s,%s*****\n",dfd_path,c_dirname[i]);

					int fd = open(dfd_path, O_RDWR | O_CREAT, 0644);
	    				if (fd == -1)
						perror("open dest file");

					for (j = 0; j < FD_SETSIZE; j++){ 
						if( exist_dir[j].id!=NULL && !strcmp(exist_dir[j].id,c_dirname[i]) ){ break; } 
					}

					if(exist_dir[j].filelist==NULL){
						//exist_dir[j].filelist=(char*)strdup(dfd);
						exist_dir[j].filelist = malloc(sizeof(char) * 1024); /* buffer for 1024 chars */
						snprintf(exist_dir[j].filelist, 1024, "%s", dfd);
					}
					else {
						strcat(exist_dir[j].filelist," ");strcat(exist_dir[j].filelist,dfd);
					}
					//printf("*****%s,%d*****\n",exist_dir[j].filelist,j);

					    struct sockaddr_in  servaddr,cliaddr;
					    socklen_t           length;
					    int                 s_temp_fd,c_temp_fd;
					    char		buff_t[BUF_SIZE];

					    //flag=fcntl(s_temp_fd,F_GETFL,0);
					    //fcntl(s_temp_fd,F_SETFL,flag|O_NONBLOCK);

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
					    servaddr.sin_port = htons(putport_t[i]++);
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

							//flag=fcntl(c_temp_fd,F_GETFL,0);
							//fcntl(c_temp_fd,F_SETFL,flag|O_NONBLOCK);

							char size[25],chr;
							int p;
							for(p=0;p<25;p++)
							{
								read(c_temp_fd,(char*)&chr,1);
								if(chr=='s')
									break;
								size[p]=chr;
							}size[p]='\0';
							int big;
							big=atoi(size);

							int recv_len_t,num;
							do
							{
								memset(&buff_t, 0, sizeof(buff_t));
								if(big < sizeof(buff_t))num = big;
								else num = sizeof(buff_t);

		                				recv_len_t = read(c_temp_fd, buff_t, num);
								if (recv_len_t < 0) {
								    perror("server temp recv()");
								    return -1;
								}
								else if(recv_len_t == 0){
								    // Clean up
								    close(s_temp_fd);
								    close(c_temp_fd);
								    close(fd);
								    break;
								}
								else{
								    write( fd, buff_t, recv_len_t );
								}
								//printf("server temp receive: len=[%d] msg=[%s]\n", recv_len_t, buff_t);
								big-=recv_len_t;

							}while(1);
						}
						
					for (k = 4; k <= max_fd; k++){ 
						if( c_dirname[k]!=NULL ){
							//printf("%d,%s,%s\n",k,c_dirname[k],c_dirname[i]);
							if( !strcmp(c_dirname[k],c_dirname[i]) && k!=i){
								write(k, getfile, strlen(getfile));
							}
						} 
					}
				}
				else if(!strcmp(func,"LIST")){
					for (j = 0; j < FD_SETSIZE; j++){ 
						if( exist_dir[j].id!=NULL && !strcmp(exist_dir[j].id,c_dirname[i]) ){ break; } 
					}

					if(exist_dir[j].filelist==NULL){

						write( i, "LIST suceeded\n", strlen("LIST suceeded\n") );					
						continue;
					}

					int fname_len = strlen(exist_dir[j].filelist);
					char og_fname[fname_len];
					strcpy(og_fname,exist_dir[j].filelist);

					char *fname = strtok(og_fname," \n");
					if(fname!=NULL){
						//printf("%s\n",fname);
						int fname_len_t = strlen(exist_dir[j].filelist);
						char fname_t[fname_len];
						strcpy(fname_t,fname);
						strcat(fname_t,"\n");
						write( i, fname_t, strlen(fname_t) );
					}
					while(fname!=NULL)
					{
					    fname = strtok(NULL," \n");
					    if(fname!=NULL){
						//printf("%s\n",fname);
						int fname_len_t = strlen(exist_dir[j].filelist);
						char fname_t[fname_len];
						strcpy(fname_t,fname);
						strcat(fname_t,"\n");
						write( i, fname_t, strlen(fname_t) );
					    }		
					}

					write( i, "LIST suceeded\n", strlen("LIST suceeded\n") );

				}
				else{
					
				}

                        }
                    }
                } // end of if
            } //end of for
        } // end of if
    } // end of while
     
    return 0;
}
