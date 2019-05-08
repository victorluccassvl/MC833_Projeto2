//_________________________________________________________________________________________________

#include "network_structures.c"

//tumbleupon.com

//_________________________________________________________________________________________________

Socket      UDP_socket ( const char* IP, unsigned short int port );
void        UDP_bind   ( Socket *sock, const char* IP, unsigned short int port );
RequestMsg* UDP_receive( Socket sock, struct sockaddr_in *client_address );
void        UDP_send( Socket sock, AnswerMsg *msg, struct sockaddr_in *client_address );

//_________________________________________________________________________________________________

suseconds_t Server_getTime();

void Server_operation( Socket socket );

//_________________________________________________________________________________________________

FILE *database;
suseconds_t start, end;

int main()
{
	Socket server = UDP_socket( SERVER_IP, SERVER_PORT );

	database = Profile_open( "database.bin" );

	while ( true )
	{
		Server_operation( server );
	}

	return 0;
}

//_________________________________________________________________________________________________

Socket UDP_socket( const char* IP, unsigned short int port )
{
	Socket sock;

	sock.descriptor = socket( AF_INET, SOCK_DGRAM, 0 );

	if ( sock.descriptor < 0 )
	{
		perror( "Erro : " );
		exit( 1 );
	}

	UDP_bind( &sock, IP, port );

	return sock;
}

void UDP_bind( Socket *sock, const char* IP, unsigned short int port )
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

RequestMsg* UDP_receive( Socket sock, struct sockaddr_in *client_address )
{
	RequestMsg* msg = malloc( sizeof( RequestMsg ) );

	socklen_t address_len;

	while( recvfrom( sock.descriptor, msg, sizeof( RequestMsg ), MSG_PEEK, ( struct sockaddr* ) client_address, &address_len ) != sizeof( RequestMsg ) );
	recvfrom( sock.descriptor, msg, sizeof( RequestMsg ), MSG_PEEK, ( struct sockaddr* ) client_address, &address_len );
	start = Server_getTime();

	return msg;
}

void UDP_send( Socket sock, AnswerMsg *msg, struct sockaddr_in *client_address )
{
	end = Server_getTime();
	msg->server_time = end - start;

	printf("%lu", end - start );
	sendto( sock.descriptor, msg, sizeof( AnswerMsg ), 0, ( struct sockaddr* ) client_address, sizeof( struct sockaddr_in ) );
}

//_________________________________________________________________________________________________

suseconds_t Server_getTime()
{
	struct timeval time;

	gettimeofday( &time, NULL );

	return time.tv_usec + time.tv_sec * 1000000L;
}

void Server_operation( Socket socket )
{
	struct sockaddr_in client_address;

	RequestMsg *request = UDP_receive( socket, &client_address );

	AnswerMsg *answer = malloc( sizeof( AnswerMsg ) );

	Profile_rewind( database );

	Profile profile;

	Profile_get_profile( database, request->email, &profile );

	Profile_file_to_buffer( profile.img_path, answer->img, &( answer->img_size ) );

	strcpy( answer->name, profile.name );
	strcat( answer->name, "_" );
	strcat( answer->name, profile.surname );

	strcpy( answer->img_path, profile.img_path );

	UDP_send( socket, answer, &client_address );

	free( answer );
	free( request );
}

//_________________________________________________________________________________________________