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

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];
    int n;

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
    /* listen(sockfd, 5);  <-- COMENTADO A PROPÓSITO */


    /* A partir de acá, el servidor queda bloqueado */
    pause();


    /* 8. Bloquearse esperando un cliente */
    /* accept() nunca va a retornar porque no hay conexiones en cola */
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");


    // ========================= Comunicación ========================= //


    /* 9. Leer los datos del cliente */
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) 
        error("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);


    /* 10. Envia una respuesta al cliente */
    n = write(newsockfd, "I got your message", 18);
    if (n < 0) 
        error("ERROR writing to socket");


    /* 11. Cierra los sockets */
    close(newsockfd);
    close(sockfd);

    return 0; 

}