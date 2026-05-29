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
### Sockets

- `listen()` tells the kernel that this socket is on the server side, not the client side
- `socket()` create a socket descriptor
- `connect()` client establishes a connection with the server
- `accept()` the server wait for the connection request from clients
- `bind()` server establish a connection with client