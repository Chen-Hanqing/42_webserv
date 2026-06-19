# Web Server
A WebServer is the actual HTTP server process that receives and handles HTTP requests. It owns the listening sockets, accepts client connections, and routes requests to the appropriate runtime server.

Each ServerBlock is essentially a website configuration, it serves a particular website.

A web server hosts multiple virtual hosts. Each runtime server represents one virtual host and is selected according to the listening port and Host Header of the incoming request.

- What is a runtime server?
- It is the runtime representation of a server block (virtual host).
- Why virtual host?
- Virtual Hosting enables one `IP:Port` serves many websites at the same time.



```
                WebServer
                     |
     +---------------+---------------+
     |                               |
 listen socket                 event loop
     |
     |
  port 8080
     |
     +-----------------------+
     |                       |
 Virtual Host A       Virtual Host B
 (cat.com)            (dog.com)
    
```
### The workflow of a HTTP webserver:
* step 1. find the listening socket and mapping port
* step 2. check the list of servers that listen on the port `port2servers[8080]`
* step 3. check the Host Header to find the server block
* step 4. check the location, which location matches URI?
* step 5. get the requested file `./dog/index.html`
* step 6. return `HTTP/1.1 200 OK`

### Port
IP tells which machine, and **port** specifies  which program/service. 
>_"Your machine is like a building and the port is the room number"_
### Socket 
Socket is the internet fd (file descriptor) used for internet communication. Listening socket wait for incoming connections. Client sockets represent established TCP connections between client and server.
>_"Socket is like the telephone wire for each room or each guest and TCP connection is like the phone call."_

### Routing
Routing is the decision of who solves the request. Host Routing finds the serverblock, and Location Routing decides how to handle the request.

### Location
A location is a routing rule inside a virtual host, it matches a URL path and determines how the request should be handled.
```
    /images
    /upload
```


### Virtual Hosting
**One listening port may host multiple websites.**
>_'It's like one office accommodating people from different departments.'_

`port2servers()` function lists out all possible servers, and `hostRouting()` pins down which server and which host, for example "Host: cat.com, host: dog.com etc."

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

## The 4 Main Layers of the code
1. Config Layer
    - config
    - parser
2. Network Layer
    - Listening sockets `socket(), bind(), listen(), connect(), accept()`
    - Event Loop
    - Connection management
    - Buffer management
    - Non-blocking I/O `select()`
3. HTTP Layer
    - Http Request
    - Http Response
    - CGI Handler
    - Routing
4. CGI
    - fork()
    - execve()

## 1. Configuration
Config is a file of instructions that tells how a server should behave.

## 2. Network programming

- `socket()` create a socket descriptor
### the server side

- `bind()` establishes the pairing conenction of a listen socket and a port: `bind(listen sockfd, port 8000)` 
    >_"Register the hotel address as room 8080 (port 8080) in the building"_

- `listen()` tells the kernel that this socket is on the server side, not the client side 
    >_"Server standing by the gate and ready to welcome guests"_ 


- `accept()` the server wait for the connection request from clients, it creates new **client socket**
    >_"Server opens the door"_

### the client (browser) side
- `connect()` client establishes a connection with the server 
    >_"The guest knocks the door"_

- A server serves many clients:
 $$ 1 \text{ listening socket} =  N \text{ client sockets}$$

- A server block can listen on multiple ports.
```
server {
    listen 80;
    listen 443;

    server_name cat.com;
}
``` 
means that cat.com can be reached via cat.com:80 and cat.com:443, HTTP -> port 80 and HTTPS -> port 443

- And all the servers listening to a port share the same listening socket, so `port2socket()` is a 1->1 mapping.

$$ 1 \text{ port} = 1 \text{ listening socket}$$

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
 (Server Block)           (Server Block)

       |                           |
       +-------------+-------------+
                     |
              Client Connections

             fd=7
             fd=8
             fd=9
             fd=10
```
### Multiplexing
The problem: blocking Input/Output when multiple requests arrive simultaneously.
- `send()`
- `recv()`
- `select()` asks the Operating System which fds are ready to use.
> _"The web server is like a state machine that manages tons of fds."_

## 3. HTTP (hyper text transfer protocol)
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

```
Webserv
    = socket + poll/select
    + HTTP parser
    + Config parser
    + Find Server Block 
    + Find Location 
    + CGI
    + get Response 
```

## usage
curl is a HTTP client endpoint
`curl http://localhost:8080` actually sends the following request to the webserver:
```
GET / HTTP/1.1
Host: localhost:8080
User-Agent: curl/8.5.0
Accept: */*
```

check the full response
`curl -i http://localhost:8080`
check the request head and the response head
`curl -v http://localhost:8080`

nc for netcat opens a TCP connection
handwrite the HTTP request
```nc localhost 8080
GET /hello.html HTTP/1.1
Host: localhost```
