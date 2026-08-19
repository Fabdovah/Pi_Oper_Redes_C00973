/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *
  *  Cliente de prueba para el protocolo propio "ECCIP/1.0".
  *  Usa la clase Socket (derivada de VSocket) ya completada.
  *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VSocket.h"
#include "Socket.h"

/**
  * Envia una linea (agregando "\r\n") y lee la respuesta del servidor.
 **/
static void sendAndPrint( VSocket * s, const char * line ) {

   char msg[512];
   snprintf( msg, sizeof( msg ), "%s\r\n", line );
   s->Write( msg );

   char reply[512];
   memset( reply, 0, sizeof( reply ) );

   // Leemos byte a byte hasta '\n' para no bloquear esperando 512 bytes
   size_t total = 0;
   char c = 0;
   while ( total < sizeof( reply ) - 1 ) {
      size_t n = s->Read( &c, 1 );
      if ( 0 == n ) {
         break;                    // servidor cerro la conexion
      }
      if ( '\n' == c ) {
         break;
      }
      if ( '\r' != c ) {
         reply[total++] = c;
      }
   }

   printf( "C: %s\nS: %s\n\n", line, reply );

}

int main( int argc, char * argv[] ) {

   const char * host = ( argc > 1 ) ? argv[1] : "127.0.0.1";
   int port           = ( argc > 2 ) ? atoi( argv[2] ) : 5000;

   VSocket * s = new Socket( 's' );

   printf( "Conectando a %s:%d ...\n\n", host, port );
   s->Connect( host, port );

   sendAndPrint( s, "HELLO grupo5" );
   sendAndPrint( s, "ECHO probando el protocolo ECCIP" );
   sendAndPrint( s, "TIME" );
   sendAndPrint( s, "QUIT" );

   delete s;               // llama a Socket/VSocket destructor -> Close()

   return 0;

}
