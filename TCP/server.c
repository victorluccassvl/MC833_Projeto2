//_________________________________________________________________________________________________

#include "network_structures.c"

#define MAX_INCOMING_CONNECTIONS 10

//_________________________________________________________________________________________________

Socket      TCP_socket ( const char* IP, unsigned short int port );
void        TCP_bind   ( Socket *socket, const char* IP, unsigned short int port );
void        TCP_listen ( Socket socket );
Connection  TCP_accept ( Socket socket );
RequestMsg* TCP_receive( Connection connection, suseconds_t *start );
void        TCP_send   ( Connection connection, AnswerMsg *msg, suseconds_t start );
void        TCP_close  ( Connection connection );

//_________________________________________________________________________________________________

suseconds_t Server_getTime();

void Server_operation( Connection connection );

//_________________________________________________________________________________________________

FILE *database;
pthread_mutex_t database_lock;
void *answer_client( void *connection_arg );

//_________________________________________________________________________________________________

int main()
{

	Socket server =  TCP_socket( SERVER_IP, SERVER_PORT );

	TCP_listen( server );

	database = Profile_open( "database.bin" );

	if( pthread_mutex_init( &database_lock, NULL ) != 0 )
    {
        printf( "Falha ao criar o lock do database.\n" );
        return 1;
    }

	while ( true )
	{
		Connection connection = TCP_accept( server );

		pthread_t thread_id;

		sleep( 2 ); // Unsafe way to assert the new born thread will access right connection struct

	    if ( pthread_create( &thread_id, NULL, &answer_client, ( void* ) &connection ) != 0 )
	    {
	        printf( "Falha ao criar a thread filha.\n" );
	        return 1;
	    }
	}

	return 0;
}

//_________________________________________________________________________________________________

void *answer_client( void *connection_arg )
{
	Connection connection = *( ( Connection* ) connection_arg );

	for( int i = 0 ; i < TIME_TEST_SAMPLE_SIZE ; i++ )
	{
		Server_operation( connection );
	}

	TCP_close( connection );
}

//_________________________________________________________________________________________________

Socket TCP_socket( const char* IP, unsigned short int port )
{
	Socket sock;

	sock.descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	if ( sock.descriptor < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	TCP_bind( &sock, IP, port );

	return sock;
}

void TCP_bind( Socket *sock, const char* IP, unsigned short int port )
{
	sock->address.sin_family = AF_INET;
	sock->address.sin_port   = htons( port );
	inet_pton( AF_INET, IP, &( sock->address.sin_addr ) );

	if ( bind( sock->descriptor, ( struct sockaddr* ) &( sock->address ), sizeof( sock->address ) ) < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}
}

void TCP_listen( Socket sock )
{
	if ( listen( sock.descriptor, MAX_INCOMING_CONNECTIONS ) < 0 )
	{
		perror( "Erro : " );
		exit( 0 );
	}
}

Connection TCP_accept( Socket sock )
{
	struct sockaddr_in client_address;
	socklen_t address_size;

	address_size = sizeof( client_address );

	Connection connection;

	connection.descriptor = accept( sock.descriptor, ( struct sockaddr * ) &client_address, &address_size );

	if ( connection.descriptor < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	connection.server_address = sock.address;
	connection.client_address = client_address;

	return connection;
}

RequestMsg* TCP_receive( Connection connection, suseconds_t *start )
{
	RequestMsg *msg = malloc( sizeof( RequestMsg ) );

	while( recv( connection.descriptor, msg, sizeof( RequestMsg ), MSG_PEEK ) != sizeof( RequestMsg ) );

	recv( connection.descriptor, msg, sizeof( RequestMsg ), 0 );

	*start = Server_getTime();

	return msg;
}

void TCP_send( Connection connection, AnswerMsg *msg, suseconds_t start )
{
	suseconds_t end = Server_getTime();

	msg->server_time = end - start;

	send( connection.descriptor, msg, sizeof( AnswerMsg ), 0 );
}

void TCP_close( Connection connection )
{

	if ( shutdown( connection.descriptor, SHUT_RDWR ) < 0 )
	{
		perror( "Erro : " );
		exit( 0 );
	}
}

//_________________________________________________________________________________________________

suseconds_t Server_getTime()
{
	struct timeval time;

	gettimeofday( &time, NULL );

	return time.tv_usec + time.tv_sec * 1000000L;
}

void Server_operation( Connection connection )
{
	suseconds_t start;
	
	RequestMsg *request = TCP_receive( connection, &start );

    pthread_mutex_lock( &database_lock );

	AnswerMsg *answer = malloc( sizeof( AnswerMsg ) );

	Profile profile;

	Profile_get_profile( database, request->email, &profile );

	Profile_file_to_buffer( profile.img_path, answer->img, &( answer->img_size ) );

	strcpy( answer->name, profile.name );
	strcat( answer->name, "_" );
	strcat( answer->name, profile.surname );

	strcpy( answer->img_path, profile.img_path );

	TCP_send( connection, answer, start );

	free( answer );
	free( request );

	pthread_mutex_unlock( &database_lock );
}

//_________________________________________________________________________________________________