Building a non-blocking web server.
Blocking occurs when there are simultaneous connections. (Multiple, concurrent connections to the server)
Solution: `select` `poll` `epoll`
