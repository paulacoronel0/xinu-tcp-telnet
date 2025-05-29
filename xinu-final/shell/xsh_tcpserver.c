/*  tcpclient.c  - a tcp client for XINU */

/* #include <xinu.h>

shellcmd xsh_tcpserver(int nargs, char *args[])
{
    int32 res, i, len;
	int32 sock;
	int32 ipaddr;
	int port;
	unsigned char msg[40];

        if (nargs < 3) {
                printf("%s: invalid arguments\n", args[0]);
                printf("%s IP PORT\n", args[0]);
                return 1;
        }

	res = dot2ip(args[1], &ipaddr);
	if ((int32)res == SYSERR) {
		printf("%s: invalid IP address\n", args[1]);
		return 1;
	}

	port = atoi(args[2]);
    
    printf("connecting to server... \n");
    sock = tcp_register(ipaddr, port, -1);

} */

#include <xinu.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define DELAY_SECONDS 15

shellcmd xsh_tcpserver(int nargs, char *args[])
{
    int32 res, len;
	int32 sock;
	int32 ipaddr;
	int port;
	unsigned char buffer[BUFFER_SIZE];

    if (nargs < 3) {
        printf("%s: invalid arguments\n", args[0]);
        printf("Uso: %s IP PORT\n", args[0]);
        return 1;
    }

	// IP no se usa directamente en modo servidor, pero se puede validar
	res = dot2ip(args[1], &ipaddr);
	if ((int32)res == SYSERR) {
		printf("%s: invalid IP address\n", args[1]);
		return 1;
	}

	port = atoi(args[2]);

    // Modo servidor (listen == 0)
    sock = tcp_register(0, port, 0);
    if (sock == SYSERR) {
        printf("Error: no se pudo registrar el socket en el puerto %d\n", port);
        return 1;
    }

    printf("Servidor TCP esperando conexión en puerto %d...\n", port);

    // Esperar mensaje del cliente
    res = tcp_recv(sock, buffer, BUFFER_SIZE - 1);
    if (res > 0) {
        buffer[res] = '\0';
        printf("Mensaje recibido (%d bytes): %s\n", res, buffer);
    } else {
        printf("Error al recibir mensaje o conexión cerrada.\n");
        tcp_close(sock);
        return 1;
    }

    // Delay antes de responder
    printf("Esperando %d segundos antes de responder...\n", DELAY_SECONDS);
    sleep(DELAY_SECONDS); // Xinu's sleep() espera segundos

    // Enviar respuesta
    char *respuesta = "Respuesta del servidor Xinu con delay";
    tcp_send(sock, respuesta, strlen(respuesta));
    printf("Respuesta enviada.\n");

    tcp_close(sock);
    return OK;
}

