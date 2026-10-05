#include <stdio.h>
#include <stdlib.h>   // Para exit, atoi y otras funciones estándar
#include <string.h>   // Para bzero y otras funciones de cadenas
#include <unistd.h>   // Para read, write y otras funciones de E/S
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

ssize_t recv_all(int sockfd, void *destino, size_t cantidad)
{
    size_t bytes_recibidos = 0;  /* contador para la cantidad de bytes */
    char *cursor = destino;      /* puntero para ir desplazándose */

    while (bytes_recibidos < cantidad) {
        /* Leer los bytes pendientes */
        ssize_t recibidos_esta_vez = read(
            sockfd,
            cursor + bytes_recibidos,
            cantidad - bytes_recibidos
        );

        /* Error de lectura */
        if (recibidos_esta_vez < 0)
            return -1;

        /* Conexión cerrada */
        if (recibidos_esta_vez == 0)
            return bytes_recibidos;

        /* Acumular los bytes recibidos */
        bytes_recibidos += recibidos_esta_vez;
    }

    return bytes_recibidos;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // ========================= Configuración y Conexión ========================= //


    /* 1. Validar argumentos (programa + puerto) */
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }


    /* 2. Obtener el puerto de los argumentos */
    portno = atoi(argv[1]);


    /* 3. Creación del socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");


    /* 4. Reutilización del puerto (Opcional) */
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    

    /* 5. Configurar la dirección del servidor (serv_addr) */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


    /* 6. Vincula el socket a la dirección del servidor (serv_addr) */ 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");


    /* 7. Habilita el Socket para escuchar conexiones (hasta 5 en la cola de espera) */
    listen(sockfd, 5);
    printf("Server listening on port %d\n", portno);
    fflush(stdout);

    // ========================= Comunicación ========================= //

    while (1) {

        /* 8. Bloquearse esperando un cliente */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");


        /* 9. Leer los datos del cliente */
        /* 9.1. Leer la cantidad de bytes que el cliente enviará */
        uint32_t N_red;
        if (recv_all(newsockfd, &N_red, sizeof(N_red)) != sizeof(N_red)) {
            fprintf(stderr, "Error leyendo N\n");
            close(newsockfd);
            continue;
        }
        uint32_t N = ntohl(N_red);

        if (N != 10 && N != 100 && N != 1000 &&
            N != 10000 && N != 100000 && N != 1000000) {
            fprintf(stderr, "Error: cantidad de bytes no permitida: %u\n", N);
            close(newsockfd);
            continue;
        }
        printf("Servidor: recibiré %u bytes\n", N);
        fflush(stdout);

        
        /* 9.2. Leer los datos del cliente */
        unsigned char *buf = malloc(N);
        if (!buf) error("ERROR allocating memory");


        /* 9.3. Recibir todos los datos */
        ssize_t recibidos = recv_all(newsockfd, buf, N);
        int contenido_correcto = recibidos == (ssize_t)N;

        if (contenido_correcto) {
            for (uint32_t i = 0; i < N; i++) {
                if (buf[i] != (unsigned char)(i % 256)) {
                    contenido_correcto = 0;
                    break;
                }
            }
        }

        if (!contenido_correcto) {
            fprintf(stderr, "Error: los datos recibidos no son correctos\n");
        } else {
            printf("Servidor: verifiqué %u bytes correctamente\n", N);
        }
        fflush(stdout);


        /* 10. Envia una respuesta al cliente */
        uint32_t confirmacion = contenido_correcto ? N : 0;
        uint32_t confirmacion_red = htonl(confirmacion);
        n = write(newsockfd, &confirmacion_red, sizeof(confirmacion_red));
        if (n < 0)
            error("ERROR writing to socket");

        
        /* 11. Cierra los sockets */
        /* 11.1. Liberar memoria y cerrar el socket del cliente */
        free(buf);
        close(newsockfd);

    }

    /* 11.2. Cierra el socket del servidor */
    close(sockfd);

    return 0; 

}
