#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>

#define SIZE 1024
#define CREDENTIALS "credentials.json"
#define MAX_ATTEMPTS 3

#define INTRO_1   "\n     WELCOME!\n This is a secured app. At this point the only two"
#define INTRO_2   "\ncommands available are \"login\" and \"exit\".\n"
#define I_U_R     " Please enter a username. The default is \"admin\".\n> "
#define I_P_R     " Please enter the password. The default is \"admin\".\n> "
#define I_ACC     "\n Nice to see you %s."
#define I_ERR     " Authentication failed."
#define I_E_T     " To many failed attempts."
#define I_EXIT    "\n Press Enter to Exit."
#define LOGIN     "login"
#define MYFIND    "myfind"
#define MYSTAT    "mystat"
#define EXIT      "exit"
#define MENU_1    "\n This is the main menu. You can choose from the fallowing options,"
#define MENU_2    "\nby writing the right command: \"myfind\", \"mystat\" or \"exit\"."
#define MENU_3    "\nWarning: \"myfind\" and \"mystat\" also require arguments. Type "
#define MENU_4    "\n\"help myfind\" or \"help mystat\" for more datails.\n"
#define REQUEST   "> "
#define NOT_FOUND " Command not found.\n"
#define DIR_ERR   "Directory can not be opened."

#define FIFO_1 "fifo_1"
#define FIFO_2 "fifo_2"
#define INIT_COMMUNICATION \
    int file_descriptors[4], child_pid;\
    if(type == external_pipe){\
        mknod(FIFO_1, S_IFIFO | 0666, 0);\
        mknod(FIFO_2, S_IFIFO | 0666, 0);\
    }\
    else if(type == internal_pipe){\
        pipe(file_descriptors);\
        pipe(file_descriptors + 2);\
    }\
    else{\
        socketpair(AF_UNIX, SOCK_STREAM, 0, file_descriptors);\
    }\
    child_pid = fork();
#define INIT_READING_IN_CHILD \
    if(!child_pid){\
        if(type != external_pipe)\
            close(file_descriptors[1]);\
        else \
            file_descriptors[0] = open(FIFO_1, O_RDONLY);
#define END_READING_IN_CHILD \
        if(type != socket_pair)\
            close(file_descriptors[0]);
#define INIT_WRITING_IN_CHILD \
        if(type == internal_pipe) close(file_descriptors[2]);\
        if(type == external_pipe) file_descriptors[3] = open(FIFO_2, O_WRONLY);
#define END_WRITING_IN_CHILD \
        close(file_descriptors[((type == socket_pair)?0:3)]);\
        _exit(0);\
    }
#define INIT_WRITING_IN_PARENT \
    if(type != external_pipe) close(file_descriptors[0]);\
    else file_descriptors[1] = open(FIFO_1, O_WRONLY);
#define END_WRITING_IN_PARENT \
    if(type != socket_pair) close(file_descriptors[1]);
#define INIT_READING_IN_PARENT \
    if(type == internal_pipe) close(file_descriptors[3]);\
    else if(type == external_pipe) file_descriptors[2] = open(FIFO_2, O_RDONLY);
#define END_READING_IN_PARENT \
    close(file_descriptors[((type == socket_pair)?1:2)]);

enum communication_type {internal_pipe, external_pipe, socket_pair};

char *readCredentials(){
    /*Put the content from credentials.json in a buffer.*/
    char *credentials = (char *) malloc(SIZE * SIZE);
    memset(credentials, 0, sizeof(char) * SIZE * SIZE);
    FILE *f = fopen(CREDENTIALS, "rb");
    int count = 0;
    while((credentials[count++] = fgetc(f)) != EOF);
    return credentials;
}

int verifyCredential(char *username, char *password){
    /*Verify the existance of the given name and password.*/
    char *current_credential = (char *) malloc(SIZE);
    memset(current_credential, 0, sizeof(char) * SIZE);
    char *credentials = readCredentials(); 
    sprintf(current_credential, "\"%s\": \"%s\"", username, password);
    int return_value = 0;
    if(strstr(credentials, current_credential))
         return_value = 1;
    free(current_credential);
    free(credentials);
    return return_value;
}

 char *readFromStdin(){
    /*Read from stdin untill '\n' is encountered.*/

    char *buffer = (char *) malloc(SIZE);
    memset(buffer, 0, sizeof(char) * SIZE);
    int count = 0;
    while((buffer[count++] = getchar()) != '\n');
    buffer[count-1] = '\0';
    return buffer;
 }

