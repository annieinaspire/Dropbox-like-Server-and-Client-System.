# Network-Programming

*** Chat Room ***

+ Overview

Wrote two programs, a client and a server, to build a chat room system. 
Clients talk to one another via connecting to the server. 
The server accepts connections from clients and sends commands to clients.

+ Use select() for constructing the client and server programs.

+ Specification of server

A server can serve multiple clients simultaneously. 
After a connection is built, server will send a hello message to the client, and give the client a username 'anonymous'. 
Clients can send different commands to server.

*** Non-Blocking File Transfer ***

+ The concept is just like dropbox

1. An user can save his files on the server.
2. The clients of the user are running on different hosts.
3. When any of client of the user upload a file, the server have to transmit it to all the other clients of the user.
4. When a new client of the user connects to the server, the server should transmit all the files, which are uploaded by the other clients of the user, to the new client immediately.
5. We will type /put fileXXX on different clients at the same time, your programs have to deal with this case.
6. If one of the client is sleeping, server has to send the data to the other clients of the user in a non-blocking way.
7. The uploading data only need to send to the clients which are belong to the user.
