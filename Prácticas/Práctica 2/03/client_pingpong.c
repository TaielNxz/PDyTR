#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>  // Para strlen y otras funciones de cadenas
#include <unistd.h>  // Para read, write y otras funciones de E/S
#include <stdlib.h>  // Para exit y otras funciones estándar
#include <time.h>    // Para clock_gettime
#include <stdint.h>  // Para uint32_t

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


ssize_t send_all(int sockfd, const void *origen, size_t cantidad)
{
    size_t bytes_enviados = 0;       /* contador para la cantidad de bytes */
    const char *cursor = origen;     /* puntero para ir desplazándose */

    while (bytes_enviados < cantidad) {
        /* Escribir los bytes pendientes */
        ssize_t enviados_esta_vez = write(
            sockfd,
            cursor + bytes_enviados,
            cantidad - bytes_enviados
        );

        /* Error de escritura o conexión cerrada */
        if (enviados_esta_vez <= 0)
            return -1;

        /* Acumular los bytes enviados */
        bytes_enviados += enviados_esta_vez;
    }

    return bytes_enviados;
}


long long diferencia_ns(struct timespec inicio, struct timespec fin)
{
    return (fin.tv_sec - inicio.tv_sec) * 1000000000LL +
           (fin.tv_nsec - inicio.tv_nsec);
}


int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // ========================= Configuración ========================= //


    /* 1. Validar argumentos (programa + host + puerto) */
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(1);
    }


    /* 2. Obtener el puerto de los argumentos */
    portno = atoi(argv[2]);


    /* 3. Resolver el nombre del host */
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }


    /* 4. Configurar la dirección del servidor (serv_addr) */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);


    // ========================= Experimento Ping-Pong ========================= //

    /* 5. Configurar los tamaños y las repeticiones */
    uint32_t tamanos[] = {10, 100, 1000, 10000, 100000, 1000000};
    size_t cantidad_tamanos = sizeof(tamanos) / sizeof(tamanos[0]);
    const int REPETICIONES = 10;


    /* 6. Mostrar el encabezado CSV */
    printf("bytes,repeticiones,write_avg_ns,read_avg_ns,roundtrip_avg_ns,oneway_avg_ns\n");
    fflush(stdout);


    /* 7. Ejecutar el experimento para cada tamaño */
    for (size_t i = 0; i < cantidad_tamanos; i++) {
        uint32_t N = tamanos[i];

        /* 7.1. Crear los buffers para enviar y recibir N bytes */
        unsigned char *buf_envio = malloc(N);
        unsigned char *buf_recepcion = malloc(N);
        if (!buf_envio || !buf_recepcion)
            error("malloc");

        /* 7.2. Llenar el buffer de envío con datos conocidos */
        for (uint32_t j = 0; j < N; j++) {
            buf_envio[j] = (unsigned char)(j % 256);
        }

        /* 7.3. Inicializar los acumuladores */
        long long suma_write_ns = 0;
        long long suma_read_ns = 0;
        long long suma_roundtrip_ns = 0;

        /* 7.4. Repetir las mediciones */
        for (int repeticion = 0; repeticion < REPETICIONES; repeticion++) {

            /* 7.4.1. Crear el socket */
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket");

            /* 7.4.2. Conectarse al servidor */
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR connecting");

            /* 7.4.3. Mandar la cantidad de bytes (no se mide) */
            uint32_t N_red = htonl(N);
            if (send_all(sockfd, &N_red, sizeof(N_red)) != sizeof(N_red))
                error("ERROR writing N to socket");


            /* 8. Medir el tiempo total de ida y vuelta (PING-PONG) */
            struct timespec inicio_roundtrip, fin_roundtrip;
            struct timespec inicio_write, fin_write;
            struct timespec inicio_read, fin_read;

            /* 8.1. Registrar T0 antes de enviar los N bytes */
            if (clock_gettime(CLOCK_MONOTONIC, &inicio_roundtrip) < 0)
                error("ERROR reading clock for roundtrip T0");
            inicio_write = inicio_roundtrip;

            /* 8.2. Enviar al servidor los N bytes (PING) */
            if (send_all(sockfd, buf_envio, N) != (ssize_t)N)
                error("ERROR writing data to socket");

            /* 8.3. Registrar el final de la escritura */
            if (clock_gettime(CLOCK_MONOTONIC, &fin_write) < 0)
                error("ERROR reading clock for write T1");

            /* 8.4. Recibir del servidor los mismos N bytes (PONG) */
            if (clock_gettime(CLOCK_MONOTONIC, &inicio_read) < 0)
                error("ERROR reading clock for read T0");

            if (recv_all(sockfd, buf_recepcion, N) != (ssize_t)N)
                error("ERROR reading data from socket");

            /* 8.5. Registrar T1 después de recibir los N bytes */
            if (clock_gettime(CLOCK_MONOTONIC, &fin_read) < 0)
                error("ERROR reading clock for read T1");
            fin_roundtrip = fin_read;

            /* 8.6. Calcular y acumular los tiempos */
            suma_write_ns += diferencia_ns(inicio_write, fin_write);
            suma_read_ns += diferencia_ns(inicio_read, fin_read);
            suma_roundtrip_ns += diferencia_ns(inicio_roundtrip, fin_roundtrip);


            /* 9. Verificar que los datos recibidos coincidan con los enviados */
            if (memcmp(buf_envio, buf_recepcion, N) != 0) {
                fprintf(stderr,
                        "Cliente: ERROR, los datos recibidos no coinciden con los enviados\n");
                free(buf_envio);
                free(buf_recepcion);
                close(sockfd);
                exit(1);
            }


            /* 10. Cerrar la conexión de esta repetición */
            close(sockfd);
        }


        /* 11. Mostrar los promedios de este tamaño en formato CSV */
        double roundtrip_avg_ns = (double)suma_roundtrip_ns / REPETICIONES;
        printf("%u,%d,%.2f,%.2f,%.2f,%.2f\n",
               N,
               REPETICIONES,
               (double)suma_write_ns / REPETICIONES,
               (double)suma_read_ns / REPETICIONES,
               roundtrip_avg_ns,
               roundtrip_avg_ns / 2.0);
        fflush(stdout);


        /* 12. Liberar los buffers de este tamaño */
        free(buf_envio);
        free(buf_recepcion);
    }

    return 0;
}
