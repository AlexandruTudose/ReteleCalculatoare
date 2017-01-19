#ifndef CN_SERVER_H
#define CN_SERVER_H

#include "../client-server.h"

#include <thread>
#include <mutex>
#include <iostream>

#include <dirent.h>
using namespace std;


typedef struct my_files{
    int number_of_files;
    char **file_names;
}my_files;

typedef struct id_file_pair{
    int client_numbers[100];
    bool available_client[100];
    int client_size;
    char *file_name;
}id_file_pair;

class cnServer {
    /*This class is used for server management.
    The Collaborative Notepad server has a main thread that handle
    server initialization and thread spawning. This class is used
    to keep track of any active threads and their corespondet clients.

    */
public:

    cnServer(){
        client_number = 0;
        client_sd = new int[max_clients]();
        thread_id = new thread[max_clients]();
        available = new bool[max_clients]();
        loop_error = new bool[max_clients]();
        files_editing = (char **) malloc(sizeof(char *) * max_clients);
        for(int i=0; i<max_clients; ++i)
            updateFilesEditing(i);
        server_sd = initServer();
        files_tracked = 0;
    };

    void spawnThreads(int threadCode(cnServer *, int)){
        /* Spawns threads that handle new clients.
        */

        do{
            int current_client_number = getAvailableNumber();
            if((client_sd[current_client_number] = acceptClient(server_sd)) != -1){
                available[current_client_number] = true;
                loop_error[current_client_number] = false;
                thread_id[current_client_number] = thread(threadCode, this, current_client_number);
                thread_id[current_client_number].detach();
            }
            client_number++;
        }while(true);
    };

    int endThread(const int current_client_number){
        client_number--;
        available[current_client_number] = false;
        close(client_sd[current_client_number]);
        return 0;
    };

    const int getClientSD(const int current_client_number){
        return client_sd[current_client_number];
    };

    int *getAssociateUsers(const int current_client_number){
        int *users = (int *) malloc(client_number * sizeof(int));
        int index = 0;
        printf("Clientii lui %d: ", current_client_number);
        for(int i=0; i<max_clients && index<client_number-1; i++){
            if(client_sd[i] != client_sd[current_client_number] && files_editing[i] != NULL &&
               !strcmp(files_editing[i], files_editing[current_client_number])){
                printf("%d ", i);
                users[++index] = client_sd[i];
                loop_error[i] = true;
            }
        }
        users[0] = index;
        printf("\nClient_number: %d     \n", client_number);
        return users;
    };

    bool hasLoopError(int current_client_number){
        return loop_error[current_client_number];
    };

    void flipLoopError(int current_client_number){
        loop_error[current_client_number] = !(loop_error[current_client_number]);
    };


    my_files *getFileNames(){
        my_files *server_files = new my_files();
        server_files->number_of_files = 0;
        struct dirent *p_dirent;
        DIR *p_dir;
        p_dir = opendir(FILES_DIR);
        while((p_dirent = readdir(p_dir))){
            if(strcmp(p_dirent->d_name, (char *) ".") && strcmp(p_dirent->d_name, (char *) "..")){
                server_files->file_names = (char **) realloc(server_files->file_names, (server_files->number_of_files + 1) * sizeof(char *));
                (server_files->file_names)[(server_files->number_of_files)++] = (char *) malloc(strlen(p_dirent->d_name) + 1);
                memset((server_files->file_names)[(server_files->number_of_files) - 1], 0, strlen(p_dirent->d_name) + 1);
                strcpy(server_files->file_names[server_files->number_of_files - 1], p_dirent->d_name);
            }
        }
        return server_files;
    };

    char *getFileName(int current_client_number){
        return files_editing[current_client_number];
    };

    char *getFileName(char *request){
        char *file_name = (char *) malloc(strlen(request) - strlen(C_EDIT));
        strcpy(file_name, request + strlen(C_EDIT) + 1);
        return file_name;
    };

