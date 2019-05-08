//_________________________________________________________________________________________________

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "profile_data.c"

#define SERVER_PORT           20002
#define SERVER_IP             "177.220.85.57"
#define HOST_NAME_MAX         200
#define TIME_TEST_SAMPLE_SIZE 101

//_________________________________________________________________________________________________

typedef struct
{
	int descriptor;
	struct sockaddr_in address;

} Socket;

typedef struct
{
	int descriptor;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

} Connection;

//_________________________________________________________________________________________________

typedef struct
{
	char name[MAX_NAME_SIZE + MAX_SURNAME_SIZE + 1];
	char img_path[MAX_PATH_SIZE];
	unsigned char img[MAX_IMAGE_SIZE];
	size_t img_size;
	suseconds_t server_time;

} AnswerMsg;

//_________________________________________________________________________________________________

typedef struct
{
	char email[MAX_EMAIL_SIZE];

} RequestMsg;

//_________________________________________________________________________________________________
