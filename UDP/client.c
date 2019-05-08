//_________________________________________________________________________________________________

#include "network_structures.c"

//_________________________________________________________________________________________________

Socket     UDP_socket ();
AnswerMsg* UDP_receive( Socket socket );
void       UDP_send   ( Socket socket, RequestMsg *msg );
void       UDP_close  ( Socket socket );

//_________________________________________________________________________________________________

suseconds_t Client_getTime();

void Client_operation( Socket socket, char *email );

//_________________________________________________________________________________________________

suseconds_t server_time[TIME_TEST_SAMPLE_SIZE];
suseconds_t server_average;

suseconds_t client_time[TIME_TEST_SAMPLE_SIZE];
suseconds_t client_average;

suseconds_t start, end;

int time_it;

//_________________________________________________________________________________________________

int main()
{
	Socket client = UDP_socket();

	char email[MAX_EMAIL_SIZE];
	printf( "\nEmail: " );
	scanf( "%s", email );

	server_average = 0;
	client_average = 0;
	time_it = 0;

	for( int i = 0 ; i < TIME_TEST_SAMPLE_SIZE ; i++ )
	{
		server_time[i] = 0;
		client_time[i] = 0;
	}

	for( int i = 0 ; i < TIME_TEST_SAMPLE_SIZE ; i++ )
	{
		Client_operation( client, email );
	}

	{
		client_average /= TIME_TEST_SAMPLE_SIZE;
		server_average /= TIME_TEST_SAMPLE_SIZE;

		suseconds_t communication_time_average = 0;

		printf( "\n\nTempo por cada iteração\n\n" );

		printf(" Iter ClientTime(µs)  ServerTime(µs) CommunicationTime(µs)\n");

		for( int i = 0 ; i < TIME_TEST_SAMPLE_SIZE ; i++ )
		{
			printf("(%3d)   %9lu       %9lu          %9lu\n", i+1, client_time[i], server_time[i], client_time[i] - server_time[i] );
			communication_time_average += client_time[i] - server_time[i];
		}

		communication_time_average /= TIME_TEST_SAMPLE_SIZE;

		printf("\n        %9lu       %9lu          %9lu  <-- Média\n", client_average, server_average, communication_time_average );

	}

	return 0;
}

//_________________________________________________________________________________________________

Socket UDP_socket()
{
	Socket sock;

	sock.descriptor = socket( AF_INET, SOCK_DGRAM, 0 );

	if ( sock.descriptor < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	sock.address.sin_family = AF_INET;
	sock.address.sin_port   = htons( SERVER_PORT );
	inet_pton( AF_INET, SERVER_IP, &( sock.address.sin_addr ) );

	return sock;
}

AnswerMsg* UDP_receive( Socket sock )
{
	AnswerMsg *msg = malloc( sizeof( AnswerMsg ) );
	socklen_t address_len;
	struct sockaddr_in address;

	suseconds_t timeout = Client_getTime();
	size_t msg_size;

	do
	{
		do
		{
			if ( Client_getTime() - timeout >= 10000L )
			{
				free( msg );
				return NULL;
			}

			msg_size = recvfrom( sock.descriptor, msg, sizeof( AnswerMsg ), MSG_PEEK, ( struct sockaddr* ) &( address ), &address_len );
		}
		while( msg_size != sizeof( AnswerMsg ) );

		recvfrom( sock.descriptor, msg, sizeof( AnswerMsg ), 0, ( struct sockaddr* ) &( address ), &address_len );

	}
	while( msg->iteration != time_it );

	end = Client_getTime();

	return msg;
}

void UDP_send( Socket sock, RequestMsg *msg )
{
	start = Client_getTime();
	sendto( sock.descriptor, msg, sizeof( RequestMsg ), 0, ( struct sockaddr* ) &( sock.address ), sizeof( sock.address ) );
}

//_________________________________________________________________________________________________

suseconds_t Client_getTime()
{
	struct timeval time;

	gettimeofday( &time, NULL );

	return time.tv_usec + time.tv_sec * 1000000L;
}

void Client_operation( Socket socket, char *email )
{
	RequestMsg *request = malloc( sizeof( RequestMsg ) );

	strcpy( request->email, email );
	request->iteration = time_it;

	UDP_send( socket, request );
	AnswerMsg *answer = UDP_receive( socket );

	if ( answer != NULL )
	{
		//Profile_buffer_to_file( answer->img_path, answer->img, answer->img_size );

		server_time[time_it] = answer->server_time;
		client_time[time_it] =  end - start;

		client_average += client_time[time_it];
		server_average += server_time[time_it];

		free( answer );
	}

	time_it++;

	free( request );
}

//_________________________________________________________________________________________________