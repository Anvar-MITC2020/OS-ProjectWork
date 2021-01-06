#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
  
#define BUFSIZE 1024
char *bus_num={'\0'};
char *card_num={'\0'};

void f(const char* s, char* res) {
    char* l = strchr(s, ' ');
    char* r = strchr(l + 1, ' ');
    memcpy(res, l + 1, r - l);
    res[r - l - 1] = '\0';
}


void send_recv(int i, int sockfd)
{
  char send_buf[BUFSIZE];
  char recv_buf[BUFSIZE];
  int nbyte_recvd;
  
  if (i == 0){

    fgets(send_buf, BUFSIZE, stdin);
	//send_buf=bus_num;
    if (strcmp(send_buf , "quit\n") == 0) {
      exit(0);
    }else
      send(sockfd, send_buf, strlen(send_buf), 0);
  }else {
    nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0);
    recv_buf[nbyte_recvd] = '\0';
    printf("%s\n" , recv_buf);
    fflush(stdout);
  }
}
    
    
void connect_request(int *sockfd, struct sockaddr_in *server_addr)
{
  if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(8888);
  server_addr->sin_addr.s_addr = inet_addr("192.168.43.40");
  memset(server_addr->sin_zero, '0', sizeof server_addr->sin_zero);
  
  if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {
    perror("connect");
    exit(1);
  }
}
  
int main(int argc, char *argv[])
{
  bus_num=argv[1];
  card_num=argv[2];


    

  int sockfd, fdmax, i;
  struct sockaddr_in server_addr;
  fd_set master;
  fd_set read_fds;
 
  connect_request(&sockfd, &server_addr);
  FD_ZERO(&master);
        FD_ZERO(&read_fds);
        FD_SET(0, &master);
        FD_SET(sockfd, &master);
  fdmax = sockfd;
  send(sockfd, argv[1], strlen(argv[1]), 0);
  send(sockfd, argv[2], strlen(argv[2]), 0);
 /* while(1){
    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      exit(4);
    }
    printf("%s\n", argv[1]);
   for(i=0; i <= fdmax; i++ ){
	printf("%s", "ye zdes");
     if(FD_ISSET(i, &read_fds)){
       // send_recv(i, sockfd);
	  char send_buf[BUFSIZE];
	  char recv_buf[BUFSIZE];
	  int nbyte_recvd;
	 // send(sockfd, argv[1], strlen(argv[1]), 0);
	  if (i == 0){

	    //fgets(send_buf, BUFSIZE, stdin);
		//send_buf=bus_num;
	    if (strcmp(argv[1] , "quit\n") == 0) {
	      exit(0);
	    }else
	      send(sockfd, argv[1], strlen(argv[1]), 0);
	  }else {
	    nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0);
	    recv_buf[nbyte_recvd] = '\0';
	    printf("%s\n" , recv_buf);
	    fflush(stdout);
	  }
		
	}
    }
  }*/
  printf("client-quited\n");
  close(sockfd);
  return 0;
}