    char *getFileContent(char *file_name){
        char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
        sprintf(path, "%s/%s", FILES_DIR, file_name);
        char *buffer = (char *) malloc(max_clients);
        memset(buffer, 0, max_clients);
        FILE *f = fopen(path, "rb");
        int count = 0;
        while((buffer[count++] = fgetc(f)) != EOF);
        buffer[count-1]='\0';
        fclose(f);
        return buffer;
    };

    int getClientForFile(char *file_name, int current_client_number){
        int index = 0;
        for(int i=0; i<max_clients && index<client_number; ++i)
            if(available[i]){
                index++;
                //printf("siruri: %s %s\n", file_name, files_editing[i]);
                if(i != current_client_number && files_editing[i] && !strcmp(file_name, files_editing[i]))
                    return i;
            }
        return -1;
    };

    void updateFileContent(char *file_name){
            char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
            sprintf(path, "%s/%s", FILES_DIR, file_name);
            FILE *f = fopen(path, "wb");
            fprintf(f, "%s", update_text[0]);
            fclose(f);
    };

    void newFile(char *file_name){
        char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
        sprintf(path, "%s/%s", FILES_DIR, file_name);
        FILE *f = fopen(path, "wb");
        fclose(f);
    }

    void removeFile(char *file_name){
        char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
        sprintf(path, "%s/%s", FILES_DIR, file_name);
        remove(path);
    }

    void updateFileContent(char *file_name, int current_client_number){
       int update_client_number = getClientForFile(file_name, current_client_number);
        //printf("client de contact: %d\n", update_client_number);
        if(update_client_number != -1){
            char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
            sprintf(path, "%s/%s", FILES_DIR, file_name);
            FILE *f = fopen(path, "wb");
            sendNullEvent(client_sd[update_client_number], 1);
            update_semaphore = true;
            do{
            }while(update_semaphore);
            //printf("inUPDATE: \n");
            fprintf(f, "%s", update_text[0]);
            fclose(f);
            //printf("inUPDATE: \n");
        }
    };

    void addToEditTracker(int current_client_number, char *file_name){
        bool file_found = false;
        for(int i=0; i<files_tracked; ++i){
            if(!strcmp(file_name, edit_tracker[i].file_name)){
                file_found = true;
                edit_tracker[i].client_size++;
                int available_spot;
                for(int j=0; j<100; j++){
                    if(!edit_tracker[i].available_client[j]){
                        available_spot = j;
                        break;
                    }
                }
                edit_tracker[i].available_client[available_spot] = true;
                edit_tracker[i].client_numbers[available_spot] = current_client_number;
            }
        }
        if(file_found == false){
            files_tracked++;
            //edit_tracker = (id_filene_pair *) realloc (sizeof(id_file_pair) * files_tracked);
           // memset(edit_tracker[files_tracked - 1].available_clit, 0,
             //      sizeof(edit_tracker[files_tracked - 1].available_client));
            edit_tracker[files_tracked - 1].file_name = file_name;
            edit_tracker[files_tracked - 1].client_size = 1;
            edit_tracker[files_tracked - 1].client_numbers[0] = current_client_number;
            edit_tracker[files_tracked - 1].available_client[0] = true;
        }
    };

    void updateFilesEditing(int current_client_number, char *file_name=NULL){
        files_editing[current_client_number] = file_name;
    };

    void changeUpdateSemaphore(){
        update_semaphore = !update_semaphore;
    };

    char **update_text;

private:
    const int max_clients = 10000;
    const char* TEST_FILE = "test.txt";
    const char* FILES_DIR = "Files";

    int client_number;
    int server_sd;
    int *client_sd;
    bool *available;
    bool *loop_error;
    thread *thread_id;
    char **files_editing;


    const int getAvailableNumber(){
        for(int i=0; i<max_clients; ++i)
            if(!available[i])
                return i;
    };



    id_file_pair *edit_tracker;
    int files_tracked;


    bool update_semaphore;
    mutex m;
};




#endif // CN_SERVER_H
