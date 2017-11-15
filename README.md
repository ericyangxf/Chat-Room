#Chat-Room
A simple chat room system (includes client and server) using TCP connection in C language.

# Description
The client program can connect with server program. There are multiple chat rooms on the server. If there are two or more client in the same chatroom, they can send message each other.

The server can verify the identity of each client with username and password. 

# Usage
The server has two config files:<br />
pass-word.txt store user information with "&lt;username> &lt;password>" format in each line.<br />
rooms.txt store chatroom information with "&lt;room name>" format in each line.

To run the Server program, please type "./Server <port number>"<br />
For example: "./server 17071" will start the server program on port 17071

To run the Client program, please type "./Client -h <host> -p <port>"<br />
For example: "./Client -h example.com -p 17071" will start the client on port 17071 and connect to example.com<br />
For example: "./Client -h 127.0.0.1 -p 17071" will start the client on port 17071 and connect to 127.0.0.1<br />

# Compile the source code
A Makefile for compile is provided.<br />

# License
Chat-Room is released under the MIT License  
http://opensource.org/licenses/MIT
