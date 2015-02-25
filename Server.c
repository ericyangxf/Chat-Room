//
//  Server.c
//  
//
//  Created by XueFei Yang on 2014-06-27.
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
#include <pthread.h>
#include <signal.h>
#include "list.h"
#include "Node.h"

#define MAX_BUFFER (200)

static int sock;
List_t Userinfo;
List_t Chatrooms;

void sigint_handler(int sig_number);
void *handle_client(void *arg);
void init_userinfo();
void init_rooms();
void SendMessage(int msgsock, char *responsecode, char *version, char *key, char *value);
void SendMessage2(int msgsock, char *responsecode, char *version);
void SendMessage3(int msgsock, char *responsecode, char *version, char *key, char *value,int length, char *body);
int string_length(char *string);
int check_rooms(char *roomname2);


int main(int argc, char **argv){
    int PORT;
    List_init(&Userinfo);
    List_init(&Chatrooms);
    init_userinfo();
    init_rooms();
    
    //Get port number
    if(argc==2){
        PORT=atoi(argv[1]);
    }
    else{
        exit(1);
    }
    
    struct sockaddr_in server;
    int msgsock;
    int val;
    pthread_t proc;
    pthread_attr_t attrib;
    
    /* Set up default behaviour for the threads. */
    
    pthread_attr_init( &attrib );
    pthread_attr_setscope( &attrib, PTHREAD_SCOPE_SYSTEM );
    
    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    
    /* Name socket using wildcards. */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    if (bind(sock, (struct sockaddr *) &server, sizeof(server))) {
        perror("binding stream socket");
        exit(1);
    }
    
    /* Allow the socket to be re-used immediately once the server ends. */
    val = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(val));
    
    
    /* Start accepting connections */
    listen(sock, 5);
    
    /* Give myself a way out. */
    
    signal( SIGINT, sigint_handler );
    
    /* Now wait for messages. */
    
    do {
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
            perror("accept");
        else {
            pthread_create(&proc, &attrib, handle_client, (void *)msgsock);
        }
        
    } while (1);
    
    close( sock );
    return 0;
}

void sigint_handler( int sig_number ){
    close( sock );
    
    exit(0);
}

