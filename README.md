# Web Server
A web server is a **virtual host** that hosts many websites, because of various website types(static, PHP, autoindex, etc), each website needs a server for it. Thus a web server contains many runtime servers.

- What does a runtime server do?

- It reads the configuration and really builds the server with runtime resources, including: 
    - creating and binding listen sockets, 
    - building connections with clients, 
    - and processing HTTP requests and responding.

### Port
IP tells which machine, and **port** specifies  which program/service. 

>_"Your machine is like a building and the port is the room number"_
### Socket 
Socket is the internet fd (file descriptor) for one TCP connection between client and server.
>_"Socket is like the telephone wire for each room or each guest and TCP connection is like the phone call."_
### Location
If you see a server as a website, then a location is a rule of Routing for that website.
```
    /images
    /upload
```


### Virtual Hosting
**Virtual Hosting is a 1->N Mapping from port to servers.**

One port may correspond to multiple servers (virtual hosts). It's like one office accommodating people from different departments. `port2servers()` function lists out all possible servers, and `findServerByHost()` pins down which server and which host, for example "Host: cat.com, host: dog.com etc."

```
server {
    listen 8080;
    server_name cat.com;
}
server {
    listen 8080;
    server_name dog.com;
}
```

## The 3 Main Layers of the code
1. Config Layer
    - config
    - parser
2. Network Layer
    - Listening sockets
    - Event Loop
    - Connection management
    - Buffer management
    - Non-blocking I/O
3. HTTP Layer
    - Http Request
    - Http Response
    - CGI
    - Routing

## 1. Configuration
Config is a file of instructions that tells how a server should behave.

## 2. Network programming

- `socket()` create a socket descriptor
### the server side
- `listen()` tells the kernel that this socket is on the server side, not the client side 
    >_"Server standing by the gate and ready to welcome guests"_ 

- `bind()` server establish a connection with client `bind(sockfd, port 8000)` 
    >_"Assign a room to the guest"_

- `accept()` the server wait for the connection request from clients, it creates new **client socket**
    >_"Server opens the door"_

### the client (browser) side
- `connect()` client establishes a connection with the server 
    >_"The guest knocks the door"_

- A server serves many clients:

 $$1  \text{ server} = 1  \text{ listening socket} +  N \text{ client sockets}$$

- And all the servers listening to a port share the same listening socket, so `port2socket()` is a 1->1 mapping.

 workflow:

**server:** `socket()` --> `bind()` --> `listen()` --> 

**client:** `socket()` --> `connect()` --> 

**server:** `accept()`


```
                Port 8080
                    |
                    fd=3
             Listening Socket
                     |
                     |
       +-------------+-------------+
       |                           |
 google.com                 youtube.com
 (Runtime Server)           (Runtime Server)

       |                           |
       +-------------+-------------+
                     |
              Client Connections

             fd=7
             fd=8
             fd=9
             fd=10
```


### 3. HTTP (hyper text transfer protocol)
- how it works
    1. a web client (browser) opens an internet connection to a server and request content
    2. the server responds with the requested  content and displays it on the screen
    3. the browser reads the content and displays it on the screen
- HTTP request
    - GET, POST, OPTIONS, HEAD, PUT, DELETE, TRACE
- HTTP responses
    - status code 200, 301, 400, 403, 404, 501, 505
#### Other Web Server Architecture
There are several kinds of web serveer architecture:
- process-based: Github
- thread-based: LinkedIn
- event-driven: Paypal (Node.js), multi-process event-driven: Nginx
- hybrid and modern: Twitter (Event-driven + thread pool), Docker, Whatsapp, discord, Dropbox