void exitProgram(){
    enum communication_type type = 0;
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
            char *command = (char *) malloc(SIZE);
            memset(command, 0, SIZE);
            read(file_descriptors[0], command, SIZE);
    END_READING_IN_CHILD
        if(!strcmp(command, EXIT))
            kill(getppid(), 9);
    INIT_WRITING_IN_CHILD
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        char *command = (char *) malloc(SIZE);
        memset(command, 0, SIZE);
        strcpy(command, EXIT);
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
    END_READING_IN_PARENT
}

int authenticate(){
    enum communication_type type = 1;
    
    int count = 0;
    while(count < MAX_ATTEMPTS){
        
        INIT_COMMUNICATION
        INIT_READING_IN_CHILD
            char *username = (char *) malloc(SIZE);
            memset(username, 0, SIZE);
            char *password = (char *) malloc(SIZE);
            memset(password, 0, SIZE);
            read(file_descriptors[0], username, SIZE);
            read(file_descriptors[0], password, SIZE);
        END_READING_IN_CHILD
            int unprocessed_response = verifyCredential(username, password);
            char *response = (char *) malloc (SIZE);
            memset(response, 0, SIZE);
            response[0] = '1'; //number of characters to be sent
            response[1] = ' ';
            response[2] = (unprocessed_response)?'1':'0';
        INIT_WRITING_IN_CHILD
            write(file_descriptors[((type == socket_pair)?0:3)], response, SIZE);
        END_WRITING_IN_CHILD
        INIT_WRITING_IN_PARENT
            count++;
            printf(I_U_R);
            char *username = readFromStdin();
            printf(I_P_R);
            char *password = readFromStdin();
            write(file_descriptors[1], username, SIZE);
            write(file_descriptors[1], password, SIZE);
        END_WRITING_IN_PARENT
        INIT_READING_IN_PARENT
            char *response = (char *) malloc(SIZE);
            memset(response, 0, SIZE);
            read(file_descriptors[((type == socket_pair)?1:2)], response, SIZE);
        END_READING_IN_PARENT
        
        if(response[2] == '1'){
            printf(I_ACC, username);
            count = MAX_ATTEMPTS + 1;
            return 0;
        }
        else{
            printf("%s", I_ERR);
        }
    }
    if(count == MAX_ATTEMPTS){
        printf("%s", I_E_T);
        printf(I_EXIT);
        char *input = readFromStdin();
        free(input);
        exitProgram();
    }
    return 1;
}

void getStat(char *path, char *stats){
    char *name = (char *) malloc(SIZE);
    memset(name, 0, sizeof(char) * SIZE);
    strcpy(name, basename(path));
    struct stat filestat;
    if (stat(path, &filestat) < 0)
        printf("error on stat");
    
    memset(stats, 0, sizeof(char) * SIZE);
    sprintf(stats, "  File: %s \n", name);
    sprintf(stats + strlen(stats), "  Size: %ld\t\t\tBlocks: %ld\t\tIO Block: %ld\n", filestat.st_size, filestat.st_blocks, filestat.st_blksize);
    sprintf(stats + strlen(stats), "Device: %ld\t\t\tInode: %ld\t\tLinks: %ld\n", filestat.st_dev, filestat.st_ino, filestat.st_nlink);
    sprintf(stats + strlen(stats), "Access: (%o/", filestat.st_mode);
    sprintf(stats + strlen(stats), (S_ISDIR(filestat.st_mode)) ? "d" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IRUSR) ? "r" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IWUSR) ? "w" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IXUSR) ? "x" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IRGRP) ? "r" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IWGRP) ? "w" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IXGRP) ? "x" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IROTH) ? "r" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IWOTH) ? "w" : "-");
    sprintf(stats + strlen(stats), (filestat.st_mode & S_IXOTH) ? "x" : "-");
    sprintf(stats + strlen(stats), ")\tUid: %d\t\tGid: %d\n", filestat.st_uid, filestat.st_gid);
    sprintf(stats + strlen(stats), "Access: %s", ctime((long *) &filestat.st_atim));
    sprintf(stats + strlen(stats), "Modify: %s", ctime((long *) &filestat.st_mtim));
    sprintf(stats + strlen(stats), "Change: %s\n", ctime((long *) &filestat.st_ctim));
}

