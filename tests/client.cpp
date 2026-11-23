#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAXLINE 1024

int main(int argc, char **argv)
{
	int		sockfd;
	ssize_t	n;
	char	recvline[MAXLINE + 1];
	struct	sockaddr_in	servaddr;

	if(argc != 2)
	{
		fprintf(stderr, "usage: a.out <IPaddress>\n");
		exit(EXIT_FAILURE);
	}
	//Flujo del cliente: socket -> connect-> read -> close
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(13);//htons convierte un numero de 16 bits al formato estandar de red.
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		fprintf(stderr, "inet_pton error for %s\n", argv[1]);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	if(connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connect");
		close(sockfd);
		exit(EXIT_FAILURE);
	}	
		while((n = read(sockfd, recvline, MAXLINE)) > 0)
		{
			recvline[n] = 0;
			if(fputs(recvline, stdout) == EOF)
			{
				perror("fputs");
				close(sockfd);
				exit(EXIT_FAILURE);
			}
		}
	if(n < 0)
	{
		perror("read");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	close(sockfd);
	exit(EXIT_SUCCESS);
}