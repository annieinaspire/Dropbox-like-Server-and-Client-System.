#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include  <arpa/inet.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>

#define MAXLINE 4096

int sleepcount=1;

void my_alarm_handler(int a)
{
    printf("Sleep %d\n",sleepcount++);
}


//!> 注意输入是由stdin，接受是由server发送过来
//!> 所以在client端也是需要select进行处理的
void send_and_recv( int connfd , char * address, char * username)
{
    FILE * fp = stdin;
    int   lens,n,srcfd_num=0,getport_t=0,putport_t=0;
    char send[MAXLINE];
    char recv[MAXLINE];
    char srcfd[FD_SETSIZE][50];
    fd_set rset;
    FD_ZERO( &rset );
    int maxfd = ( fileno( fp ) > connfd ? fileno( fp ) : connfd  + 1 );   

    char clientid[] = "CLIENTID: ";
    strcat(clientid,username);
    write( connfd, clientid, strlen( clientid ) );
   
    while( 1 )
    {
        FD_SET( fileno( fp ), &rset );
        FD_SET( connfd, &rset );            
       
        if( select( maxfd, &rset, NULL, NULL, NULL ) == -1 )
        {
            printf("Client Select Error..\n");
            exit(EXIT_FAILURE);
        }
       
        //!> if 连接口有信息
        if( FD_ISSET( connfd, &rset ) )    //!> if 连接端口有信息
        {
            //printf( "client get from server ...\n" );
            memset( recv, 0, sizeof( recv ) );
            n = read( connfd, recv, MAXLINE );
            if( n == 0 )
            {
                printf("Recv ok...\n");
                break;
            }
            else if( n == -1 )
            {
                printf("Recv error...\n");
                break;
            }
            else
            {
		int og_recv_len = strlen(recv);
		char og_recv[og_recv_len];
		strcpy(og_recv,recv);

		char *func,*sfd,*dfd;
		func = strtok(recv," \n");
		if(!strcmp(func,"GET")){
			//printf("$$$$$ %s $$$$$\n",og_recv);

			while(func!=NULL){

				int og_send_len = strlen(og_recv);
				char og_send[og_send_len];
				strcpy(og_send,og_recv);

				int addr_len = strlen(address);
				char address_t[addr_len];
				strcpy(address_t,address);

				sfd = strtok(NULL," \n");
				dfd = strtok(NULL," \n");
				func = strtok(NULL," \n");
				char * dfd_t = malloc(sizeof(char) * 1024); /* buffer for 1024 chars */
				snprintf(dfd_t, 1024, "%s", dfd);

				char getfile[] = "GET ";strcat(getfile,sfd);strcat(getfile," ");strcat(getfile,dfd);
				write(connfd, getfile, strlen(getfile));
				
				int fd = open(dfd, O_RDWR | O_CREAT, 0644);
    				if (fd == -1)
        				perror("open dest file");

				    char      buff_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
				    
				    //printf("-----%s,%d-----\n",address_t,port_t);

				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(getport_t++);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..1\n");
					exit(EXIT_FAILURE);
				    }

					char size[25],chr;
					int p;
					for(p=0;p<25;p++)
					{
						read(connfd_t,(char*)&chr,1);
						if(chr=='s')
							break;
						size[p]=chr;
					}size[p]='\0';
					int big,og_big;
					og_big=big=atoi(size);

					printf("Downloading file : %s\n",dfd_t);
					int recv_len_t,num,ratio=0,down=0,ratio_t,tt;
					do
					{
						memset(&buff_t, 0, sizeof(buff_t));
						if(big < sizeof(buff_t))num = big;
						else num = sizeof(buff_t);

                				recv_len_t = read(connfd_t, buff_t, num);
						if (recv_len_t < 0) {
						    perror("server temp recv()");
						}
						else if(recv_len_t == 0){
						    
						    printf("Download %s complete!\n",dfd_t);
						    break;
						}
						else{
						    write( fd, buff_t, recv_len_t );
						}

						big-=recv_len_t;
						down+=recv_len_t;
						ratio_t=(50*down)/og_big;
						if(ratio_t > ratio){
							ratio = ratio_t;
							printf("Progress : [");
							for( tt=0;tt<50;tt++){
							    if(tt<=ratio)printf("#");
							    else printf(" ");
							}
							printf("]\n");
							//printf("%d,%d,%d^^\n",ratio,down,og_big);
						}


					}while(1);

					// Clean up
				        close(connfd_t);
		    			close(fd);
			}
		}
		else if(!strcmp(func,"PORT")){
			
			char *getport,*putport;
			int length;
			getport = strtok(NULL," \n");
			putport = strtok(NULL," \n");

			length = strlen( getport );
			getport[length] = '\0';
			getport_t=atoi(getport);
			
			length = strlen( putport );
			putport[length] = '\0';
			putport_t=atoi(putport);

			sfd = strtok(NULL," \n");
			if(sfd!=NULL){
				//printf("%s\n",sfd);
				strcpy(srcfd[srcfd_num],sfd);
				srcfd_num++;

			}
			while(sfd!=NULL)
			{
				sfd = strtok(NULL," \n");
				if(sfd!=NULL){
					//printf("%s\n",sfd);
					strcpy(srcfd[srcfd_num],sfd);
					srcfd_num++;
				}
			}
	
			int q;
			for(q=0;q<srcfd_num;q++){
				char getfile[] = "GET ";
				strcat(getfile,srcfd[q]);strcat(getfile," ");strcat(getfile,srcfd[q]);
				//printf("%s\n",srcfd[q],getfile);
				write( connfd, getfile, strlen(getfile) );

				int og_send_len = strlen(getfile);
				char og_send[og_send_len];
				strcpy(og_send,getfile);

				int addr_len = strlen(address);
				char address_t[addr_len];
				strcpy(address_t,address);

				//printf("11111\n");
				int fd = open(srcfd[q], O_RDWR | O_CREAT, 0644);
    				if (fd == -1)
        				perror("open dest file");

				    char      buff_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
 
				    //printf("-----%s,%d-----\n",address_t,port_t);
				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(getport_t++);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..2\n");
					exit(EXIT_FAILURE);
				    }

					char size[25],chr;
					int p;
					for(p=0;p<25;p++)
					{
						read(connfd_t,(char*)&chr,1);
						if(chr=='s')
							break;
						size[p]=chr;
					}size[p]='\0';
					int big,og_big;
					og_big=big=atoi(size);

					printf("Downloading file : %s\n",srcfd[q]);
					int recv_len_t,num,ratio=0,down=0,ratio_t,tt;
					do
					{
						memset(&buff_t, 0, sizeof(buff_t));
						if(big < sizeof(buff_t))num = big;
						else num = sizeof(buff_t);

                				recv_len_t = read(connfd_t, buff_t, num);
						if (recv_len_t < 0) {
						    perror("server temp recv()");
						}
						else if(recv_len_t == 0){
						    
						    printf("Download %s complete!\n",srcfd[q]);
						    break;
						}
						else{
						    write( fd, buff_t, recv_len_t );
						}

						//printf("server temp receive: len=[%d] msg=[%s]\n", recv_len_t, buff_t);
						big-=recv_len_t;
						down+=recv_len_t;
						ratio_t=(50*down)/og_big;
						if(ratio_t > ratio){
							ratio = ratio_t;
							printf("Progress : [");
							for( tt=0;tt<50;tt++){
							    if(tt<=ratio)printf("#");
							    else printf(" ");
							}
							printf("]\n");
							//printf("%d,%d,%d^^\n",ratio,down,og_big);
						}

					}while(1);

					// Clean up
				        close(connfd_t);
		    			close(fd);
			}
		}
		else{
			lens = strlen( og_recv );
		        og_recv[lens] = '\0';
		        //!> 写到stdout
		        write( STDOUT_FILENO, og_recv, strlen(og_recv) );
		        //printf("\n");
		}
            }

        }
       
        //!> if 有stdin输入
        if( FD_ISSET( fileno( fp ), &rset ) )    //!> if 有输入
        {
            //!> printf("client stdin ...\n");
           
            memset( send, 0, sizeof( send ) );
            if( fgets( send, MAXLINE, fp ) == NULL )
            {
                printf("End...\n");
                exit( EXIT_FAILURE );
            }
            else
            {
                //!>if( str )
                lens = strlen( send );
                send[lens-1] = '\0';

                if( strcmp( send, "/exit" ) == 0 ){
                    //printf( "Bye..\n" );
                    return;
                }
		else if( strcmp( send, "LIST" ) == 0 )write( connfd, send, strlen( send ) );
		else{
                    
			write( connfd, send, strlen( send ) );
				
			int og_send_len = strlen(send);
			char og_send[og_send_len];
			strcpy(og_send,send);

			int addr_len = strlen(address);
			char address_t[addr_len];
			strcpy(address_t,address);

			char *func,*sfd,*dfd,*time;
			func = strtok(send," \n");
			if(!strcmp(func,"/sleep")){

				time = strtok(NULL," \n");
				if(time==NULL)continue;
				int timeout = atoi(time);

				struct itimerval t;
				t.it_interval.tv_usec = 0;
				t.it_interval.tv_sec = 1;
				t.it_value.tv_usec = 0;
				t.it_value.tv_sec = 1;
				if( setitimer( ITIMER_REAL, &t, NULL) < 0 ){
				    printf("settimer error 0\n");
				}
				signal( SIGALRM, my_alarm_handler );

				printf("Client starts to sleep\n");
				for(;;){
				    sleep(timeout);
				    if(sleepcount > timeout){
					t.it_interval.tv_usec = 0;
					t.it_interval.tv_sec = 0;
					t.it_value.tv_usec = 0;
					t.it_value.tv_sec = 0;
					if( setitimer( ITIMER_REAL, &t, NULL) < 0 ){
				    		printf("settimer error 1\n");
					}
					sleepcount=1;
					break;
				    }
				}
				printf("Client wakes up\n");
			}
			if(!strcmp(func,"GET")){
				
				dfd = sfd = strtok(NULL," \n");
				//dfd = strtok(NULL," \n");
				char * dfd_t = malloc(sizeof(char) * 1024); /* buffer for 1024 chars */
				snprintf(dfd_t, 1024, "%s", dfd);

				int fd = open(dfd, O_RDWR | O_CREAT, 0644);
    				if (fd == -1)
        				perror("open dest file");

				    char      buff_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
				    
				    //printf("-----%s,%d-----\n",address_t,port_t);

				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(getport_t++);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..3\n");
					exit(EXIT_FAILURE);
				    }

					char size[25],chr;
					int p;
					for(p=0;p<25;p++)
					{
						read(connfd_t,(char*)&chr,1);
						if(chr=='s')
							break;
						size[p]=chr;
					}size[p]='\0';
					int big,og_big;
					og_big=big=atoi(size);

					printf("Downloading file : %s\n",dfd_t);
					int recv_len_t,num,ratio=0,down=0,ratio_t,tt;
					do
					{
						memset(&buff_t, 0, sizeof(buff_t));
						if(big < sizeof(buff_t))num = big;
						else num = sizeof(buff_t);

                				recv_len_t = read(connfd_t, buff_t, num);
						if (recv_len_t < 0) {
						    perror("server temp recv()");
						}
						else if(recv_len_t == 0){		
			    
						    printf("Download %s complete!\n",dfd_t);
						    break;
						}
						else{
						    write( fd, buff_t, recv_len_t );
						}

						big-=recv_len_t;
						down+=recv_len_t;
						ratio_t=(50*down)/og_big;
						if(ratio_t > ratio){
							ratio = ratio_t;
							printf("Progress : [");
							for( tt=0;tt<50;tt++){
							    if(tt<=ratio)printf("#");
							    else printf(" ");
							}
							printf("]\n");
							//printf("%d,%d,%d^^\n",ratio,down,og_big);
						}


					}while(1);

					// Clean up
				        close(connfd_t);
		    			close(fd);

			}
			else if(!strcmp(func,"/put")){

				dfd = sfd = strtok(NULL," \n");
				//dfd = strtok(NULL," \n");
				int fd = open(sfd, O_RDONLY);
    				if (fd == -1)
        				perror("open src file");

				    char      buf_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
				    
				    //printf("-----%s,%d-----\n",address_t,port_t);

				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(putport_t++);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..4\n");
					exit(EXIT_FAILURE);
				    }

				    struct stat st;//file size
				    stat(sfd, &st);
				    char size[25];
				    sprintf(size,"%llds",st.st_size);
				    write(connfd_t,size,strlen(size));
				
				    int big,og_big,send_len_t,num,ratio=0,up=0,ratio_t,tt;
				    og_big=big=atoi(size);

				    printf("Uploading file : %s\n",dfd);
				    do
				    {
					memset(&send_t, 0, sizeof(send_t));
					if(big < sizeof(send_t))num = big;
					else num = sizeof(send_t);

					send_len_t = read(fd, send_t, num);
					if (send_len_t < 0) {
					    perror("server temp recv()");
					}
					else if(send_len_t == 0){
					    // Clean up
					    close(connfd_t);
    				    	    close(fd);
					    break;
					}
					else{
					    write( connfd_t, send_t, send_len_t );
					}

					num-=send_len_t;
					up+=send_len_t;
					ratio_t=(50*up)/og_big;
					if(ratio_t > ratio){
						ratio = ratio_t;
						printf("Progress : [");
						for( tt=0;tt<50;tt++){
						    if(tt<=ratio)printf("#");
						    else printf(" ");
						}
						printf("]\n");
						//printf("%d,%d,%d^^\n",ratio,down,og_big);
					}

				    }while(1);
				    printf("Upload %s complete!\n",dfd);
			}
			else{}
		}
            }
        }
       
    }
   
}

int main( int argc, char * argv[] )
{
    //!> char * SERV_IP = "10.30.97.188";
    char      buf[MAXLINE];
    int       connfd;
    struct sockaddr_in servaddr;
   
    if( argc != 4 )
    {
        printf("./client <ip> <port> <username>\n");
        return 0;
    }
   
    //!> 建立套接字
    if( ( connfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        printf("Socket Error %d.\n",errno);
        return 0;
    }

    //!> 套接字信息
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
   
    //!> 链接server
    if( connect( connfd, ( struct sockaddr *  )&servaddr, sizeof( servaddr ) ) < 0 )
    {
        printf("Connect error..\n");
        exit(EXIT_FAILURE);
    }   
   
    //!> send and recv
    send_and_recv( connfd , argv[1], argv[3] );

    close( connfd );
    //printf("Exit\n");
   
    return 0;
}

