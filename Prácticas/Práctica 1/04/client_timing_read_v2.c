/* Versión 2: suma el tiempo de cada llamada a read(). */
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

    // ========================= Configuración y Conexión ========================= //


    /* 1. Validar argumentos (programa + host + puerto + cantidad de bytes) */
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port bytes\n", argv[0]);
       exit(1);
    }


    /* 2. Obtener el puerto de los argumentos */
    portno = atoi(argv[2]);


    /* 2.1. Validar la cantidad de bytes a enviar */
    char *fin;
    unsigned long cantidad = strtoul(argv[3], &fin, 10);
    if (*argv[3] == '\0' || *fin != '\0' ||
        (cantidad != 10 && 
         cantidad != 100 && 
         cantidad != 1000 &&
         cantidad != 10000 && 
         cantidad != 100000 && 
         cantidad != 1000000)) {
        fprintf(stderr,"ERROR, bytes must be 10, 100, 1000, 10000, 100000 or 1000000\n");
        exit(1);
    }
    uint32_t N = (uint32_t)cantidad;


    /* 3. Crear Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
	

    /* 4. Resolver el nombre del host */
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    /* 5. Configurar la dirección del servidor (serv_addr) */
    bzero((char *) &serv_addr, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
	

    /* 6. Conectarse al servidor */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


    // ========================= Comunicación ========================= //

    /* 7.1. Mostrar el número de bytes a enviar */
    printf("Cliente: enviando %u bytes\n", N);


    /* 7.2. Mandar la cantidad de bytes (no se mide) */
    uint32_t N_red = htonl(N);
    n = write(sockfd, &N_red, sizeof(N_red));
    if (n < 0)
        error("ERROR writing to socket");


    /* 7.3. Crear un buffer de N bytes y llenarlo con datos */
    unsigned char *buf = malloc(N);
    if (!buf) error("malloc");
    for (uint32_t i = 0; i < N; i++) {
        buf[i] = (unsigned char)(i % 256);
    }


    /* 8. Enviar el buffer de datos (no se mide) */
    n = write(sockfd, buf, N);
    if (n < 0)
        error("ERROR writing to socket");


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


    /* 9.1. Mostrar el tiempo total de las llamadas a read() */
    printf("Cliente: read() de %zu bytes tomó %lld ns (%.3f us)\n",
           sizeof(confirmacion_red), tiempo_read_ns, tiempo_read_ns / 1000.0);


    /* 9.2. Verificar la confirmación del Servidor */
    uint32_t confirmacion = ntohl(confirmacion_red);

    if (confirmacion == N) {
        printf("Cliente: OK, servidor verificó %u bytes\n", confirmacion);
    } else {
        printf("Cliente: ERROR, el servidor no verificó correctamente los datos\n");
    }

    /* 10. Libera la memoria y Cierra la conexión */
    free(buf);
    close(sockfd);

    return 0;

}
