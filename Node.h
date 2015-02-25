//
//  Node.h
//  
//
//  Created by XueFei Yang on 2014-06-28.
//
//

#ifndef _Node_h
#define _Node_h

typedef struct{
    char username[20];
    char password[20];
}User;

typedef struct{ 
    char message[100];
}Message;

typedef struct{
    char roomname[20];
    void *msg_list;
}Chatroom_list;

#endif
