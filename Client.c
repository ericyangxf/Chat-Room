//
//  Client.c
//  
//
//  Created by XueFei Yang on 2014-06-10.
//
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_RESPONSE (200)

void Auth();
void Enter();
void Say();
void Noop();
void Exit();
void Bye();
void SendMessages(char *operation, char *target, char *key, char *value);
void SendMessages2(char *operation, char *target, char *key, char *value, char *body);

int sock;
char cookie[25];
char chatRoom[20];
int status = 1;

int main(int argc, char **argv){
    struct sockaddr_in server;
    struct hostent *hp, *gethostbyname();
    char host_to_contact[50];
    int PORT;
    
    //Welcome messages
    printf("CSCI 3171 Assignment 2\nXueFei Yang B00580035\n\n");
    
    //Get host name and port number
    if(argc==5){
        if(strcmp(argv[1], "-h")==0){
            strcpy(host_to_contact, argv[2]);
        }
        else if(strcmp(argv[1], "-p")==0){
            PORT=atoi(argv[2]);
        }
        else{
            printf("This program needs two command line arguments:\n-h <host name> -p <port number>\n\n");
            exit(1);
        }
        if(strcmp(argv[3], "-h")==0){
            strcpy(host_to_contact, argv[4]);
        }
        else if(strcmp(argv[3], "-p")==0){
            PORT=atoi(argv[4]);
        }
        else{
            printf("This program needs two command line arguments:\n-h <host name> -p <port number>\n\n");
            exit(1);
        }
    }
    else{
        printf("This program needs two command line arguments:\n-h <host name> -p <port number>\n\n");
        exit(1);
    }
    
    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    server.sin_family = AF_INET;
    hp = gethostbyname(host_to_contact);
    if (hp == 0) {
        fprintf(stderr, "localhost: unknown host\n");
        exit(2);
    }
    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(PORT);
    
    // Connect to the far end server
    
    if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) {
        perror("connecting stream socket");
        exit(1);
    }
    
    printf("Chat Server Connected.\n\n");
    
    while(1){
        if(status==1){
            Auth();
        }
        else if(status==2){
            Enter();
        }
        else if(status==3){
            Say();
        }
        else if(status==4){
            Bye();
            break;
        }
    }
    
    return 0;
}

void Auth(){
    char username[20];
    char password[20];
    //Get username and password
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    char operation[] = "AUTH";
    char key[] = "Password";
    SendMessages(operation, username, key, password);
}

void Enter(){
    printf("Type \"bye\"to deregister from the server\n\n");
    printf("Chat Room: ");
    scanf("%s", chatRoom);
    char operation[] = "ENTER";
    char key[] = "Cookie";
    if(strcmp(chatRoom, "bye")!=0){
        SendMessages(operation, chatRoom, key, cookie);
    }
    else{
        status=4;
    }
}

void Say(){
    char operation[] = "SAY";
    char key[] = "Cookie";
    char Say[100];
    printf("Say: ");
    //scanf("%s", Say);
    fgets(Say, 100, stdin);
    if(Say[0]=='\n'){//avoid a bug
        fgets(Say, 100, stdin);
    }
    if(strcmp(Say, "exit\n")!=0&&strcmp(Say, "bye\n")!=0&&strcmp(Say, "noop\n")!=0&&strcmp(Say, "wrong\n")!=0){
        SendMessages2(operation, chatRoom, key, cookie, Say);
    }
    if(strcmp(Say, "exit\n")==0){
        Exit();
    }
    else if(strcmp(Say, "bye\n")==0){
        status=4;
    }
    else if(strcmp(Say, "noop\n")==0){
        Noop();
    }
    else if(strcmp(Say, "wrong\n")==0){
        SendMessages2(operation, operation, key, cookie, Say);
    }
}

void Noop(){
    char operation[] = "NOOP";
    char key[] = "Cookie";
    SendMessages(operation, operation, key, cookie);
}

void Exit(){
    char operation[] = "EXIT";
    char key[] = "Cookie";
    SendMessages(operation, chatRoom, key, cookie);
}

void Bye(){
    char operation[] = "BYE";
    char key[] = "Cookie";
    SendMessages(operation, operation, key, cookie);
}

