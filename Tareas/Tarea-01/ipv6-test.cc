/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *
  *
 **/

#include <stdio.h>
#include <string.h>
#include "Socket.h"

int main( int argc, char * argv[] ) {
   const char * lab = "fe80::8f5a:e2e1:7256:ffe3%enp0s31f6";
   const char * request = "GET / HTTP/1.1\r\nhost: redes.ecci\r\n\r\n";

   Socket s( 's', true );
   char a[512];

   memset( a, 0, 512 );
   s.Connect( lab, (char *) "http" );
   s.Write( request );
   s.Read( a, 512 );
   printf( "%s\n", a);

}

