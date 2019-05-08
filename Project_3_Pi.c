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
  float rotational;
  int mode;
} input_command;

struct __attribute__((__packed__)) robot_info
{
  double odo[3];
  double imu[6];
  double head;
} cur_info = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 0.0};

void send_command(struct command c, int sockfd, struct sockaddr_in serveraddr);
struct robot_info receive_info(int sockfd, struct sockaddr_in serveraddr);

int main(int argc, char **argv)
{
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    int key;

	initscr();
	keypad(stdscr, TRUE);
	noecho();
	scrollok(stdscr, TRUE);
	timeout(-1);

	while(1)
		printw("1\n");

	/* check command line arguments */
	if (argc != 3) {
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");


	printw("2\n");	

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    printw("1!\n");

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

	
	while (1)
	{
		//key = getch();
		switch (key)
		{
		case KEY_UP:
			if (input_command.translational == 0.0f)
				input_command.translational = 700.0f;
			else
				input_command.translational += 50.0f;
			break;
		case KEY_DOWN:
			if (input_command.translational == 700.0f)
				input_command.translational = 0.0f;
			else
				input_command.translational -= 50.0f;
			break;
		case KEY_LEFT:
			input_command.rotational -= 10.0f;
			break;
		case KEY_RIGHT:
			input_command.rotational += 10.0f;
			break;
		case 113:
			printw("Robot Info: %f, %f, %f\n", cur_info.odo[0], cur_info.imu[0], cur_info.head);
			break;
		default:
			break;
		}
		
		printw("Waiting...\n");
		//send_command(input_command, sockfd, serveraddr);
		cur_info = receive_info(sockfd, serveraddr); 
		printw("Robot Info: %f, %f, %f\n", cur_info.odo[0], cur_info.imu[0], cur_info.head);
	}

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
