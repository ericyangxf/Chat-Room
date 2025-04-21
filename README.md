# Chat-Room
A simple chat room system (includes both client and server) implemented in C, using TCP connections.

# Description
The client program connects to the server program, which supports multiple chat rooms. When two or more clients join the same chat room, they can exchange messages with each other.

The server verifies the identity of each client using a username and password.

# Usage
The server uses two configuration files:
* pass-word.txt — Stores user credentials in the format: `<username>` `<password>` (one per line)
* rooms.txt — Stores chat room names, one per line, in the format: `<room name>`

To start the server, run the following command:</br>
`./Server <port number>`</br>
For example: "./server 17071" starts the server program on port 17071

To start the client, run the following command:</br>
`./Client -h <host> -p <port>`<br />
Examples:</br>
"./Client -h example.com -p 17071" starts the client on port 17071 and connect to example.com<br />
"./Client -h 127.0.0.1 -p 17071" starts the client on port 17071 and connect to 127.0.0.1<br />

# License
Chat-Room is released under the MIT License  
http://opensource.org/licenses/MIT
