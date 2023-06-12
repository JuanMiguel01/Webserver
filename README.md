# Proyecto de Servidor Web

Este proyecto es un servidor web simple escrito en C.

## Instalación

Para instalar el servidor web, sigue estos pasos:

1. Clona el repositorio del proyecto en tu máquina local.
2. Abre una terminal y navega hasta el directorio del proyecto.
3. Ejecuta el comando `gcc -g -Wall -o MiPrograma sockets.c Process_and_handler_request.c Process_and_handler_request.h generate_directory.c generate_directory.h -lpthread` para compilar el código fuente.

## Uso

Para ejecutar el servidor web, sigue estos pasos:

1. Abre una terminal y navega hasta el directorio del proyecto.
2. Ejecuta el comando `./webserver [port] [root_directory]`, donde `[port]` es el número de puerto en el que deseas que el servidor escuche las conexiones entrantes y `[root_directory]` es la ruta del directorio que deseas servir.
3. Abre un navegador web y navega a `http://localhost:[port]` para acceder al servidor web.

## Funcionalidades

Este servidor web cumple según nuestra opinión todos los puntos de la orientación
