/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ncurses.h>

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

struct __attribute__((__packed__)) command
{
  float translational;
  float angle;
  int mode;
} input_command = {0.0, 0.0, 0.0};

struct __attribute__((__packed__)) robot_info
{
  double odo[3];
  double imu[6];
  double head;
} cur_info = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0, 7.0, 8.0, 9.0}, 10.0};

void send_command(struct command c, int sockfd, struct sockaddr_in serveraddr);
struct robot_info receive_info(int sockfd, struct sockaddr_in serveraddr);

int main(int argc, char **argv)
{
	int send_sockfd, receive_sockfd, send_port = 5005, receive_port = 4242, n;
	int serverlen;
	struct sockaddr_in send_serveraddr, receive_serveraddr;
	struct hostent *send_server, *receive_server;
	char *hostname;
	int key;

	initscr();
	keypad(stdscr, TRUE);
	noecho();
	scrollok(stdscr, TRUE);
	timeout(-1);

	/* check command line arguments */
	if (argc != 2) {
		fprintf(stderr,"usage: %s <hostname>\n", argv[0]);
		exit(0);
	}
	hostname = argv[1];

	/* socket: create the socket */
	send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (send_sockfd < 0) 
		error("ERROR opening socket");
	
	/* socket: create the socket */
	receive_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (receive_sockfd < 0) 
		error("ERROR opening socket");

	/* gethostbyname: get the server's DNS entry */
	send_server = gethostbyname(hostname);
	if (send_server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}
	
	/* gethostbyname: get the server's DNS entry */
	receive_server = gethostbyname(hostname);
	if (receive_server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}

	/* build the server's Internet address */
	bzero((char *) &send_serveraddr, sizeof(send_serveraddr));
	send_serveraddr.sin_family = AF_INET;
	bcopy((char *)send_server->h_addr,
			(char *)&send_serveraddr.sin_addr.s_addr, send_server->h_length);
	send_serveraddr.sin_port = htons(send_port);

	/* build the server's Internet address */
	bzero((char *) &receive_serveraddr, sizeof(receive_serveraddr));
	receive_serveraddr.sin_family = AF_INET;
	bcopy((char *)receive_server->h_addr,
			(char *)&receive_serveraddr.sin_addr.s_addr, receive_server->h_length);
	receive_serveraddr.sin_port = htons(receive_port);
	
	while (1)
	{
		key = getch();
		switch (key)
		{
		case KEY_UP:
			if (input_command.translational == 0.0f)
				input_command.translational = 100.0f;
			else
				input_command.translational += 50.0f;
			printw("%f\n", input_command.translational);
			break;
		case KEY_DOWN:
			if (input_command.translational == 100.0f)
				input_command.translational = 0.0f;
			else
				input_command.translational -= 50.0f;
			printw("%f\n", input_command.translational);
			break;
		case KEY_LEFT:
			input_command.mode = 0;
			input_command.angle -= 15.0f;
			break;
		case KEY_RIGHT:
			input_command.mode = 0;
			input_command.angle += 15.0f;
			break;
		case 113:
			printw("Robot Info: %f, %f, %f\n", cur_info.odo[0], cur_info.imu[0], cur_info.head);
			break;
		case 119:
			input_command.mode = 1;
			input_command.angle += 0.0f;
			break;
		case 97:
			input_command.mode = 2;
			input_command.angle += 0.0f;
			break;
		case 115:
			input_command.mode = 3;
			input_command.angle += 0.0f;
			break;
		case 100:
			input_command.mode = 4;
			input_command.angle += 0.0f;
		default:
			break;
		}
		
		send_command(input_command, send_sockfd, send_serveraddr);
		//cur_info = receive_info(send_sockfd, send_serveraddr);
		//printw("Robot Info: %f, %f, %f\n", cur_info.odo[0], cur_info.imu[0], cur_info.head);
		refresh();
	}

	endwin();
    return 0;
}

void send_command(struct command c, int sockfd, struct sockaddr_in serveraddr)
{
	int serverlen = sizeof(serveraddr);
	/* send the message to the server */
	if (sendto(sockfd, &c, sizeof(c), 0, &serveraddr, serverlen) < 0)
		error("ERROR in sendto");
}

struct robot_info receive_info(int sockfd, struct sockaddr_in serveraddr)
{
	struct robot_info current;
	int serverlen = sizeof(serveraddr);
	
	/* print the server's reply */
	if (recvfrom(sockfd, (char*)(&current), sizeof(current), 0, &serveraddr, &serverlen) < 0)
		error("ERROR in recvfrom");

	return current;
}
