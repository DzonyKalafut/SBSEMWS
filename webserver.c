/**
 * @brief SBSEMWS - Simple Bash Scripting Enabled Multithreaded Web Server
 *
 * Simple HTTP server that implements GET method for sending files over the network. Server uses threads 
 * to serve multiple clients at once. If client sends request for file ending with ".bash", 
 * server will get parameters from url(if any), execute script and sends output to the client.
 */ 

/**
 * @file webserver.c
 * @author Ján Hajnár
 * @date November 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUFFERSIZE 1024 /**< Veľkosť buffera na na čítanie/zapisovanie */
#define MAX_NUM_THREADS 10 /**< Maximálny počet súčasne bežiacich vlákien */
#define ROOT_FOLDER "www/" /**< Adresár v ktorom sú umiestnené súbory */

#define SUCC_200 "HTTP/1.1 200 OK\n\n"
#define SUCC_200_L 17 
#define ERR_400 "HTTP/1.1 400 BAD REQUEST\n\n"
#define ERR_400_L 26 
#define ERR_403 "HTTP/1.1 403 FORBIDDEN\n\n<html><head></head><body>ERROR 403: Prístup k súboru odmietnutý</body></html>"
#define ERR_403_L 103 
#define ERR_404 "HTTP/1.1 404 NOT FOUND\n\n<html><head></head><body>ERROR 404: Súbor sa nenašiel</body></html>"
#define ERR_404_L 93 

int free_threads; /**< počet voľných vlákien */
pthread_mutex_t lock_free_threads; /**< mutex slúžiaci na uzamykanie premennej free_threads */

/**
 * @brief Functions that decodes name of file that should be sent to the client
 *
 * If client sends any request other that GET,
 * ERROR 400 and an empty string is returned.If client requests "/" server will return "index.html", 
 * else it will try to return file with name decoded from the request.
 *
 * @param socket on which communication occurs
 * @param buffer array  of chars
 * @return name of file that client sent requested for
 */
char* parse_request ( int socket, char* buffer )
{
    if ( recv ( socket, buffer, BUFFERSIZE, NULL ) > 0 )
    { 
        char* req_word = strtok ( buffer, " ");
        char* file_to_send;
        if ( strncmp ( req_word, "GET", 3 ) == 0 ) 
        {
            req_word = strtok ( NULL, " " );
            if ( strcmp ( req_word, "/" ) == 0 )
            {
                file_to_send = malloc ( ( strlen ("index.html") + 1 ) * sizeof(char) );
                strcpy ( file_to_send, "index.html" );
                printf ( "Poziadavka na subor '%s'\n", "index.htm" );
            }
            else
            {
                req_word++;
                file_to_send = malloc ( ( strlen ( req_word )  + 1 ) * sizeof(char) );
                strcpy ( file_to_send, req_word );
                printf ( "Poziadavka na subor '%s'\n", file_to_send );
            }
            return file_to_send;
        }
        else
        {
            send ( socket, ERR_400, ERR_400_L, MSG_NOSIGNAL );  
            return "";
        }
    }
    else
        return "";
}

/**
 * @brief Function reads a file specified in parameter and then sends the file to socket specified in parameter
 *
 * At first function checks for "/" or ".." at the begining of requested filename(this would enable client to get file outside of our www root directory).
 * If requested file is .bash file, function will try to execute it and then send stdout to the socket
 * If requested file is not a bash script, it will be read to the buffer and sent to socket.
 * In both cases ERROR 403 is returned in case that server user do not have acces to the file and ERROR 404 
 * if file does not exist. 
 *
 * @param socket on which communication occurs
 * @param buffer array  of chars
 * @param file_to_send file that will be sent to the client
 */
