#include "cn_server.h"

using namespace std;

cnServer *server = new cnServer();

int interpretRequest(char* request, char **response, int client_number){
    /*  Puts in *response an answer for the request.
    If the process was successful it returns 1.
    If it was the last request it returns 0.
    */
    if(!strcmp(request, (char *) "")){
        return 1;
    }
    if(!strcmp(request, C_EMPTY)){
        return 1;
    }
    if(!strcmp(request, C_UPDATE)){
        int number;
        readStrings(server->getClientSD(client_number), &number, &server->update_text);
        server->changeUpdateSemaphore();
        return 1;
    }
    if(!strcmp(request, C_EXIT)){
        return server->endThread(client_number);
    }
    if(!strcmp(request, C_NULL_EVENT)){
        //removeFromEditTracker(client_number);
        int number;
        readStrings(server->getClientSD(client_number), &number, &server->update_text);
        server->updateFileContent(server->getFileName(client_number));
        server->updateFilesEditing(client_number);
        return sendNullEvent(server->getClientSD(client_number));
    }
    if(!strcmp(request, C_EVENT)){
        textBoxEvent *event = new textBoxEvent;
        readEvent(server->getClientSD(client_number), &event);
        if(server->hasLoopError(client_number)){
            server->flipLoopError(client_number);
            return 1;
        }

        int *usrs = server->getAssociateUsers(client_number);
        for(int sd = 1; sd<=usrs[0]; sd++){
            //printf("from here %d", sd);
            fflush(0);
            writeEvent(usrs[sd], event);
        }
        return 1;
    }
    if(!strcmp(request, C_FILES)){
        my_files *files = server->getFileNames();
        writeStrings(server->getClientSD(client_number), files->number_of_files, files->file_names);
        return 1;
    }
    if( strstr(request, C_EDIT)){
        char *file_name;
        file_name = server->getFileName(request);
        //server->addToEditTracker(file_name, client_number);
        server->updateFilesEditing(client_number, file_name);
        //printf("filename: %s\n", file_name);
        server->updateFileContent(file_name, client_number);
        *response = server->getFileContent(file_name);
        return 1;
    }
    if( strstr(request, C_NEW)){
        char *file_name;
        file_name = server->getFileName(request);
        server->newFile(file_name);
    }

    if (strstr(request, C_REMOVE)){
        char *file_name;
        file_name = server->getFileName(request);
        server->removeFile(file_name);
    }

    return 1;
}


int threadCode(cnServer *server, int client_number){
    /*  The code that every thread executes for it's assigned client.
    */
    do{
    }while(processRequest(server->getClientSD(client_number), client_number, &interpretRequest));
    return 0;
}


int main(){
    server->spawnThreads(&threadCode);
    return 0;
}
