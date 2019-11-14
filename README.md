IOS Project
Aug-Dec 2019
Sem-5 Sec 'D'

Master-Worker Process using Shared Memory.

Problem:
Consider an application that is composed of one master process and multiple worker processes that are forked off the master 
at the start of application execution. All processes have access to a pool of shared memory pages, and have permissions to 
read and write from it. This shared memory region (also called the request buffer) is used as follows: the master process 
receives incoming requests from clients over the network, and writes the requests into the shared request buffer. The worker 
processes must read the request from the request buffer, process it, and write the response back into the same region of the buffer. 
Once the response has been generated, the server must reply back to the client. The server and worker processes are single-threaded, 
and the server uses event-driven I/O to communicate over the network with the clients (you must not make these processes multi threaded). 
You may assume that the request and the response are of the same size, and multiple such requests or responses can be accommodated in the 
request buffer. You may also assume that processing every request takes similar amount of CPU time at the worker threads. 
Using this design idea as a starting point, design the communication and synchronization mechanisms that must be used between the 
server and worker processes, in order to let the server correctly delegate requests and obtain responses from the worker processes. 
Your design must ensure that every request placed in the request buffer is processed by one and only one worker thread. 
You must also ensure that the system is efficient (e.g., no request should be kept waiting if some worker is free) and 
fair (e.g., all workers share the load almost equally). Use any IPC mechanism of your choice that works on a modern multicore system 
and implement this on Linux.

Execution Steps:

Server:
gcc server.c -o server -pthread
./server

Client:
gcc client.c -o client -pthread
./client
