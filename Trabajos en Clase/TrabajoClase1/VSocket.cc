/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *******   VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons, inet_pton
#include <stdexcept>            // runtime_error
#include <cstring>		// memset, strncpy, strchr
#include <netdb.h>		// getaddrinfo, freeaddrinfo, gai_strerror
#include <unistd.h>		// close
#include <net/if.h>		// if_nametoindex (IPv6 scope id, ej. "%eno1")
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
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st = -1;

   this->port = port;

   if ( !IPv6 ) {

      struct sockaddr_in host4;
      memset( (char *) &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;

      st = inet_pton( AF_INET, hostip, &host4.sin_addr );
      if ( 1 != st ) {
         throw std::runtime_error( "VSocket::TryToConnect, inet_pton" );
      }

      host4.sin_port = htons( port );
      st = connect( sockId, (struct sockaddr *) &host4, sizeof( host4 ) );

   } else {

      struct sockaddr_in6 host6;
      memset( (char *) &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;

      // Soporta direcciones link-local con scope id, ej. "fe80::1%eno1"
      char addrbuf[INET6_ADDRSTRLEN] = { 0 };
      const char * pct = strchr( hostip, '%' );
      if ( nullptr != pct ) {
         size_t len = pct - hostip;
         strncpy( addrbuf, hostip, len );
         addrbuf[len] = '\0';
         host6.sin6_scope_id = if_nametoindex( pct + 1 );
      } else {
         strncpy( addrbuf, hostip, INET6_ADDRSTRLEN - 1 );
      }

      st = inet_pton( AF_INET6, addrbuf, &host6.sin6_addr );
      if ( 1 != st ) {
         throw std::runtime_error( "VSocket::TryToConnect, inet_pton (IPv6)" );
      }

      host6.sin6_port = htons( port );
      st = connect( sockId, (struct sockaddr *) &host6, sizeof( host6 ) );

   }

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
   struct addrinfo * res = nullptr;
   struct addrinfo * rp  = nullptr;

   memset( &hints, 0, sizeof( hints ) );
   hints.ai_family   = IPv6 ? AF_INET6 : AF_INET;
   hints.ai_socktype = ( 's' == type ) ? SOCK_STREAM : SOCK_DGRAM;

   st = getaddrinfo( host, service, &hints, &res );
   if ( 0 != st ) {
      throw std::runtime_error(
         std::string( "VSocket::TryToConnect, getaddrinfo: " ) + gai_strerror( st ) );
   }

   st = -1;
   for ( rp = res; nullptr != rp; rp = rp->ai_next ) {
      st = connect( sockId, rp->ai_addr, rp->ai_addrlen );
      if ( 0 == st ) {
         break;
      }
   }

   freeaddrinfo( res );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect, connect (host/service)" );
   }

   return st;

}