void send_response ( int socket, char* buffer, char* file_to_send )
{
    if ( strncmp ( file_to_send, "/", 1 ) == 0 || strncmp ( file_to_send, "..", 2 ) == 0 )
    {
        send( socket, ERR_403, ERR_403_L, MSG_NOSIGNAL );  
        free ( file_to_send );
        return;
    }

    char* work_dir = getcwd ( NULL, 0 );
    char* full_path = malloc( ( strlen ( work_dir ) + strlen ( ROOT_FOLDER ) + strlen ( file_to_send ) + 2 ) * sizeof ( char ) );
    sprintf ( full_path, "%s/%s", work_dir, ROOT_FOLDER );        
    free ( work_dir );
    char* token = strtok ( file_to_send, "?" );                
    sprintf( full_path, "%s%s", full_path, token );        

    if ( strstr ( file_to_send, ".bash" ) != NULL)
    {
        if ( access ( full_path, X_OK)  == 0 )
        {
            char* parameters[10];
            int i = 1;
            parameters[0] = full_path;
            token = strtok ( NULL , "?" );                
            while ( token != NULL )
            {
                parameters[i] = token;                
                token = strtok ( NULL, "?" );
                i++;
            }
            parameters[i] = NULL;
            send( socket, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n", 41, MSG_NOSIGNAL );
            int status;
            pid_t cpid = fork();

            if (cpid == 0) 
            {
            dup2(socket, STDOUT_FILENO);
            dup2(socket, STDERR_FILENO);
            execv( full_path, parameters );
            }
            else
            {
                wait(&status);
            }
        }
        else if ( errno == EACCES )
        {
            send( socket, ERR_403, ERR_403_L, MSG_NOSIGNAL );
            printf ( "Subor '%s' nieje spustitelny\n", full_path );  
        }
        else if ( errno == ENOENT )
        {
            send( socket, ERR_404, ERR_404_L, MSG_NOSIGNAL );
            printf ( "Subor '%s' sa nenasiel\n", full_path );  
        }

    }
    else
    {
        int file = open ( full_path , O_RDONLY );
        if ( file > 0 )
        {
            send( socket, SUCC_200, SUCC_200_L, MSG_NOSIGNAL );  
            printf ( "Vraciam subor '%s'\n", full_path );
            int num_of_bytes;
            while ( ( num_of_bytes = read ( file, buffer, BUFFERSIZE ) ) > 0 )
            {
                if ( send( socket, buffer, num_of_bytes, MSG_NOSIGNAL ) < 0 )
                   break; 
                bzero ( buffer, BUFFERSIZE );
            }
            close ( file );
        }
        else if ( errno == EACCES )
        {
            send( socket, ERR_403, ERR_403_L, MSG_NOSIGNAL );
            printf ( "Pristup k suboru '%s' odmietnuty\n", full_path );  
        }
        else if ( errno == ENOENT )
        {
            send( socket, ERR_404, ERR_404_L, MSG_NOSIGNAL );
            printf ( "Subor '%s' sa nenasiel\n", full_path );  
        }
    }
    free ( file_to_send );
    free ( full_path );
    return;
}

/**
 * @brief Function that is executed by each thread
 *
 * Function first create buffer of size = BUFFERSIZE, then function
 * parse_request() tries to find out which file to send. Filename is sent to 
 * function send_response() which sends the file to the socket. 
 *
 * @param args socket on which communication occurs
 */
void* server_thread_func ( void* args )
{
    int client_socket = ( int ) args; //bla bla
    char buffer[BUFFERSIZE]; 

    bzero ( buffer, BUFFERSIZE );
    char* file_to_send = parse_request (client_socket, buffer );

    bzero ( buffer, BUFFERSIZE );
    
    if ( file_to_send != "" )
        send_response ( client_socket, buffer, file_to_send );  

    close( client_socket );
    
    pthread_mutex_lock ( &lock_free_threads );
    free_threads++; 
    pthread_mutex_unlock ( &lock_free_threads );

    pthread_exit ( NULL );
}

/**
 * @brief Function that terminates server process when SIGTERM or SIGKILL is catched
 *
 * @param signal number
 */
void terminate ( int a )
{
    printf ( "Zacyteny signal na ukoncenie programu\n" );
    printf ( "Ukoncujem program....\n" );
    exit ( EXIT_SUCCESS );
}

int main( int argc, char *argv[] )
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    free_threads = MAX_NUM_THREADS;
    pthread_mutex_init ( &lock_free_threads, NULL );

    signal ( SIGKILL, terminate );
    signal ( SIGTERM, terminate );

    if ( argc < 2 ) 
    {
        fprintf( stderr,"CHYBA, nezadal si port\n" );
        exit ( EXIT_FAILURE );
    }

    sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 )
        perror ( "CYBA, socket sa neda otvorit" );
    bzero ( (char *) &serv_addr, sizeof ( serv_addr ) );
    portno = atoi ( argv[1] );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if ( bind ( sockfd, (struct sockaddr *) &serv_addr, sizeof ( serv_addr ) ) < 0) 
    {
        perror( "CHYBA, binding zlyhal" );
        exit ( EXIT_FAILURE );
    }

    listen ( sockfd, MAX_NUM_THREADS );
    clilen = sizeof ( cli_addr );

    while ( 1 )
    {
        pthread_t vlakno;

        newsockfd = accept ( sockfd, (struct sockaddr *) &cli_addr, &clilen );
        if ( newsockfd < 0 ) 
        {
            perror("CHYBA, pripojenie sa nepodarilo akceptovat");
            exit( EXIT_FAILURE );
        }
        else
        {
            if ( free_threads > 0 )
            {
                pthread_mutex_lock ( &lock_free_threads );
                free_threads--;
                pthread_mutex_unlock ( &lock_free_threads );
                pthread_create ( &vlakno, NULL, server_thread_func, (void*) newsockfd );
                pthread_detach ( vlakno );
            }
        }

    }
    close( sockfd );
    return EXIT_SUCCESS;
}
