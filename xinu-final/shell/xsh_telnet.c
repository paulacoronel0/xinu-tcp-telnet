#include <xinu.h>
#include <stdlib.h>
#include <dns.h>

#define PORT 23
#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31
#define MAX_BUFFER_SIZE 128

int32 sock; // Descriptor del socket
sid32 socket_sem; // Semáforo para proteger el socket
sid32 console_sem; // Semáforo para proteger la consola
sid32 exit_sem; // Semáforo para coordinar la salida
struct dentry *devptr;
int echo_enabled = 1; // Estado de ECHO: 0 si el servidor controla el eco

/*############# Codigo Cliente Telnet  #############*/

void enable_echo(int opcion) {  
    /*Opcion 0 para deshabilitar el eco, opcion 1 para reestablecerlo*/
    if(opcion == 0){
        echo_enabled = 0;
        ttycontrol(devptr, TC_NOECHO, 0, 0);  // Desactivar eco
    }else{
        echo_enabled = 1;
        ttycontrol(devptr, TC_ECHO, 0, 0);  // Activar eco
    }
}

// Función para negociar opciones Telnet 
void negotiate(int32 sock, unsigned char *buf, int32 len) {
    int32 i;
    printf("Negociación recibida: IAC %d %d (", buf[0], buf[1], buf[2]);
    if (buf[1] == DO) printf("DO ");
    else if (buf[1] == DONT) printf("DONT ");
    else if (buf[1] == WILL) printf("WILL ");
    else if (buf[1] == WONT) printf("WONT ");
    printf("opción %d)\n", buf[2]);

    // Manejo del tamaño de la ventana 
    if (buf[1] == DO && buf[2] == CMD_WINDOW_SIZE) {
        unsigned char tmp1[3] = {CMD, WILL, CMD_WINDOW_SIZE}; // WILL WINDOW_SIZE
        printf("Respondiendo: IAC WILL WINDOW_SIZE (255 251 31)\n");
        tcp_send(sock, tmp1, 3);
        unsigned char tmp2[9] = {CMD, 250, CMD_WINDOW_SIZE, 0, 80, 0, 24, 255, 240}; // SB WINDOW_SIZE 80x24 SE
        printf("Respondiendo: IAC SB WINDOW_SIZE 80x24 SE (255 250 31 0 80 0 24 255 240)\n");
        tcp_send(sock, tmp2, 9);
        return;
    }
    // Manejo para el resto de las caracteristicas
    for (i = 0; i < len; i++) {
        if (buf[i] == DO) {
            buf[i] = WONT;
            printf("Cambiando DO %d a WONT %d\n", buf[2], buf[2]);
            if(buf[i+1] == CMD_ECHO) {
                enable_echo(0);
            }
        } else if (buf[i] == WILL) {
            buf[i] = DO;
            printf("Cambiando WILL %d a DO %d\n", buf[2], buf[2]);
        }
    }
    printf("Enviando respuesta: IAC %d %d\n", buf[1], buf[2]);
    tcp_send(sock, buf, len);
}

// Leer una cadena desde la consola hasta Enter 
int getstring(char *buf, int max_len) {
    int pos = 0;
    char c;

    while (pos < max_len - 1) {
        c = getchar();
        if (c == '\n' || c == '\r') {
            buf[pos] = '\0';
            return pos;
        }
        buf[pos++] = c;
    }

    buf[pos] = '\0';
    return pos;
}


// Proceso para manejar la recepcion de los datos del servidor
void receive_process(void) {
    unsigned char recv_buf[MAX_BUFFER_SIZE];
    int32 recv_pos = 0;
    int32 len;

    while (1) {
        len = tcp_recv(sock, &recv_buf[recv_pos], 1);
        if (len == SYSERR) {
            sleepms(10); // Pausa para evitar uso excesivo de CPU
            continue;
        } else if (len == 0) {
            wait(console_sem);
            printf("Conexión cerrada por el servidor\n");
            signal(console_sem);
            signal(exit_sem); // Notificar salida
            break;
        } else if (len > 0) {
            if (recv_buf[recv_pos] == CMD) {
                // Procesar comando Telnet (IAC)
                len = tcp_recv(sock, &recv_buf[recv_pos + 1], 2);
                if (len == SYSERR || len == 0) {
                    wait(console_sem);
                    printf("Error o cierre al leer comando Telnet\n");
                    signal(console_sem);
                    signal(exit_sem);
                    break;
                }
                wait(socket_sem);
                negotiate(sock, &recv_buf[recv_pos], 3);
                signal(socket_sem);
                recv_pos = 0;
            } else {
                // Imprimir datos inmediatamente
                wait(console_sem);
                recv_buf[recv_pos + 1] = '\0';
                printf("%c", recv_buf[recv_pos]);
                signal(console_sem);
                recv_pos += len;
                if (recv_pos >= MAX_BUFFER_SIZE - 1) {
                    wait(console_sem);
                    recv_buf[recv_pos] = '\0';
                    printf("%s", recv_buf);
                    signal(console_sem);
                    recv_pos = 0;
                }
            }
        }
    }
}

