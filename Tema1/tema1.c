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
#include <sys/wait.h>
#include <signal.h>

#define SIZE 1024
#define CREDENTIALS "credentials.json"
#define MAX_ATTEMPTS 3

#define INTRO_1   "\n     WELCOME!\nThis is a secured app. At this point the only three"
#define INTRO_2   "\ncommands available are \"login\", \"cctt\" and \"quit\".\n"
#define I_U_R     "Please enter a username. The default is \"admin\".\n> "
#define I_P_R     "Please enter the password. The default is \"admin\".\n> "
#define I_ACC     "\nNice to see you %s!"
#define I_ERR     "Authentication failed."
#define I_E_T     "To many failed attempts."
#define I_EXIT    "\nPress Enter to quit."
#define LOGIN     "login"
#define MYFIND    "myfind"
#define MYSTAT    "mystat"
#define EXIT      "quit"
#define CCTT      "cctt"
#define CCTT_0    "cctt pipe"
#define CCTT_1    "cctt fifo"
#define CCTT_2    "cctt socket"
#define CCTT_T(A) (A)?((A==1)?"external pipe (fifo)":"socket pair"):"internal pipe"
#define CCTT_S    "Comunication type changed to %s.\n"
#define H_CCTT    "help cctt"
#define H_MYFIND  "help myfind"
#define H_MYSTAT  "help mystat"
#define MENU      "\nThis is the main menu. You can choose from the fallowing options, \
                  \nby writing the right command: \"myfind\", \"mystat\", \"cctt\" or \"quit\".\
                  \nWarning: \"myfind\", \"mystat\" and \"cctt\" also require arguments. Type \
                  \n\"help myfind\", \"help mystat\" or \"help cctt\" for more datails.\n"
#define REQUEST   "> "
#define NOT_FOUND "Command not found.\n"
#define H_F_MSG   "MYFIND - myfind\n    You can use this command to search for files around your computer.\
                   \nIt requires two arguments separated by space. The first one is the path\
                   \nand the second one is the name of the file you are looking for.\
                   \n    Example: \"myfind /home tema1.c\"\n"
#define H_S_MSG   "MYSTAT - mystat\n    You can use this command to discover information about a file from\
                   \nyour computer. It requires one argument that represent the path of the \
                   \nfile you want to discover information about.\n    Example: \"mystat /home/retele/tema1.c\"\n"
#define H_C_MSG   "CHANGE COMMUNICATION TYPE TO - cctt\n    You can use this command to switch between different communication types\
                   \nIt requires one argument representing the communication type. There are \
                   \nonly three communication types available: \"pipe\", \"fifo\" and \"socket\".\
                   \n    Example: \"cctt pipe\"\n"
#define ERR_OPENDIR "Directory can not be opened."
#define ERR_STAT    "Stat getting failed. Invalid path.\n"
#define ERR_ARGS    "This command is wrong or it requires different arguments. Type \"help %s\" for details.\n"
#define ERR_BUFF    "During the communication process an error occured and the message was altered."
#define Q_MSG       "Session ended.\n"

#define STRING(A); char *A = (char *) malloc(SIZE);\
                   memset(A, 0, SIZE);

#define ADD_SIZE(A);               {STRING(aux); sprintf(aux, "%ld %s", strlen(A), A); memcpy(A, aux, SIZE);}
#define CHECK_AND_REMOVE_SIZE(A);  {STRING(auxa); STRING(auxb); int counts = -1; while(A[++counts] != ' ') auxa[counts] = A[counts];\
                                               memcpy(A, A + counts +1, SIZE - counts - 1); sprintf(auxb, "%ld", strlen(A));\
                                               if(strcmp(auxa, auxb)) printf(ERR_BUFF);}

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

enum communication_type {internal_pipe, external_pipe, socket_pair} type = internal_pipe;

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
    STRING(current_credential);
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
    STRING(buffer);
    int count = 0;
    while((buffer[count++] = getchar()) != '\n');
    buffer[count-1] = '\0';
    return buffer;
 }

