=========================================
   XINU TCP & TELNET FINAL - DOCUMENTACIÓN
=========================================

Implementación del protocolo TCP y un cliente TELNET en XINU, con entorno gráfico
y ejecución en QEMU.

=========================================
  ESTRUCTURA DEL PROYECTO
=========================================

xinu-tcp-final/
└── xinu-final/
    ├── compile/
    └── (archivos fuente)

=========================================
  COMPILACIÓN Y EJECUCIÓN
=========================================

1. Posicionarse en el directorio de compilación:
   cd xinu-tcp-final/xinu-final/compile

2. Limpiar compilaciones previas:
   make clean

3. Compilar:
   make

4. Ejecutar en QEMU:
   make run-qemu

=========================================
  ENTORNO GRÁFICO EN QEMU
=========================================

Al levantar XINU con 'make run-qemu':

- IP de XINU: 10.0.2.15
- IP de Linux (host): 10.0.2.2

Estas direcciones las maneja internamente QEMU.

IMPORTANTE:
- Desde XINU se puede salir a internet o alcanzar otras máquinas.
- Desde Linux no se puede acceder a XINU (QEMU crea una red privada por software).

Para verificar la IP dentro de XINU:
   netinfo

=========================================
  COMANDOS ÚTILES EN XINU
=========================================

clear          -> limpia la terminal
netinfo        -> muestra configuración de red
ping google.com -> verifica conectividad de red

=========================================
  NOTAS DEL ENTORNO GRÁFICO
=========================================

- Para hacer foco en una ventana de terminal XINU:
  hacer clic dentro de la ventana (NO en la barra de título).
- Si las teclas dejan de responder:
  volver a hacer clic dentro.
- Para cambiar a consola de texto:
  CTRL + ALT + 3
- Para volver al entorno gráfico:
  CTRL + ALT + 1

=========================================
  PRUEBA DE CLIENTE HTTP (heredado)
=========================================

Ejecutar desde XINU:
   client gnu.msn.by 80 /

=========================================
  PRUEBA DE CLIENTE TELNET
=========================================

Para probar el cliente TELNET de XINU, usar el comando:

   telnet <ip> <puerto>

Por ejemplo:
   telnet 10.0.2.2 23

(Esto intentará conectarse desde XINU a la IP y puerto especificados)

=========================================
  XINU EN VIRT-MANAGER (OPCIONAL)
=========================================

Para correr XINU desde Virt-Manager y conectarlo a la red local:

Video tutorial:
https://youtu.be/9rOI7ldjm_Y

=========================================
  SOBRE ESTE PROYECTO
=========================================

Incluye:
- Implementación de TCP
- Cliente TELNET
- Scripts de compilación para QEMU
- Entorno gráfico con varias terminales virtuales

=========================================
  AUTORES PARCIALES
=========================================

- Paula Coronel 
- Albany Petit
- Antonio Sarmiento

=========================================
