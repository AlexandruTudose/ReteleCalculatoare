#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#define SIZE 1024
#define CREDENTIALS "credentials.json"
#define MAX_ATTEMPTS 3

#define INTRO_1 "\n     WELCOME!\n This is a secured app. At this point the only two"
#define INTRO_2 "\ncommands available are \"login\" and \"exit\".\n"
#define I_U_R   " Please enter a username. The default is \"admin\".\n>"
#define I_P_R   " Please enter the password. The default is \"admin\".\n>"
#define I_ACC   "\n Nice to see you %s."
#define I_ERR   " Authentication failed."
#define I_E_T   " To many failed attempts."
#define I_EXIT  "\n Press Enter to Exit."
#define LOGIN   "login"
#define MYFIND  "myfind"
#define MYSTAT  "mystat"
#define EXIT    "exit"
#define MENU_1  "\n This is the main menu. You can choose from the fallowing options,"
#define MENU_2  "\nby writing the right command: \"myfind\", \"mystat\" or \"exit\"."
#define MENU_3  "\nWarning: \"myfind\" and \"mystat\" also require arguments. Type "
#define MENU_4  "\n\"help myfind\" or \"help mystat\" for more datails."
#define REQUEST "> "


char *readCredentials(){
    /*Put the content from credentials.json in a buffer.*/
    char *credentials = (char *) malloc (SIZE * SIZE);
    memset(credentials, 0, sizeof(char) * SIZE * SIZE);
    FILE *f = fopen(CREDENTIALS, "rb");
    int count = 0;
    while((credentials[count++] = fgetc(f)) != EOF);
    return credentials;
}

int verifyCredential(char *username, char *password){
    /*Verify the existance of the given name and password.*/
    char *current_credential = (char *) malloc (SIZE);
    memset(current_credential, 0, sizeof(char) * SIZE);
    char *credentials = readCredentials(); 
    sprintf(current_credential, "\"%s\": \"%s\"", username, password);
    if(strstr(credentials, current_credential))
        return 1;
    return 0;
}

 char *readFromStdin(){
    /*Read from stdin untill '\n' is encountered.*/

    char *buffer = (char *) malloc (SIZE);
    memset(buffer, 0, sizeof(char) * SIZE);
    int count = 0;
    while((buffer[count++] = getchar()) != '\n');
    buffer[count-1] = '\0';
    return buffer;
 }

void exitProgram(){
    //kill(getppid(), 9);
    exit(0);
}

int authenticate(){
    /// not yet
    int count = 0;
    while(count < MAX_ATTEMPTS){
        count++;
        printf(I_U_R);
        char *username = readFromStdin();
        printf(I_P_R);
        char *password = readFromStdin();
        
        //should be done in son.
        //and the true false message sent back here
        if(verifyCredential(username, password)){
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
        readFromStdin();

        //should exit from a son.
        exitProgram();
    }
    return 1;
}

void findFile(){
    //find command
}

void statFile(){
    //stat command
}


void notFound(){
    //not found
}

int interpret(char *command, int before_login){
    /*Recognize and act according to the given command.*/
    if(!strcmp(EXIT, command))
        exitProgram();
    if(before_login){
        if(!strcmp (LOGIN, command))
            return authenticate();
    }
    else{
        if(strstr(command, MYFIND))
            findFile();
        if(strstr(command, MYSTAT))
            statFile();
    }
    notFound();
    return 1;
}

void menu(){
    printf(INTRO_1);
    printf(INTRO_2);
    char *command;
    do{
        printf(REQUEST);
        command = readFromStdin();
    }while(interpret(command, 1));
    
    printf(MENU_1);
    printf(MENU_2);
    printf(MENU_3);
    printf(MENU_4);
    do{
        printf(REQUEST);
        command = readFromStdin();
    }while(interpret(command, 0));
}

int main(){
    menu();
    return 0;
}
