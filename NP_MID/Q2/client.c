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

#define MAXLINE 102400

//!> 注意输入是由stdin，接受是由server发送过来
//!> 所以在client端也是需要select进行处理的
void send_and_recv( int connfd , char * address)
{
    FILE * fp = stdin;
    int   lens;
    char send[MAXLINE];
    char recv[MAXLINE];
    fd_set rset;
    FD_ZERO( &rset );
    int maxfd = ( fileno( fp ) > connfd ? fileno( fp ) : connfd  + 1 );   
    int n;
   
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
                lens = strlen( recv );
                recv[lens] = '\0';
                //!> 写到stdout
                write( STDOUT_FILENO, recv, MAXLINE );
                //printf("\n");
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

                if( strcmp( send, "EXIT" ) == 0 ){
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

			char *func,*sfd,*dfd;
			func = strtok(send," \n");
			if(!strcmp(func,"GET")){
				
				//printf("11111\n");
				sfd = strtok(NULL," \n");
				dfd = strtok(NULL," \n");
				int fd = open(dfd, O_RDWR | O_CREAT, 0644);
    				if (fd == -1)
        				perror("open dest file");

				    char      buff_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
				    
				    memset( recv, 0, sizeof( recv ) );
				    n = read( connfd, recv, MAXLINE );
				    if( n == 0 )printf("Recv ok...\n");
				    else if( n == -1 )printf("Recv error...\n");
				    else
				    {
					lens = strlen( recv );
					recv[lens] = '\0';
					//!> 写到stdout
					port_t=atoi(recv);

				    }

				    
				    //printf("-----%s,%d-----\n",address_t,port_t);

				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(port_t);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..\n");
					exit(EXIT_FAILURE);
				    }

				    
					int recv_len_t;
					for(;;){

						memset(&buff_t, 0, sizeof(buff_t));
		                		recv_len_t = read(connfd_t, buff_t, sizeof(buff_t));
						if (recv_len_t == -1) {
						    perror("client temp recv()");
						}
						else if (recv_len_t == 0) {

						    strcat(og_send," suceeded");
						    printf("%s\n",og_send);

						    close(connfd_t);
		    				    close(fd);

						    break;
						}
						else {
							//printf("client temp receive: len=[%d] msg=[%s]\n", recv_len_t, buff_t);
							write( fd, buff_t, recv_len_t );
						}
					}

			}
			else if(!strcmp(func,"PUT")){

				//printf("22222\n");
				sfd = strtok(NULL," \n");
				dfd = strtok(NULL," \n");
				int fd = open(sfd, O_RDONLY);
    				if (fd == -1)
        				perror("open src file");

				    char      buf_t[MAXLINE];
				    int       connfd_t;
				    struct    sockaddr_in servaddr_t;
				    int       lens_t;
    				    char      send_t[MAXLINE];
				    int       port_t;
				    
				    memset( recv, 0, sizeof( recv ) );
				    n = read( connfd, recv, MAXLINE );
				    if( n == 0 )printf("Recv ok...\n");
				    else if( n == -1 )printf("Recv error...\n");
				    else
				    {
					lens = strlen( recv );
					recv[lens] = '\0';
					//!> 写到stdout
					port_t=atoi(recv);

				    }

				    
				    //printf("-----%s,%d-----\n",address_t,port_t);

				    if( ( connfd_t = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
				    {
					printf("client temp socket Error %d.\n",errno);
					exit( EXIT_FAILURE );
				    }

				    //!> 套接字信息
				    bzero(&servaddr_t, sizeof(servaddr_t));
				    servaddr_t.sin_family = AF_INET;
				    servaddr_t.sin_port = htons(port_t);
				    inet_pton(AF_INET, address_t, &servaddr_t.sin_addr);

				   
				    //!> 链接server
				    if( connect( connfd_t, ( struct sockaddr *  )&servaddr_t, sizeof( servaddr_t ) ) < 0 )
				    {
					printf("client temp connect error..\n");
					exit(EXIT_FAILURE);
				    }

				    memset( send_t, 0, sizeof( send_t ) );
				    while( read(fd, send_t, sizeof(send_t)) != 0 )
				    {
					write( connfd_t, send_t, strlen( send_t ) );
				    }
				    //printf("client temp end...\n");
				    strcat(og_send," suceeded");
				    printf("%s\n",og_send);

				    close(connfd_t);
    				    close(fd);
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
   
    if( argc != 3 )
    {
        printf("Input server ip !\n");
        exit( EXIT_FAILURE );
    }
   
    //!> 建立套接字
    if( ( connfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        printf("Socket Error %d.\n",errno);
        exit( EXIT_FAILURE );
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
    send_and_recv( connfd , argv[1] );

    close( connfd );
    //printf("Exit\n");
   
    return 0;
}

