/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  *  Servidor de prueba para el protocolo propio "ECCIP/1.0"
  *  (usado para simular la interaccion cliente-servidor pedida en el TC#01)
  *
  *  Nota: el "bind/listen/accept" todavia no forma parte de la clase
  *  VSocket/Socket (esa es la segunda entrega), por eso este servidor
  *  usa las llamadas Unix directamente. El cliente si usa la clase
  *  Socket ya completada (ver client.cc).
  *
  *  Un hilo POSIX atiende cada conexion aceptada.
  *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG   10
#define BUFSZ     512

/**
  * Lee una linea terminada en "\n" del socket, sin pasarse del buffer.
  * Devuelve la cantidad de bytes leidos (0 si el cliente cerro).
 **/
static ssize_t readLine( int fd, char * buf, size_t maxlen ) {

   size_t total = 0;
   char c;
   ssize_t st;

   while ( total < maxlen - 1 ) {
      st = read( fd, &c, 1 );
      if ( st <= 0 ) {
         break;                    // error o el otro lado cerro
      }
      if ( '\n' == c ) {
         break;
      }
      if ( '\r' != c ) {
         buf[total++] = c;
      }
   }

   buf[total] = '\0';
   return (ssize_t) total;

}

/**
  * Atiende una sesion completa del protocolo ECCIP/1.0 sobre "fd".
 **/
static void handleSession( int fd ) {

   char line[BUFSZ];
   char reply[BUFSZ];
   char clientName[BUFSZ] = "anonimo";

   for ( ;; ) {

      ssize_t n = readLine( fd, line, BUFSZ );
      if ( n <= 0 ) {
         break;                    // conexion cerrada por el cliente
      }

      if ( 0 == strncmp( line, "HELLO ", 6 ) ) {
         strncpy( clientName, line + 6, BUFSZ - 1 );
         snprintf( reply, BUFSZ, "OK HELLO %s\r\n", clientName );

      } else if ( 0 == strncmp( line, "ECHO ", 5 ) ) {
         snprintf( reply, BUFSZ, "OK ECHO %s\r\n", line + 5 );

      } else if ( 0 == strcmp( line, "TIME" ) ) {
         time_t now = time( nullptr );
         struct tm tmNow;
         localtime_r( &now, &tmNow );
         char hhmmss[16];
         strftime( hhmmss, sizeof( hhmmss ), "%H:%M:%S", &tmNow );
         snprintf( reply, BUFSZ, "OK TIME %s\r\n", hhmmss );

      } else if ( 0 == strcmp( line, "QUIT" ) ) {
         snprintf( reply, BUFSZ, "BYE\r\n" );
         write( fd, reply, strlen( reply ) );
         break;

      } else {
         snprintf( reply, BUFSZ, "ERR comando no reconocido\r\n" );
      }

      write( fd, reply, strlen( reply ) );

   }

   close( fd );

}

/**
  * Punto de entrada de cada hilo: recibe el fd via malloc'd int *.
 **/
static void * sessionThread( void * arg ) {

   int fd = *(int *) arg;
   free( arg );

   printf( "[servidor] atendiendo conexion (fd=%d) en hilo %lu\n",
           fd, (unsigned long) pthread_self() );

   handleSession( fd );

   printf( "[servidor] conexion (fd=%d) finalizada\n", fd );

   return nullptr;

}

int main( int argc, char * argv[] ) {

   int listenPort = ( argc > 1 ) ? atoi( argv[1] ) : 5000;

   int listenFd = socket( AF_INET, SOCK_STREAM, 0 );
   if ( -1 == listenFd ) {
      perror( "socket" );
      return 1;
   }

   int yes = 1;
   setsockopt( listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) );

   struct sockaddr_in addr;
   memset( &addr, 0, sizeof( addr ) );
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port        = htons( listenPort );

   if ( -1 == bind( listenFd, (struct sockaddr *) &addr, sizeof( addr ) ) ) {
      perror( "bind" );
      return 1;
   }

   if ( -1 == listen( listenFd, BACKLOG ) ) {
      perror( "listen" );
      return 1;
   }

   printf( "[servidor] ECCIP/1.0 escuchando en el puerto %d\n", listenPort );

   for ( ;; ) {

      struct sockaddr_in clientAddr;
      socklen_t clientLen = sizeof( clientAddr );

      int clientFd = accept( listenFd, (struct sockaddr *) &clientAddr, &clientLen );
      if ( -1 == clientFd ) {
         perror( "accept" );
         continue;
      }

      int * fdCopy = (int *) malloc( sizeof( int ) );
      *fdCopy = clientFd;

      pthread_t tid;
      pthread_create( &tid, nullptr, sessionThread, fdCopy );
      pthread_detach( tid );        // el hilo se limpia solo al terminar

   }

   close( listenFd );
   return 0;

}
