#define PORT       2908
#define IP_ADDRESS "127.0.0.1"

#define LOOP while(true)


// Server
#define ERR_S_SOCKET  "[SERVER] In function \"intitServer\": socket() failed.\n"
#define ERR_S_BIND    "[SERVER] In function \"intitServer\": bind() failed.\n"
#define ERR_S_LISTEN  "[SERVER] In function \"intitServer\": listen() failed.\n"
#define ERR_S_ACCEPT  "[SERVER] In function \"acceptClient\": accept() failed.\n"
#define ERR_S_READ    "[SERVER] In function \"processRequest\": read() failed.\n"
#define ERR_S_WRITE   "[SERVER] In function \"processRequest\": write() failed.\n"

#define ACK_S_LISTEN  "[SERVER] Listening on port %d started.\n"
#define ACK_S_ACCEPT  "[SERVER] Client \"%d\" was accepted.\n"
#define ACK_S_RECIVED "[SERVER] Request \"%s\" received.\n"
#define ACK_S_SENT    "[SERVER] Response \"%s\" sent.\n"

// Client
#define ERR_C_SOCKET  "[CLIENT] In function \"connectToServer\": socket() failed.\n"
#define ERR_C_CONNECT "[CLIENT] In function \"connectToServer\": connect() failed.\n"
#define ERR_C_READ    "[CLIENT] In function \"requestToServer\": read() failed.\n"
#define ERR_C_WRITE   "[CLIENT] In function \"requestToServer\": write() failed.\n"

#define ACK_C_CONNECT "[CLIENT] Conected to %s, on port %d.\n"
#define ACK_C_SENT    "[CLIENT] Request \"%s\" sent.\n"
#define ACK_C_RECIVED "[CLIENT] Response \"%s\" received.\n"

// Commands
#define C_EXIT        (char *) "exit\0"
#define C_EVENT       (char *) "EVENT\0"
#define C_FILES       (char *) "getFiles\0"
#define C_NULL_EVENT  (char *) "nullEvent\0"
#define C_EDIT        (char *) "edit\0"
#define C_EMPTY       (char *) "empty\0"
#define C_UPDATE      (char *) "update\0"
#define C_NEW         (char *) "newf\0"
#define C_DOWNLOAD    (char *) "download\0"
#define C_REMOVE      (char *) "rmov\0"
