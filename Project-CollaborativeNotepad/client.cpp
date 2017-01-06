#include "client-server.h"


int main (int argc, char *argv[]){
    int socket_descriptor = connectToServer();
    char *request = (char *) "Request!";
    char *response;
    requestToServer(socket_descriptor, request, &response);
    close(socket_descriptor);
    return 0;
}
