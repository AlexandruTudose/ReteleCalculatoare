#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "constants.h"


int connectToServer(char *ip_address = (char *) IP_ADDRESS, int port = PORT){
    /*  Initialize a connection to a specified ip and port.
    If successful it returns an integer representing the socket descriptor of the connection.
    In case of failure it returns -1.
    */
 
    int socket_descriptor;
    struct sockaddr_in *server = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    memset(server, 0, sizeof(server));
    if((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror(ERR_C_SOCKET);
        return -1;
    }
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = inet_addr(ip_address);
    server->sin_port = htons(port);
    if(connect(socket_descriptor, (struct sockaddr *) server, sizeof(struct sockaddr)) == -1){
        perror (ERR_C_CONNECT);
        return -1;
    }
    printf(ACK_C_CONNECT, ip_address, port);
    return socket_descriptor;
}
 
 
int requestToServer(int socket_descriptor, char *request, char **response){
    /*  Initialize a request to the server and waiting for response.
    If succesful it returns 0 and the result in the *response string.
    In case of failure it returns -1.
    */
 
    int buffer_size = strlen(request);
    // Sending first the number of bytes of the actual request.
    if(write(socket_descriptor, &buffer_size, sizeof(int)) < 1){
        perror(ERR_C_WRITE);
        return -1;
    }
    // Sending the request.
    if(write(socket_descriptor, request, buffer_size) < 1){
        perror(ERR_C_WRITE);
        return -1;
    }
    printf(ACK_C_SENT, request);
    // Reading first the number of bytes of the actual response.
    if(read(socket_descriptor, &buffer_size, sizeof(int)) < 0){
        perror(ERR_C_READ);
        return -1;
    }
    // Reading the response.
    *response = (char *) malloc(buffer_size);
    if(read(socket_descriptor, *response, buffer_size) < 0){
        perror(ERR_C_READ);
        return -1;
    }
    printf(ACK_C_RECIVED, *response);
    return 0;
}


int intitServer(int port = PORT, int reuse_addr = 1){
    /*  Initialize a server to a specified port.
    If successful it returns the socket descriptor at which connections can be accepted.
    In case of failure it returns -1.
    */

    int socket_descriptor;
    struct sockaddr_in *server = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    memset(server, 0, sizeof(server));
    if((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror(ERR_S_SOCKET);
        return -1;
    }
    if(reuse_addr)
        setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    server->sin_family = AF_INET;    
    server->sin_addr.s_addr = htonl(INADDR_ANY);
    server->sin_port = htons(port);
    if(bind(socket_descriptor, (struct sockaddr *) server, sizeof(struct sockaddr)) == -1){
        perror(ERR_S_BIND);
        return -1;
    }
    if(listen(socket_descriptor, 2) == -1){
        perror(ERR_S_LISTEN);
        return -1;
    }
    printf(ACK_S_LISTEN, port);
    return socket_descriptor;
}


int acceptClient(int server_sd){
    /*  Accept a connection from a client.
    If successful it returns the socket descriptor of that client.
    In case of failure it returns -1.
    */

    int client_sd;
    struct sockaddr_in *client = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    memset(client, 0, sizeof(client));
    socklen_t sockaddr_size = sizeof(struct sockaddr);
    if ((client_sd = accept(server_sd, (struct sockaddr *) client, &sockaddr_size)) < 0){
        perror(ERR_S_ACCEPT);
        return -1;
    }
    printf(ACK_S_ACCEPT, client_sd);
    return client_sd;
}


int processRequest(int socket_descriptor, int interpret_request(char *, char **)){
    /*  Wait for a request and then respond to it.
    If the process was succesful it returns 1.
    In case of failure it returns -1.
    If this is the last request to process it returns 0;
    */

    int buffer_size;
    // Reading first the number of bytes of the actual request.
    if (read(socket_descriptor, &buffer_size, sizeof(int)) < 0){
        perror(ERR_S_READ);
        return -1;
    }
    // Reading the request.
    char *request = (char *) malloc(buffer_size);
    if (read(socket_descriptor, request, buffer_size) < 0){
        perror(ERR_S_READ);
        return -1;
    }
    printf(ACK_S_RECIVED, request);
    // Compute response
    char *response;
    int exit_flag = interpret_request(request, &response);
    buffer_size = strlen(response);
          
    // Sending first the number of bytes of the actual response.i
    if(write(socket_descriptor, &buffer_size, sizeof(int)) < 1){
        perror(ERR_S_WRITE);
        return -1;
    }
    // Sending the response.
    if(write(socket_descriptor, response, buffer_size) < 1){
        perror(ERR_S_WRITE);
        return -1;
    }
    printf(ACK_S_SENT, response);
    return exit_flag;
}