// Proceso para manejar entrada y envio del cliente al server
void input_process(void) {
    unsigned char input_buf[MAX_BUFFER_SIZE];
    int32 input_pos = 0;

    while (TRUE) {
        int32 c = getchar();
        if (c == EOF) {
            sleepms(10); // Pausa si no hay entrada
            continue;
        }

        input_buf[input_pos] = (unsigned char)c;
        if (input_buf[input_pos] == '\r' || input_buf[input_pos] == '\n') {
            // Enviar CRLF para Telnet
            input_buf[0] = '\n';
            input_buf[1] = '\r';
            wait(socket_sem);
            if (tcp_send(sock, input_buf, 2) == SYSERR) {
                wait(console_sem);
                printf("Error al enviar datos\n");
                signal(console_sem);
                signal(exit_sem);
                break;
            }
            signal(socket_sem);
            wait(console_sem);
            printf("\r\n");
            signal(console_sem);
            input_pos = 0;
        } else {
            // Verificar comando "salir"
            if (input_pos < 5) {
                input_buf[input_pos + 1] = '\0';
                if (strncmp((char *)input_buf, "salir", input_pos + 1) == 0 && input_pos == 4) {
                    wait(console_sem);
                    printf("Cerrando conexión por comando 'salir'\n");
                    signal(console_sem);
                    signal(exit_sem);
                    break;
                }
            }
            // Enviar carácter individual
            wait(socket_sem);
            if (tcp_send(sock, &input_buf[input_pos], 1) == SYSERR) {
                wait(console_sem);
                printf("Error al enviar datos\n");
                signal(console_sem);
                signal(exit_sem);
                break;
            }
            signal(socket_sem);
            wait(console_sem);
            printf("%c", input_buf[input_pos]); // Eco local
            signal(console_sem);
            input_pos++;
            if (input_pos >= MAX_BUFFER_SIZE - 1) {
                input_pos = 0; // Reiniciar búfer
            }
        }
    }
}

shellcmd xsh_telnet(int nargs, char *args[]) {
    int32 len;
    int32 sock;
    int32 ipaddr;
    int port;
    int input_pos = 0;
    int recv_pos = 0;

    devptr = &devtab[CONSOLE];  // Obtén el puntero al dispositivo de consola

    printf("nargs: %d\n", nargs);
    
    for (int i = 0; i < nargs; i++) {
        printf("args[%d]: %s\n", i, args[i]);
    }

    // Verifica los argumentos
    if (nargs < 2 || nargs > 3) {
        printf("Uso: %s IP PUERTO\n", args[0]);
        return 1;
    }

    // Convierte IP o lookup DNS
    if (dot2ip(args[1], &ipaddr) == SYSERR) {
        ipaddr = dnslookup(args[1]); // Intentar resolver como nombre de dominio
        if (ipaddr == SYSERR) {
            printf("%s: Dirección IP o dominio inválido\n", args[1]);
            return 1;
        }
    }

    // Puerto 23 por defecto (Telnet), o usar argumento
    port = (nargs == 3) ? atoi(args[2]) : PORT;

    printf("Conectando a %s: %d...\n", args[1], port);

    // Conexión TCP
    sock = tcp_register(ipaddr, port, 1);
    if (sock == SYSERR) {
        printf("Fallo al conectar\n");
        return 1;
    }
    printf("Conexión lista!!!!\n");
    enable_echo(0);

    // Creanos los semaforos
    socket_sem = semcreate(1); // Semaforo  para el socket
    console_sem = semcreate(1); // Semaforo para la consola
    exit_sem = semcreate(0); // Semaforo para salida (inicia en 0)
    if (socket_sem == SYSERR || console_sem == SYSERR || exit_sem == SYSERR) {
        printf("Error al crear semáforos\n");
        enable_echo(1);
        tcp_close(sock);
        return 1;
    }

    // Creamos los procesos
    pid32 recv_pid = create(receive_process, 1024, 20, "receive_process", 0);
    pid32 input_pid = create(input_process, 1024, 20, "input_process", 0);
    if (recv_pid == SYSERR || input_pid == SYSERR) {
        printf("Error al crear procesos\n");
        enable_echo(1);
        tcp_close(sock);
        return 1;
    }

    // Iniciar procesos
    resume(recv_pid);
    resume(input_pid);

    // Esperar a que se señale la salida
    wait(exit_sem);

    // Reactivamos el eco y cerrramos la conexion
    enable_echo(1);
    tcp_close(sock);

    // Eliminamos a los procesos de envio y recepcion
    kill(recv_pid);
    kill(input_pid);

    return OK;
}
