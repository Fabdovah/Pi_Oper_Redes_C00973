# Protocolo ECCIP/1.0

Protocolo de texto, orientado a línea, sobre TCP (socket 's', IPv4).
Cada mensaje termina en "\r\n". El servidor siempre responde exactamente
un mensaje por cada mensaje del cliente (protocolo síncrono, sin pipelining).

## Mensajes del cliente -> servidor

| Comando            | Significado                              |
|---------------------|-------------------------------------------|
| `HELLO <nombre>`    | Handshake inicial, identifica al cliente  |
| `ECHO <texto>`      | Pide que el servidor devuelva `<texto>`   |
| `TIME`              | Pide la hora del servidor                 |
| `QUIT`              | Cierra la sesión                          |

## Mensajes del servidor -> cliente

| Respuesta                | Cuándo se envía                              |
|---------------------------|-----------------------------------------------|
| `OK HELLO <nombre>`       | Tras un `HELLO` válido                        |
| `OK ECHO <texto>`         | Tras un `ECHO`                                |
| `OK TIME <hh:mm:ss>`      | Tras un `TIME`                                |
| `BYE`                     | Tras un `QUIT`, luego el servidor cierra el socket |
| `ERR <motivo>`            | Comando no reconocido / mal formado           |

## Ejemplo de sesión

```
C: HELLO grupo5
S: OK HELLO grupo5
C: ECHO probando el protocolo
S: OK ECHO probando el protocolo
C: TIME
S: OK TIME 14:32:07
C: QUIT
S: BYE
```

## Concurrencia / IPC

El servidor hace `bind + listen + accept` en el hilo principal. Por cada
conexión aceptada (cada `accept()` exitoso) se lanza **un hilo POSIX**
(`pthread_create`) que atiende esa sesión completa de forma aislada,
usando su propio socket devuelto por `accept()`. El "IPC" entre el
cliente y el servidor es el socket TCP mismo (la red es el medio de
comunicación entre los dos procesos); entre los hilos del servidor no
hace falta memoria compartida porque cada hilo tiene su propio socket y
no comparte estado con los demás.
