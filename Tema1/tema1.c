#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#define SIZE 1024
#define CREDENTIALS "credentials.json"
#define MAX_ATTEMPTS 3

#define INTRO "\n     WELCOME!\n In order to continue you have to authenticate.\n"
#define I_U_R " Please enter a username. The default is \"admin\".\n>"
#define I_P_R " Please enter the password. The default is \"admin\".\n>"
#define I_ACC "\n Nice to see you %s."
#define I_ERR "\n Authentication failed."
#define I_E_T "To many failed attempts."
#define EXIT  "\n Press \"%s\" to Exit."
#define ENTER "Enter"


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

void authenticate(){
    printf(INTRO);

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
        }
        else{
            printf("%s", I_ERR);
        }
    }
    if(count == MAX_ATTEMPTS){
        printf("%s", I_E_T);
        printf(EXIT, ENTER);
        readFromStdin();

        //should exit from a son.
        exitProgram();
    }
}

int main(){
    authenticate();
    
    return 0;
}
