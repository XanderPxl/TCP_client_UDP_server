#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <stdbool.h> // for boolean values
	#include <time.h> // for the random number generator
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <stdbool.h> // for boolean values
	#include <time.h> // for the random number generator
	int OSInit( void ) {}
	int OSCleanup( void ) {}
#endif

int UDPinitialization();
void execution( int internet_socket );
void sendRandomNumbers(int number_of_bytes_send, int internet_socket, struct sockaddr_storage client_internet_address, int client_internet_address_length);
int receiveGO( char *buffer, bool *go, int internet_socket, struct sockaddr_storage client_internet_address, int client_internet_address_length);
void receiveHighestNumber(char *buffer, int internet_socket, int number_of_bytes_received, struct sockaddr_storage client_internet_address, int client_internet_address_length);
void sendOK(int internet_socket, int number_of_bytes_send, struct sockaddr_storage client_internet_address, int client_internet_address_length);
void cleanup( int internet_socket );

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	int internet_socket = UDPinitialization();
	srand(time(NULL));
	/////////////
	//Execution//
	/////////////

	execution( internet_socket );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int UDPinitialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				close( internet_socket );
				perror( "bind" );
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

void execution( int internet_socket )
{
	//Step 2.1
	int number_of_bytes_received = 0;
	int number_of_bytes_send = 0;
	int highest_number = 0;
	char buffer[1000];
	bool go;
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
	
	if( number_of_bytes_received == -1 )
	{
		perror( "recvfrom" );
	}
	
	else
	{
		buffer[number_of_bytes_received] = '\0';
		receiveGO(buffer, &go, internet_socket, client_internet_address, client_internet_address_length);
		if (go == true)
		{
			sendRandomNumbers(number_of_bytes_send, internet_socket, client_internet_address, client_internet_address_length);
			receiveHighestNumber( buffer, internet_socket, number_of_bytes_received, client_internet_address, client_internet_address_length);
			sendRandomNumbers(number_of_bytes_send, internet_socket, client_internet_address, client_internet_address_length); 
			receiveHighestNumber( buffer, internet_socket, number_of_bytes_received, client_internet_address, client_internet_address_length);
			sendOK(internet_socket, number_of_bytes_send, client_internet_address, client_internet_address_length);
		}
		
		
	}


	
}




int receiveGO(char *buffer, bool *go, int internet_socket, struct sockaddr_storage client_internet_address, int client_internet_address_length)
{
	if (strcmp(buffer, "GO") == 0)
		{
			printf("yes!!!!");
			printf( "Received : %s\n", buffer );
			*go = true;
		}
	else
		{
			execution(internet_socket);
		}
	
}

void sendRandomNumbers(int number_of_bytes_send, int internet_socket, struct sockaddr_storage client_internet_address, int client_internet_address_length)
{
	//Step 2.2
	
	int random_number = 0;
	int highestNumber = 0;
	char random_number_buffer[20];
	for (int i = 0; i < 40; i++)
		{
			random_number = rand() % 40 + 1;
			if (highestNumber < random_number)
			{
				highestNumber = random_number;
			}
		
			printf("RandomNumber_%d = %d\n", i, random_number);
			//*random_number_buffer = random_number;
			htonl(random_number);
			itoa(random_number, random_number_buffer, 10);
			number_of_bytes_send = sendto( internet_socket, random_number_buffer, 2, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
			if( number_of_bytes_send == -1 )
			{
				perror( "sendto" );
			}
		}
	printf("The highest number send = %d\n", highestNumber);
	
}


void receiveHighestNumber(char *buffer, int internet_socket, int number_of_bytes_received, struct sockaddr_storage client_internet_address, int client_internet_address_length)
{
	int random_number = rand() % 5 + 1;
	for (int i = 0; i < random_number; i++)
	{
		number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
	
		if( number_of_bytes_received == -1 )
		{
			perror( "recvfrom" );
		}
		
		else
		{
			buffer[number_of_bytes_received] = '\0';
			printf( "Received highest number = %s\n", buffer );
			
		}
	}
	
}

void sendOK(int internet_socket, int number_of_bytes_send, struct sockaddr_storage client_internet_address, int client_internet_address_length)
{
	
	number_of_bytes_send = sendto( internet_socket, "OK", 2, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
	if( number_of_bytes_send == -1 )
	{
		perror( "sendto" );
	}
	else
	{
		printf("OK was sended successfully\n"); 
	}

}

void cleanup( int internet_socket )
{
	//Step 3.1
	close( internet_socket );
}