void * handle_client( void *arg ){
    int msgsock = (int) arg;
    int rval = 0;
    char buf[MAX_BUFFER];
    int status = 1;//1=AUTH 2=ENTER 3=SAY
    User *user;
    List_node_t *temp;
    int msg_id = 0;
    
    /* Get message from the client*/
    char username[20];
    char cookie[23];//cookie send to Client in AUTH operation
    char cookie2[23];//cookie from ENTER, SAY, EXIT, BYE, NOOP operation
    char version[20];
    char password[20];
    char operation[20];
    char chatroom[20];//chatroom name in ENTER operation
    char chatroom2[20];//chatroom name in SAY operation
    char key[20];
    char value[20];
    char say[100];

    while (read( msgsock, buf, MAX_BUFFER ) > 0) {
        fprintf(stderr, "%s\n", buf);
        //Read the message
        int i=0;
        int j=0;
        while(buf[i]!=' '){
            operation[j]=buf[i];
            i++;
            j++;
        }
        operation[j]='\0';
        //EXIT
        if(strcmp(operation, "EXIT")==0){
            if(strcmp(cookie, cookie2)!=0){
                char rcode[] = "403";
                SendMessage2(msgsock, rcode, version);
                continue;
            }
            else if(strcmp(chatroom, chatroom2)!=0){
                char rcode[] = "402";
                SendMessage2(msgsock, rcode, version);
                continue;
            }
            else{
                status=2;
                char rcode[] = "200";
                SendMessage2(msgsock, rcode, version);
                continue;
            }
        }
        
        //BYE
        else if(strcmp(operation, "BYE")==0){
            if(strcmp(cookie, cookie2)!=0){
                char rcode[] = "403";
                SendMessage2(msgsock, rcode, version);
                continue;
            }
            else{
                char rcode[] = "200";
                SendMessage2(msgsock, rcode, version);
                break;
            }
        }

        i++;
        j=0;
        //read the target
        while(buf[i]!=' '){
            if(strcmp(operation, "AUTH")==0){
                username[j]=buf[i];
            }
            else if(strcmp(operation, "ENTER")==0){
                chatroom[j]=buf[i];
            }
            else if(strcmp(operation, "SAY")==0){
                chatroom2[j]=buf[i];
            }
            i++;
            j++;
        }
        if(strcmp(operation, "AUTH")==0){
            username[j]='\0';
        }
        else if(strcmp(operation, "ENTER")==0){
            chatroom[j]='\0';
        }
        else if(strcmp(operation, "SAY")==0){
            chatroom2[j]='\0';
        }
        i++;
        j=0;
        //read the version
        while(buf[i]!='\r'){
            if(strcmp(operation, "AUTH")==0){
                version[j]=buf[i];
            }
            i++;
            j++;
        }
        if(strcmp(operation, "AUTH")==0){
            version[j]='\0';
        }
        i+=2;
        j=0;
        //read the key
        while(buf[i]!=':'){
            key[j]=buf[i];
            i++;
            j++;
        }
        key[j]='\0';
        i++;
        i++;
        j=0;
        //read the value
        while(buf[i]!='\r'){
            if(strcmp(operation, "AUTH")==0&&strcmp(key, "Password")==0){
                password[j]=buf[i];
            }
            else if(strcmp(key, "Cookie")==0){
                cookie2[j]=buf[i];
            }
            i++;
            j++;
        }
        if(strcmp(operation, "AUTH")==0&&strcmp(key, "Password")==0){
            password[j]='\0';
        }
        else if(strcmp(key, "Cookie")==0){
            cookie2[j]='\0';
        }
        i+=2;
        if(strcmp(operation, "SAY")==0){
            while(buf[i]!='\r'){
                i++;
            }
            //read the body message
            i+=4;
            j=0;
            while(buf[i]!=0){
                say[j]=buf[i];
                i++;
                j++;
            }
            say[j]='\0';
        }

        //AUTH
        if(strcmp(operation, "AUTH")==0){
            while(List_next_node(&Userinfo, &temp, &user)){
                if(strcmp(user->username, username)==0&&strcmp(user->password, password)==0){
                    status=2;
                    char rcode[] = "200";
                    char key[] = "Set-Cookie";
                    strcat(cookie, "foo");
                    strcat(cookie, username);
                    SendMessage(msgsock, rcode, version, key, cookie);
                    break;
                }
            }
            if(status==1){
                char rcode[] = "401";
                SendMessage2(msgsock, rcode, version);
            }
        }
        
        //ENTER
        else if(strcmp(operation, "ENTER")==0){
            if(strcmp(cookie, cookie2)!=0){
                char rcode[] = "403";
                SendMessage2(msgsock, rcode, version);
            }
            else if(check_rooms(chatroom)==1){
                status=3;
                char rcode[] = "200";
                SendMessage2(msgsock, rcode, version);
            }
            else{
                char rcode[] = "402";
                SendMessage2(msgsock, rcode, version);
            }
        }
        
        //SAY
        else if(strcmp(operation, "SAY")==0){
            if(strcmp(cookie, cookie2)!=0){
                char rcode[] = "403";
                SendMessage2(msgsock, rcode, version);
            }
            else if(strcmp(chatroom, chatroom2)!=0){
                char rcode[] = "402";
                SendMessage2(msgsock, rcode, version);
            }
            else{
                //add new message to the chat room
                Chatroom_list *room;
                List_node_t *temp = NULL;
                
                while(List_next_node(&Chatrooms, &temp, &room)){
                    if(strcmp(room->roomname, chatroom2)==0){
                        break;
                    }
                }
                
                char *new_msg = (char *)malloc(100*(sizeof(char)));
                strcpy(new_msg, username);
                strcat(new_msg, ": ");
                strcat(new_msg, say);
                strcat(new_msg, "\n");
                List_add_tail(room->msg_list, new_msg);

                //send message to the client
                char *body = (char *)malloc(100*(sizeof(char)));
                List_node_t *temp2 = NULL;
                char *msg_temp;
                
                while(string_length(body)<90&&List_next_node(room->msg_list, &temp2, &msg_temp)){
                    strcat(body, msg_temp);
                }
                char rcode[] = "200";
                SendMessage3(msgsock, rcode, version, key, cookie, string_length(body), body);
            }
        }
    }
    
    close( msgsock );
    
    return NULL;
}

