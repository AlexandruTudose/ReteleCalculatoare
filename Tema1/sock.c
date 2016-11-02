#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdarg.h>

#define FIFO_1 "fifo1"
#define FIFO_2 "fifo2"

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

void initializeCommunication(enum communication_type type){

//    for(int counter = 0; counter < number_of_child_reads; counter++)

    char *buffer = (char *) malloc(1024);
    memset(buffer,0,1024);
    strcpy(buffer, "ana are mere rosii verzi si albe" );
    char *buffer2 = (char *) malloc(1024);
    memset(buffer2,0,1024);
    char *buffer3 = (char *) malloc(1024);
    memset(buffer3,0,1024);
    printf("before: %s\n", buffer);
    printf("before: %s\n", buffer2);
    printf("before: %s\n", buffer3);
    
    INIT_COMMUNICATION
    INIT_READING_IN_CHILD
        read(file_descriptors[0], buffer2, 1024);
    END_READING_IN_CHILD
        //edit
        buffer2[1]='x';
        buffer2[20]='y';
    INIT_WRITING_IN_CHILD
        write(file_descriptors[((type == socket_pair)?0:3)], buffer2, 1024);
    END_WRITING_IN_CHILD
    INIT_WRITING_IN_PARENT
        write(file_descriptors[1], buffer, 1024);
    END_WRITING_IN_PARENT
    INIT_READING_IN_PARENT 
        read(file_descriptors[((type == socket_pair)?1:2)], buffer3, 1024);
    END_READING_IN_PARENT

    printf("after: %s\n", buffer);
    printf("after: %s\n", buffer2);
    printf("after: %s\n", buffer3);
}



int main(){
    
    initializeCommunication(internal_pipe);
    initializeCommunication(external_pipe);
    initializeCommunication(socket_pair);
    return 0;
}
