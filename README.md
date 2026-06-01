# Web Server
A web server is a **virtual host** that hosts many websites, each website, because of various website types(static, dynamic, PHP, etc), needs a server for it.

- What does a runtime server do?
It listen on sockets, config the server, location matching, server name matching.

#### Port
IP tells which machine, and **port** determines which program/service.
#### Socket 
Socket is the internet fd (file descriptor) for one TCP connection between client and server.
#### Location
A location is a rule of Routing.
```
/images
/upload

```

### Mapping from port to servers
One port may correspond to multiple servers (virtual hosts). `port2servers()` function lists out all possible servers, and `findServerByHost()` pins down which server (host)

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

## Three Main Layers
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

### HTTP (hyper text transfer protocol)
- how it works
    1. a web client (browser) opens an internet connection to a server and request content
    2. the server responds with the requested  content and displays it on the screen
    3. the browser reads the content and displays it on the screen
- HTTP request
    - GET, POST, OPTIONS, HEAD, PUT, DELETE, TRACE
- HTTP responses
    - status code 200, 301, 400, 403, 404, 501, 505
## Web Server Architecture
There are several kinds of web serveer architecture:
- process-based: Github
- thread-based: LinkedIn
- event-driven: Paypal (Node.js), multi-process event-driven: Nginx
- hybrid and modern: Twitter (Event-driven + thread pool), Docker, Whatsapp, discord, Dropbox

## Network programming

- `socket()` create a socket descriptor
### for the server
- `listen()` tells the kernel that this socket is on the server side, not the client side
- `bind()` server establish a connection with client `bind(sockfd, port 8000)`
- `accept()` the server wait for the connection request from clients, it creates new **client socket**
### for the client (browser)
- `connect()` client establishes a connection with the server

- $$1  server = 1  listening socket +  many client sockets$$

- workflow:

server: `socket()` --> `bind()` --> `listen()` 
--> client: `socket()` --> `connect()` 
--> server: `accept()`
