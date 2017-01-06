#include "client-server.h"

#include <thread>
using namespace std;


int interpretRequest(char* request, char **response){
    /*  Puts in *response an answer for the request.
    If the process was successful it returns 1.
    If it was the last request it returns 0.
    */

    *response = (char*) malloc(strlen(request));
    for(int i=0; i<strlen(request); ++i){
        (*response)[strlen(request)-1-i] = request[i];
    }
    return 0;
}


int threadCode(int socket_descriptor){
    /*  The code that every thread executes for it's assigned client.
    */

    while(processRequest(socket_descriptor, &interpretRequest)){};
    return 0;
}


int main(){
    int server_sd = intitServer();
    LOOP{
        int client_sd;
        if((client_sd = acceptClient(server_sd)) != -1){
            // Thread spawn.
            thread *client_servant = new thread(threadCode, client_sd);
            client_servant->detach();
        }
    }
    return 0;
}