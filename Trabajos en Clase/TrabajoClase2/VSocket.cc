/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  ****** VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons, inet_pton
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo, gai_strerror
#include <unistd.h>		// close
#include <string>

#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::Init( char t, bool IPv6 ){

   this->type = t;
   this->IPv6 = IPv6;
   this->port = 0;

   int domain   = IPv6 ? AF_INET6 : AF_INET;
   int socktype = ( 's' == t ) ? SOCK_STREAM : SOCK_DGRAM;

   sockId = socket( domain, socktype, 0 );

   if ( -1 == sockId ) {
      throw std::runtime_error( "VSocket::Init, socket" );
   }

}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {

   this->Close();

}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){

   int st = 0;

   if ( sockId >= 0 ) {
      st = close( sockId );
      sockId = -1;
   }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
   }

}


/**
  * TryToConnect method
  *   use "connect" Unix system call (usado en TCP; se deja implementado
  *   por completitud de la clase base aunque este trabajo es sobre UDP)
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st = -1;

   this->port = port;

   struct sockaddr_in host4;
   memset( (char *) &host4, 0, sizeof( host4 ) );
   host4.sin_family = AF_INET;

   st = inet_pton( AF_INET, hostip, &host4.sin_addr );
   if ( 1 != st ) {
      throw std::runtime_error( "VSocket::TryToConnect, inet_pton" );
   }

   host4.sin_port = htons( port );
   st = connect( sockId, (struct sockaddr *) &host4, sizeof( host4 ) );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect, connect" );
   }

   return st;

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect( const char *host, const char *service ) {

   int st = -1;
   struct addrinfo hints;
   struct addrinfo * result = nullptr;
   struct addrinfo * rp     = nullptr;

   memset( &hints, 0, sizeof( struct addrinfo ) );
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = ( 's' == type ) ? SOCK_STREAM : SOCK_DGRAM;

   st = getaddrinfo( host, service, &hints, &result );
   if ( 0 != st ) {
      throw std::runtime_error(
         std::string( "VSocket::TryToConnect, getaddrinfo: " ) + gai_strerror( st ) );
   }

   st = -1;
   for ( rp = result; nullptr != rp; rp = rp->ai_next ) {
      st = connect( sockId, rp->ai_addr, rp->ai_addrlen );
      if ( 0 == st ) {
         break;
      }
   }

   freeaddrinfo( result );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect, connect (host/service)" );
   }

   return st;

}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {

   int st = -1;
   struct sockaddr_in host4;

   this->port = port;

   memset( (char *) &host4, 0, sizeof( host4 ) );
   host4.sin_family = AF_INET;
   host4.sin_addr.s_addr = htonl( INADDR_ANY );      // acepta datagramas en cualquier interfaz local
   host4.sin_port = htons( port );
   memset( host4.sin_zero, '\0', sizeof( host4.sin_zero ) );

   st = bind( sockId, (struct sockaddr *) &host4, sizeof( host4 ) );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Bind, bind" );
   }

   return st;

}


/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data (apunta a un struct sockaddr_in ya lleno)
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {

   ssize_t st;
   struct sockaddr_in * dest = (struct sockaddr_in *) addr;

   st = sendto( sockId, buffer, size, 0, (struct sockaddr *) dest, sizeof( struct sockaddr_in ) );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::sendTo, sendto" );
   }

   return (size_t) st;

}


/**
  *  recvFrom method
  *
  *  @param	void * buffer: buffer donde se guardan los datos recibidos
  *  @param	size_t size capacidad del buffer
  *  @param	void * addr apunta a un struct sockaddr_in; se llena con la
  *                        direccion de quien envio el datagrama (identifica al emisor)
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {

   ssize_t st;
   struct sockaddr_in * from = (struct sockaddr_in *) addr;
   socklen_t fromlen = sizeof( struct sockaddr_in );

   st = recvfrom( sockId, buffer, size, 0, (struct sockaddr *) from, &fromlen );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::recvFrom, recvfrom" );
   }

   return (size_t) st;

}

