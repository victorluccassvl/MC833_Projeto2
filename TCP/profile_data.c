//_________________________________________________________________________________________________

#define MAX_EMAIL_SIZE 40
#define MAX_NAME_SIZE 30
#define MAX_SURNAME_SIZE 30
#define MAX_HOME_SIZE 30
#define MAX_COLLEGE_DEGREE_SIZE 30
#define MAX_ABILITIES_SIZE 100
#define MAX_EXPERIENCES 10
#define MAX_EXPERIENCE_SIZE 50
#define MAX_PATH_SIZE 50
#define MAX_IMAGE_SIZE 8000

//_________________________________________________________________________________________________

typedef struct {

	char email         [MAX_EMAIL_SIZE];
	char name          [MAX_NAME_SIZE];
	char surname       [MAX_SURNAME_SIZE];
	char college_degree[MAX_COLLEGE_DEGREE_SIZE];
	char home          [MAX_HOME_SIZE];
	char abilities     [MAX_ABILITIES_SIZE];
	char img_path      [MAX_PATH_SIZE];
	char experience    [MAX_EXPERIENCES][MAX_EXPERIENCE_SIZE];

} Profile;

//_________________________________________________________________________________________________

FILE* Profile_open           ( const char *arq_name );
bool  Profile_add_experience ( FILE *arq, char *email, char *experience );
void  Profile_get_experiences( FILE *arq, char *email, char **experiences );
void  Profile_get_profile    ( FILE *arq, char *email, Profile *profile_ret );
void  Profile_file_to_buffer ( const char *path, unsigned char *buffer, size_t *buffer_size );
void  Profile_buffer_to_file ( const char *path, unsigned char *buffer, size_t buffer_size );
bool  Profile_read           ( FILE *arq, Profile *profile );
void  Profile_rewind         ( FILE *arq );
void  Profile_close          ( FILE *arq );
void  Profile_print          ( Profile profile );
//_________________________________________________________________________________________________


FILE* Profile_open( const char *arq_name )
{
	FILE *arq;

	arq = fopen( arq_name, "r+b" );

	if ( arq == NULL )
	{
		printf( "Falha ao abrir os dados.\n" );
		exit(1);
	}

	return arq;
}

bool Profile_add_experience( FILE *arq, char *email, char *experience )
{
	Profile profile;

	Profile_rewind( arq );

	while ( Profile_read( arq, &profile ) )
	{
		if ( strcmp( profile.email, email ) == 0 )
		{
			if ( profile.experience[MAX_EXPERIENCES - 1][0] == '\0' )
			{
				for( int i = 0 ; i < MAX_EXPERIENCES ; i++ )
				{
					if ( profile.experience[i][0] == '\0' )
					{
						strcpy( profile.experience[i], experience );
						break;
					}
				}
				fseek( arq, (-1) * sizeof( Profile ), SEEK_CUR );
				fwrite( &profile, sizeof( Profile ), 1, arq );
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

void Profile_get_experiences( FILE *arq, char *email, char **experiences )
{
	Profile profile;

	Profile_rewind( arq );

	while ( Profile_read( arq, &profile ) )
	{
		if ( strcmp( profile.email, email ) == 0 )
		{
			for( int i = 0 ; i < MAX_EXPERIENCES ; i++ )
			{
				strcpy( experiences[i], profile.experience[i] );
			}
		}
	}
}

void Profile_get_profile( FILE *arq, char *email, Profile *profile_ret )
{
	Profile profile;

	Profile_rewind( arq );

	while ( Profile_read( arq, &profile ) )
	{
		if ( strcmp( profile.email, email ) == 0 )
		{
			*profile_ret = profile;
		}
	}
}

void Profile_file_to_buffer( const char *path, unsigned char *buffer, size_t *buffer_size )
{
	strcat( ( char* ) path, ".jpg" );

	FILE *img = fopen( path, "rb" );

	if ( img == NULL )
	{
		printf( "Could not open file to buffer.\n " );
		exit( 1 );
	}

	fseek( img, 0, SEEK_END );
	*buffer_size = ftell( img );
	fseek( img, 0, SEEK_SET );

	fread( buffer, sizeof( unsigned char ), *buffer_size, img );

	fclose( img );
}

void Profile_buffer_to_file( const char *path, unsigned char *buffer, size_t buffer_size )
{
	strcat( ( char* ) path, "_downloaded.jpg" );

	FILE *img = fopen( path, "wb" );

	if ( img == NULL )
	{
		printf( "Could not open file initialize buffer.\n " );
		exit( 1 );
	}

	fwrite( buffer, sizeof( unsigned char ), buffer_size, img );

	fclose( img );
}

bool Profile_read( FILE *arq, Profile *profile )
{
	if ( fread( profile, sizeof( Profile ), 1, arq ) != 1 )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Profile_rewind( FILE *arq )
{
	fseek( arq, 0, SEEK_SET );
}

void Profile_close( FILE *arq )
{
	if ( arq != NULL )
	{
		fclose( arq );
	}
}

void Profile_print( Profile profile )
{
	printf( "         Email : %s\n", profile.email );
	printf( "          Name : %s\n", profile.name );
	printf( "       Surname : %s\n", profile.surname );
	printf( "College Degree : %s\n", profile.college_degree );
	printf( "          Home : %s\n", profile.home );
	printf( "     Abilities : %s\n", profile.abilities );
	printf( "   Experiences : " );
	for( int i = 0 ; i < MAX_EXPERIENCES ; i++ )
	{
		if ( profile.experience[i][0] != '\0' )
		{
			if ( i != 0 )
			{
				printf( "                 " );
			}
			printf( "%s\n", profile.experience[i] );
		}
	}
	if ( profile.experience[0][0] == '\0' )
	{
		printf( "\n" );
	}
}

//_________________________________________________________________________________________________