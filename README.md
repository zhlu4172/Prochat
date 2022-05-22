# Prochat
In this project, I developed a localised chat server that will support a number of clients through named pipes (FIFOs). The server and client applications will communicate through a fixed sized binary protocol. The server will manage a global named pipe for establishing connections, after a connection has been established it will construct a separate read and write named pipes for the client to utilise. The client will communicate to the server over the named pipes. The server can be able to read from all clients asynchronously.
