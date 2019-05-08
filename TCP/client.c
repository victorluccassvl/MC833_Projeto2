//_________________________________________________________________________________________________

#include "network_structures.c"

//_________________________________________________________________________________________________

Socket     TCP_socket    ();
void       TCP_bind      ( Socket *sock );
Connection TCP_connection( Socket *sock, const char* IP, unsigned short int port );
AnswerMsg* TCP_receive   ( Connection connection );
void       TCP_send      ( Connection connection, RequestMsg *msg );
void       TCP_close     ( Connection connection );

//_________________________________________________________________________________________________

suseconds_t Client_getTime();

void Client_operation( Connection connection, char *email );

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
	Socket client = TCP_socket();

	Connection connection = TCP_connection( &client, SERVER_IP, SERVER_PORT );

	client_average = 0;
	server_average = 0;
	time_it = 0;

	char email[MAX_EMAIL_SIZE];
	printf( "\nEmail: " );
	scanf("%s", email );

	for( int i = 0 ; i < TIME_TEST_SAMPLE_SIZE ; i++ )
	{
		Client_operation( connection, email );
	}

	TCP_close( connection );

	{
		client_average /= TIME_TEST_SAMPLE_SIZE;
		server_average /= TIME_TEST_SAMPLE_SIZE;

		suseconds_t communication_time_average = 0;

		printf( "Tempo por cada iteração\n\n" );

		printf(" Iter ClientTime(µs)  ServerTime(µs) ComunnicationTime(µs)\n");
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

Socket TCP_socket()
{
	Socket sock;

	sock.descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	if ( sock.descriptor < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	TCP_bind( &sock );

	return sock;
}

void TCP_bind( Socket *sock )
{
	char host_name[HOST_NAME_MAX];

	if ( gethostname( host_name, sizeof( char ) * HOST_NAME_MAX ) < 0 )
	{
		printf( "Could not retrieve hostname.\n" );
		exit( 1 );
	}

	struct hostent *host_entry = gethostbyname( host_name );

	sock->address.sin_family = AF_INET;
	sock->address.sin_port   = 0;
	sock->address.sin_addr   = *( ( struct in_addr* ) host_entry->h_addr_list[0] );

	bind( sock->descriptor, ( struct sockaddr* ) &( sock->address ), sizeof( sock->address ) );

	socklen_t addr_len = sizeof( sock->address );

	if ( getsockname( sock->descriptor, ( struct sockaddr* ) &sock->address , &addr_len ) < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}
}

Connection TCP_connection( Socket *sock, const char* IP, unsigned short int port )
{
	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	server_address.sin_port   = htons( port );
	inet_pton( AF_INET, IP, &server_address.sin_addr.s_addr );

	Connection connection;

	connection.descriptor = sock->descriptor;
	connection.client_address = sock->address;
	connection.server_address = server_address;

	if ( connect( connection.descriptor, ( struct sockaddr* ) &server_address, sizeof( server_address ) ) != 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	return connection;
}

AnswerMsg* TCP_receive( Connection connection )
{
	AnswerMsg *msg = malloc( sizeof( AnswerMsg ) );

	while ( recv( connection.descriptor, msg, sizeof( AnswerMsg ), MSG_PEEK ) != sizeof( AnswerMsg ) );
	recv( connection.descriptor, msg, sizeof( AnswerMsg ), 0 );
	end = Client_getTime();

	return msg;
}

void TCP_send( Connection connection, RequestMsg *msg )
{
	start = Client_getTime();
	send( connection.descriptor, msg, sizeof( RequestMsg ), 0 );
}

void TCP_close( Connection connection )
{
	if ( shutdown( connection.descriptor, SHUT_RDWR ) < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}
}

//_________________________________________________________________________________________________

suseconds_t Client_getTime()
{
	struct timeval time;

	gettimeofday( &time, NULL );

	return time.tv_usec + time.tv_sec * 1000000L;
}

void Client_operation( Connection connection, char *email )
{
	RequestMsg *request = malloc( sizeof( RequestMsg ) );

	strcpy( request->email, email );

	TCP_send( connection, request );
	AnswerMsg *answer = TCP_receive( connection );

	{
		client_time[time_it] = end - start;
		server_time[time_it] = answer->server_time;
		client_average += client_time[time_it];
		server_average += server_time[time_it];
		time_it++;
	}

	Profile_buffer_to_file( answer->img_path, answer->img, answer->img_size );
	printf( "NAME: %s\n", answer->name );

	free( request );
	free( answer );
}

//_________________________________________________________________________________________________