void find(char *path, char *file, char* matches[], int *match_number){
    /*Recursive search that looks for an exact match which is than added to the "matches" vector of strings.*/
    DIR *directory;
    struct dirent *directory_entry;
    char *buffer = (char *) malloc(SIZE);
    memset(buffer, 0, sizeof(char) * SIZE);
    if(path[strlen(path) - 1] == '/')
        path[strlen(path) - 1] = '\0';
    if(!(directory = opendir(path)))
        perror(DIR_ERR);
    while((directory_entry = readdir(directory))) 
        if(strcmp(directory_entry->d_name, ".") && strcmp(directory_entry->d_name, "..")){
                memset(buffer, 0, sizeof(char) * SIZE);
                sprintf(buffer, "%s/%s", path, directory_entry->d_name);
                if(!strcmp(directory_entry->d_name, file))
                    sprintf(matches[(*match_number)++], "%s", buffer);
                if(directory_entry->d_type == DT_DIR)
                    find(buffer, file, matches, match_number);
        }
    closedir(directory);
    free(buffer);
}

void findFile(char *command){
    //find
    enum communication_type type = 1;
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
            char *command = (char *) malloc(SIZE);
            memset(command, 0, SIZE);
            read(file_descriptors[0], command, SIZE);
    END_READING_IN_CHILD
        char *path = (char *) malloc(SIZE);
        memset(path, 0, sizeof(char) * SIZE);
        memcpy(path, command+strlen(MYFIND) + 1, strlen(command) - strlen(MYFIND) - 1);
        int space_location = 0;
        while(path[++space_location] != ' ');
        char *name = (char *) malloc(SIZE);
        memset(name, 0, sizeof(char) * SIZE);
        memcpy(name, path + space_location + 1, strlen(path) - space_location - 1);
        memset(path + space_location, 0, strlen(path) - space_location); 
        int match_number = SIZE;
        char *matches[SIZE];
        while(match_number--){
            matches[match_number] = (char *) malloc(SIZE);
            memset(matches[match_number], 0, sizeof(char) * SIZE);
        }
        match_number = 0;
        find(path, name, matches, &match_number);
    INIT_WRITING_IN_CHILD
    for(int i=0; i<SIZE; ++i)
        write(file_descriptors[((type == socket_pair)?0:3)], matches[i], SIZE);
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
        int match_number = SIZE;
        char *response[SIZE];
        while(match_number--){
            response[match_number] = (char *) malloc(SIZE);
            memset(response[match_number], 0, sizeof(char) * SIZE);
        }
    for(int i=0; i<SIZE; ++i)
        read(file_descriptors[((type == socket_pair)?1:2)], response[i], SIZE);
    END_READING_IN_PARENT
    for(int i=0; i<SIZE && response[i][0]; ++i)
        printf("%s\n", response[i]);
}

void statFile(char *command){
    //stat command
    enum communication_type type = 0;
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
            char *command = (char *) malloc(SIZE);
            memset(command, 0, SIZE);
            read(file_descriptors[0], command, SIZE);
    END_READING_IN_CHILD
        char *path = (char *) malloc(SIZE);
        memset(path, 0, sizeof(char) * SIZE);
        memcpy(path, command+strlen(MYSTAT) + 1, strlen(command) - strlen(MYSTAT) - 1);
        char *response = (char *) malloc(SIZE);
        memset(response, 0, sizeof(char) * SIZE);
        
        if(path) getStat(path, response);
    INIT_WRITING_IN_CHILD
        write(file_descriptors[((type == socket_pair)?0:3)], response, SIZE);
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
        char *response = (char *) malloc(SIZE);
        memset(response, 0, SIZE);
        read(file_descriptors[((type == socket_pair)?1:2)], response, SIZE);
    END_READING_IN_PARENT
    printf("%s", response);
}

void notFound(){
    //not foundi
    printf(NOT_FOUND);
}

int stringIsCommand(char *string, char *command){
    unsigned long counter = 0;
    for(;counter<strlen(command); counter++)
        if(string[counter] != command[counter])
            return 0;
    if(string[counter] != ' ')
        return 0;
    return 1;
}

int interpret(char *command, int before_login){
    /*Recognize and act according to the given command.*/
    if(!strcmp(EXIT, command))
        exitProgram();
    if(before_login){
        if(!strcmp (LOGIN, command))
            return authenticate();
    }
    else if(stringIsCommand(command, MYFIND))
        findFile(command);
    else if(stringIsCommand(command, MYSTAT))
        statFile(command);
    else
        notFound();
    return 1;
}

void menu(){
    printf(INTRO_1);
    printf(INTRO_2);
    char *command = NULL;
    do{
        printf(REQUEST);
        if(command) free(command);
        command = readFromStdin();
    }while(interpret(command, 1));
    printf(MENU_2);
    printf(MENU_3);
    printf(MENU_4);
    do{
        printf(REQUEST);
        if(command) free(command);
        command = readFromStdin();
    }while(interpret(command, 0));
}

int main(){
    menu();
    return 0;
}