//send message without body
void SendMessage(int msgsock, char *responsecode, char *version, char *key, char *value){
    char buf[MAX_BUFFER];
    strcpy(buf, version);
    strcat(buf, " ");
    strcat(buf, responsecode);
    strcat(buf, " OK\r\n");
    strcat(buf, key);
    strcat(buf, ": ");
    strcat(buf, value);
    strcat(buf, "\r\n\r\n");
    
    if (write(msgsock, buf, strlen(buf) + 1) < 0) {
        perror("writing on stream socket");
    }
}

//send message without body and header line
void SendMessage2(int msgsock, char *responsecode, char *version){
    char buf[MAX_BUFFER];
    strcpy(buf, version);
    strcat(buf, " ");
    strcat(buf, responsecode);
    strcat(buf, " OK\r\n");
    strcat(buf, "\f\n\f\n");
    
    if (write(msgsock, buf, strlen(buf) + 1) < 0) {
        perror("writing on stream socket");
    }

}

//send message with body and header line and length of body message
void SendMessage3(int msgsock, char *responsecode, char *version, char *key, char *value, int len, char *body){
    char buf[MAX_BUFFER];
    strcpy(buf, version);
    strcat(buf, " ");
    strcat(buf, responsecode);
    strcat(buf, " OK\r\n");
    strcat(buf, key);
    strcat(buf, ": ");
    strcat(buf, value);
    strcat(buf, "\r\nContent-Length: ");
    char length[5];
    sprintf(length, "%d", len);
    strcat(buf, length);
    strcat(buf, "\r\n\r\n");
    strcat(buf, body);
    
    if (write(msgsock, buf, strlen(buf) + 1) < 0) {
        perror("writing on stream socket");
    }
}

//read pass-word.txt
void init_userinfo(){
    FILE * fp = fopen("pass-words.txt", "r");
    if (fp == NULL)
	{
		fprintf(stderr, "Error: cannot open pass-words.txt");
		exit(0);
	}
    
    char ch;
    while((ch=fgetc(fp))!=EOF){
        if(ch=='\n'||ch==' '){
            break;
        }
        char username[20];
        char password[20];
        int i=0;
        while(ch!=' '){
            username[i]=ch;
            i++;
            ch=fgetc(fp);
        }
        username[i]='\0';
        ch=fgetc(fp);
        i=0;
        while(ch!='\n'&&ch!=EOF){
            password[i]=ch;
            i++;
            ch=fgetc(fp);
        }
        password[i]='\0';
        User *newuser = (User *)malloc(sizeof(User));
        strcpy(newuser->username, username);
        strcpy(newuser->password, password);
        List_add_tail(&Userinfo, newuser);
    }
    fclose(fp);
}

//read rooms.txt
void init_rooms(){
    FILE * fp = fopen("rooms.txt", "r");
    if (fp == NULL)
	{
		fprintf(stderr, "Error: cannot open rooms.txt");
		exit(0);
	}
    
    char ch;
    char chatroom_name[20];
    while((ch=fgetc(fp))!=EOF){
        if(ch=='\n'||ch==' '){
            break;
        }
        int i=0;
        while(ch!='\n'){
            chatroom_name[i]=ch;
            i++;
            ch=fgetc(fp);
        }
        chatroom_name[i]='\0';
    
    Chatroom_list *newroom = (Chatroom_list *)malloc(sizeof(Chatroom_list));
    strcpy(newroom->roomname, chatroom_name);
    List_t *msg_list = (List_t *)malloc(sizeof(List_t));
    List_init(msg_list);
    newroom->msg_list = msg_list;
    List_add_tail(&Chatrooms, newroom);
    }
    
    fclose(fp);
}

//check the chatroom exist or not
int check_rooms(char *roomname2){
    Chatroom_list *room;
    List_node_t *temp = NULL;
    
    while(List_next_node(&Chatrooms, &temp, &room)){
        if(strcmp(room->roomname, roomname2)==0){
            return 1;
        }
    }
    
    return 0;
}

//calculate string length
int string_length(char *string){
    int i=0;
    while(string[i]!=0){
        i++;
    }
    i++;
    return i;
}


