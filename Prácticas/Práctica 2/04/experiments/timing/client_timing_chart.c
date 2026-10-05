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


ssize_t recv_all_timed(int sockfd, void *destino, size_t cantidad,
                       long long *tiempo_total_ns)
{
    size_t bytes_recibidos = 0;  /* contador para la cantidad de bytes */
    char *cursor = destino;      /* puntero para ir desplazándose */
    *tiempo_total_ns = 0;

    while (bytes_recibidos < cantidad) {
        struct timespec inicio_read, fin_read;

        /* Registrar T0i antes de read() */
        int resultado_t0 = clock_gettime(CLOCK_MONOTONIC, &inicio_read);

        /* Leer los bytes pendientes */
        ssize_t recibidos_esta_vez = read(
            sockfd,
            cursor + bytes_recibidos,
            cantidad - bytes_recibidos
        );

        /* Registrar T1i después de read() */
        int resultado_t1 = clock_gettime(CLOCK_MONOTONIC, &fin_read);

        /* Verificar los relojes fuera del intervalo medido */
        if (resultado_t0 < 0)
            error("ERROR reading clock for T0i");
        if (resultado_t1 < 0)
            error("ERROR reading clock for T1i");

        /* Sumar el tiempo de esta llamada a read() */
        *tiempo_total_ns +=
            (fin_read.tv_sec - inicio_read.tv_sec) * 1000000000LL +
            (fin_read.tv_nsec - inicio_read.tv_nsec);

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
    int sockfd, portno, n;
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


    // ========================= Experimento ========================= //

    /* 5. Configurar los tamaños y las repeticiones */
    uint32_t tamanos[] = {10, 100, 1000, 10000, 100000, 1000000};
    size_t cantidad_tamanos = sizeof(tamanos) / sizeof(tamanos[0]);
    const int REPETICIONES = 10;


    /* 6. Mostrar el encabezado CSV */
    printf("bytes,repeticiones,write_avg_ns,read_avg_ns\n");
    fflush(stdout);


    /* 7. Ejecutar el experimento para cada tamaño */
    for (size_t i = 0; i < cantidad_tamanos; i++) {
        uint32_t N = tamanos[i];

        /* 7.1. Crear un buffer de N bytes y llenarlo con datos */
        unsigned char *buf = malloc(N);
        if (!buf) error("malloc");
        for (uint32_t j = 0; j < N; j++) {
            buf[j] = (unsigned char)(j % 256);
        }

        /* 7.2. Inicializar los acumuladores */
        long long suma_write_ns = 0;
        long long suma_read_ns = 0;

        /* 7.3. Repetir las mediciones */
        for (int repeticion = 0; repeticion < REPETICIONES; repeticion++) {

            /* 7.3.1. Crear el socket */
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) 
                error("ERROR opening socket");

            /* 7.3.2. Conectarse al servidor */
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
                error("ERROR connecting");

            /* 7.3.3. Mandar la cantidad de bytes (no se mide) */
            uint32_t N_red = htonl(N);
            n = write(sockfd, &N_red, sizeof(N_red));
            if (n < 0)
                error("ERROR writing to socket");


            /* 8. Medir el tiempo de write() al enviar el buffer */
            struct timespec inicio_write, fin_write;

            /* 8.1. Registrar T0 antes de write() */
            if (clock_gettime(CLOCK_MONOTONIC, &inicio_write) < 0)
                error("ERROR reading clock for T0");

            /* 8.2. Enviar el buffer de datos */
            n = write(sockfd, buf, N);

            /* 8.3. Registrar T1 después de write() */
            if (clock_gettime(CLOCK_MONOTONIC, &fin_write) < 0)
                error("ERROR reading clock for T1");

            if (n < 0)
                error("ERROR writing to socket");

            /* 8.4. Calcular y acumular el tiempo de write() */
            long long tiempo_write_ns =
                (fin_write.tv_sec - inicio_write.tv_sec) * 1000000000LL +
                (fin_write.tv_nsec - inicio_write.tv_nsec);
            suma_write_ns += tiempo_write_ns;


            /* 9. Medir y sumar las llamadas a read() de la confirmación */
            uint32_t confirmacion_red;
            long long tiempo_read_ns;

            ssize_t bytes_confirmacion = recv_all_timed(
                sockfd,
                &confirmacion_red,
                sizeof(confirmacion_red),
                &tiempo_read_ns
            );

            if (bytes_confirmacion != sizeof(confirmacion_red))
                error("ERROR recibiendo confirmación");
            suma_read_ns += tiempo_read_ns;

            /* 9.1. Verificar la confirmación del Servidor */
            uint32_t confirmacion = ntohl(confirmacion_red);
            if (confirmacion != N) {
                fprintf(stderr,
                        "Cliente: ERROR, el servidor no verificó correctamente los datos\n");
                free(buf);
                close(sockfd);
                exit(1);
            }


            /* 10. Cerrar la conexión de esta repetición */
            close(sockfd);
        }


        /* 11. Mostrar los promedios de este tamaño en formato CSV */
        printf("%u,%d,%.2f,%.2f\n",
               N,
               REPETICIONES,
               (double)suma_write_ns / REPETICIONES,
               (double)suma_read_ns / REPETICIONES);
        fflush(stdout);


        /* 12. Liberar el buffer de este tamaño */
        free(buf);
    }

    return 0;

}