void exitProgram(){
    /*Kills the main process from a child by sending a Quit signal.*/
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
       STRING(command);
       read(file_descriptors[0], command, SIZE);
       CHECK_AND_REMOVE_SIZE(command);
    END_READING_IN_CHILD
        if(!strcmp(command, EXIT))
            kill(getppid(), SIGQUIT);
    INIT_WRITING_IN_CHILD
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        STRING(command);
        strcpy(command, EXIT);
        ADD_SIZE(command);
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
    END_READING_IN_PARENT
    sleep(1);
}

int authenticate(){
    /*"MAX_ATTEMPTS" times authentification request based on input "username" and "password" with exit on the third failure.*/
    int count = 0;
    while(count < MAX_ATTEMPTS){
        
        INIT_COMMUNICATION
        INIT_READING_IN_CHILD
            STRING(username);
            STRING(password);
            read(file_descriptors[0], username, SIZE);
            read(file_descriptors[0], password, SIZE);
            CHECK_AND_REMOVE_SIZE(username);
            CHECK_AND_REMOVE_SIZE(password);
        END_READING_IN_CHILD
            int unprocessed_response = verifyCredential(username, password);
            STRING(response);
            response[0] = '1'; //number of characters to be sent
            response[1] = ' ';
            response[2] = (unprocessed_response)?'1':'0';
        INIT_WRITING_IN_CHILD
            ADD_SIZE(response);
            write(file_descriptors[((type == socket_pair)?0:3)], response, SIZE);
        END_WRITING_IN_CHILD
        INIT_WRITING_IN_PARENT
            count++;
            printf(I_U_R);
            char *username = readFromStdin();
            printf(I_P_R);
            char *password = readFromStdin();
            ADD_SIZE(username);
            ADD_SIZE(password);
            write(file_descriptors[1], username, SIZE);
            write(file_descriptors[1], password, SIZE);
        END_WRITING_IN_PARENT
        INIT_READING_IN_PARENT
            STRING(response);
            read(file_descriptors[((type == socket_pair)?1:2)], response, SIZE);
            CHECK_AND_REMOVE_SIZE(response);
        END_READING_IN_PARENT
        if(response[2] == '1'){
            CHECK_AND_REMOVE_SIZE(username);
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
    /*Return in strig "stats" the required information.*/
    STRING(name);
    strcpy(name, basename(path));
    struct stat filestat;
    if (stat(path, &filestat) < 0){
        strcpy(stats, ERR_STAT);
        return;
    }
    if(strcmp(path, stats))
        memset(stats, 0, sizeof(char) * SIZE);
    else
        stats[strlen(stats)] = '\n';
    sprintf(stats + strlen(stats), "  File: %s \n", name);
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
    /*Recursive search that looks for an exact match which is than added to "matches"-a vector of strings.*/
    DIR *directory;
    struct dirent *directory_entry;
    STRING(buffer);
    if(path[strlen(path) - 1] == '/')
        path[strlen(path) - 1] = '\0';
    if(!(directory = opendir(path))){
        strcpy(matches[*match_number], ERR_OPENDIR);
        return;
    }
    while((directory_entry = readdir(directory))) 
        if(strcmp(directory_entry->d_name, ".") && strcmp(directory_entry->d_name, "..")){
                memset(buffer, 0, sizeof(char) * SIZE);
                sprintf(buffer, "%s/%s", path, directory_entry->d_name);
                //Change find to search for exact match or for part match.
                if(!strcmp(directory_entry->d_name, file))
                //if(strstr(directory_entry->d_name, file))
                    sprintf(matches[(*match_number)++], "%s", buffer);
                if(directory_entry->d_type == DT_DIR)
                    find(buffer, file, matches, match_number);
        }
    closedir(directory);
    free(buffer);
}

void findFile(char *command){
    /*Interpret and execute commands like "myfind *arg1 *arg2".*/
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
        STRING(command);
        read(file_descriptors[0], command, SIZE);
        CHECK_AND_REMOVE_SIZE(command);
    END_READING_IN_CHILD
        STRING(path);
        memcpy(path, command+strlen(MYFIND) + 1, strlen(command) - strlen(MYFIND) - 1);
        int space_location = 0;
        if(path[space_location] != ' ')
            while(path[++space_location] != ' ');
        STRING(name);
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
        // Comment the next to lines for original myfind capabilities.
        for(int i=0; i<SIZE && matches[i][0]; ++i)
            getStat(matches[i], matches[i]);
    INIT_WRITING_IN_CHILD
    for(int i=0; i<SIZE; ++i){
        ADD_SIZE(matches[i]);
        write(file_descriptors[((type == socket_pair)?0:3)], matches[i], SIZE);
    }
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        ADD_SIZE(command);
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
        int match_number = SIZE;
        char *response[SIZE];
        while(match_number--){
            response[match_number] = (char *) malloc(SIZE);
            memset(response[match_number], 0, sizeof(char) * SIZE);
        }
    for(int i=0; i<SIZE; ++i){
        read(file_descriptors[((type == socket_pair)?1:2)], response[i], SIZE);
        CHECK_AND_REMOVE_SIZE(response[i]);
    }
    END_READING_IN_PARENT
    for(int i=0; i<SIZE && response[i][0]; ++i)
        printf("%s\n", response[i]);
}

void statFile(char *command){
    /*Interpret and execute a command like "mystat *arg".*/
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
        STRING(command);
        read(file_descriptors[0], command, SIZE);
        CHECK_AND_REMOVE_SIZE(command);
    END_READING_IN_CHILD
        STRING(path);
        memcpy(path, command+strlen(MYSTAT) + 1, strlen(command) - strlen(MYSTAT) - 1);
        STRING(response);
        if(path) getStat(path, response);
    INIT_WRITING_IN_CHILD
        ADD_SIZE(response);
        write(file_descriptors[((type == socket_pair)?0:3)], response, SIZE);
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        ADD_SIZE(command);
        write(file_descriptors[1], command, SIZE);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT
        STRING(response);
        read(file_descriptors[((type == socket_pair)?1:2)], response, SIZE);
        CHECK_AND_REMOVE_SIZE(response);
    END_READING_IN_PARENT
    printf("%s", response);
}

void notFound(){
    /*Message for when the command is not recognized.*/
    printf(NOT_FOUND);
}

int stringIsCommand(char *string, char *command){
    /*Check if the given string is like "myfind *" or like "mystat *".*/
    unsigned long counter = 0;
    for(;counter<strlen(command); counter++)
        if(string[counter] != command[counter])
            return 0;
    if(string[counter] == ' ' && strlen(string) > (counter + 1))
        return 1;
    return 0;
}

int interpret(char *command, int before_login){
    /*Recognize and act according to the given command.*/
    if(!strcmp(EXIT, command))
        exitProgram();
    else if(!strcmp(command, H_CCTT)){
        printf(H_C_MSG);
        return 1;
    }
    else if(!strcmp(command, CCTT_0)){
        printf(CCTT_S, CCTT_T(type = internal_pipe));
        return 1;
    }
    else if(!strcmp(command, CCTT_1)){
        printf(CCTT_S, CCTT_T(type = external_pipe));
        return 1;
    }
    else if(!strcmp(command, CCTT_2)){
        printf(CCTT_S, CCTT_T(type = socket_pair));
        return 1;
    }
    else if(strstr(command, CCTT)){
        printf(ERR_ARGS, CCTT);
        return 1;
    }
    if(before_login){
        if(!strcmp (LOGIN, command))
            return authenticate();
        else
            notFound();
    }
    else if(stringIsCommand(command, MYFIND))
        findFile(command);
    else if(stringIsCommand(command, MYSTAT))
        statFile(command);
    else if(!strcmp(command, H_MYFIND))
        printf(H_F_MSG);
    else if(!strcmp(command, H_MYSTAT))
        printf(H_S_MSG);
    else{
        if(strstr(command, MYSTAT))
            printf(ERR_ARGS, MYSTAT);
        else if(strstr(command, MYFIND))
            printf(ERR_ARGS, MYFIND);
        else
            notFound();
    }
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
    printf(MENU);
    do{
        printf(REQUEST);
        if(command) free(command);
        command = readFromStdin();
    }while(interpret(command, 0));
}

void quitProc(){
    printf(Q_MSG);
    _exit(0);
}

int main(){
    signal(SIGQUIT, quitProc);
    menu();
    return 0;
}