//send message with body
void SendMessages2(char *operation, char *target, char *key, char *value, char *body){
    char buf[MAX_RESPONSE];
    strcpy(buf, operation);
    strcat(buf, " ");
    strcat(buf, target);
    strcat(buf, " 3171chat/1.0\r\n");
    strcat(buf, key);
    strcat(buf, ": ");
    strcat(buf, value);
    strcat(buf, "\r\n");
    strcat(buf, "Content-Length: ");
    int len = strlen(body)+1;
    char length[5];
    sprintf(length, "%d", len);
    strcat(buf, length);
    strcat(buf, "\r\n\r\n");
    strcat(buf, body);
    
    //Send the message
#ifdef use_send
    int bytes_sent;
#endif
    
#ifdef use_send
    bytes_sent = send(sock, buf, strlen(buf) + 1, 0);
    if (bytes_sent < 0) {
        perror("writing on stream socket");
    }
#else
    if (write(sock, buf, strlen(buf) + 1) < 0)
        perror("writing on stream socket");
#endif
    
    //Receive the message
    int rval;
    bzero( buf, sizeof(buf) );
    rval = read( sock, buf, MAX_RESPONSE );
    if (rval < 0) {
        perror("reading stream message");
    } else {
        //printf ("Server says: \"%s\"\n", buf);
    }
    
    //Read the message
    int i=0;
    while(buf[i]!=' '){
        i++;
    }
    i++;
    char responseCode[4];
    int j=0;
    while(buf[i]!=' '){
        responseCode[j]=buf[i];
        i++;
        j++;
    }
    int rCode = atoi(responseCode);
    if(rCode==200){
        if(strcmp(operation, "SAY")==0){
            printf("\nMessage Send Successful\n\n");
        }
        
    }
    else if(rCode==402){
        if(strcmp(operation, "SAY")==0){
            printf("\nNot in a Chat Room\n\n");
        }
    }
    else if(rCode==403){
        if(strcmp(operation, "SAY")==0){
            printf("\nNot Authenticated\n\n");
        }
    }
    //read message body
    if(strcmp(operation, "SAY")==0){
        i++;
        while(buf[i]!=' '){
            i++;
        }
        i++;
        while(buf[i]!='\n'){
            i++;
        }
        i++;
        while(buf[i]!='\n'){
            i++;
        }
        i++;
        char msgrev[100];
        j=0;
        while(buf[i]!='\0'){
            msgrev[j]=buf[i];
            i++;
            j++;
        }
        //print the response messages from the server
        printf("%s\n", msgrev);
    }
}

//send message without body
void SendMessages(char *operation, char *target, char *key, char *value){
    char buf[MAX_RESPONSE];
    strcpy(buf, operation);
    strcat(buf, " ");
    strcat(buf, target);
    strcat(buf, " 3171chat/1.0\r\n");
    strcat(buf, key);
    strcat(buf, ": ");
    strcat(buf, value);
    strcat(buf, "\r\n\r\n");
    
    //Send the message
#ifdef use_send
    int bytes_sent;
#endif
    
#ifdef use_send
    bytes_sent = send(sock, buf, strlen(buf) + 1, 0);
    if (bytes_sent < 0) {
        perror("writing on stream socket");
    }
#else
    if (write(sock, buf, strlen(buf) + 1) < 0)
        perror("writing on stream socket");
#endif
    
    //Receive the message
    int rval;
    bzero( buf, sizeof(buf) );
    rval = read( sock, buf, MAX_RESPONSE );
    if (rval < 0) {
        perror("reading stream message");
    } else {
        //printf ("Server says: \"%s\"\n", buf);
    }
    
    //Read the message
    int i=0;
    while(buf[i]!=' '){
        i++;
    }
    i++;
    char responseCode[4];
    int j=0;
    while(buf[i]!=' '){
        responseCode[j]=buf[i];
        i++;
        j++;
    }
    int rCode = atoi(responseCode);
    if(rCode==200){
        if(strcmp(operation, "AUTH")==0){
            printf("\nLogin Successful\n\n");
            status=2;
        }
        else if(strcmp(operation, "ENTER")==0){
            printf("\nEnter the Chat Room Successful\n\n");
            printf("Type \"exit\" to quit the chat room\nType \"bye\"to deregister from the server\n\n");
            status=3;
        }
        else if(strcmp(operation, "EXIT")==0){
            printf("\nExit the Chat Room Successful\n\n");
            status=2;
        }
    }
    else if(rCode==401){
        if(strcmp(operation, "AUTH")==0){
            printf("\nWrong Username or Password\n\n");
        }
    }
    else if(rCode==402){
        if(strcmp(operation, "ENTER")==0){
            printf("\nInvalid Chat Room\n\n");
        }
        else if(strcmp(operation, "EXIT")==0){
            printf("\nInvalid Chat Room\n\n");
        }
    }
    
    //read message body
    if(strcmp(operation, "AUTH")==0){
        i++;
        while(buf[i]!=' '){
            i++;
        }
        i++;
        j=0;
        while(buf[i]!='\r'){
            cookie[j]=buf[i];
            i++;
            j++;
        }
    }
    else if(strcmp(operation, "NOOP")==0){
        
    }
}
