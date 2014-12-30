ECEN602 HW2 Programming Assignment (TFTP protocol implementation)
-----------------------------------------------------------------

Team Number: 17
Member 1 # MANKALA, SAI SUJITH REDDY (UIN: 224002333)
Member 2 # PENG, XUEWEI (UIN: 824000328)
---------------------------------------

Description/Comments:
--------------------
1. This package can facilitate a file transfer between a server and a client using Trivial File Transfer Protocol.
This package contains three files Server.cpp server.h and makefile. Server files contains the main code and all the header files and function declaration are done in server.h.     	
To generate object files, use: "make -f makefile" in the path of these files in a linux environment.

2. To use the service compile the code using makefile and run it using the unix command /server SERVER_IP SERVER_PORT 

3. The client sends the read request RRQ to the server.The Server reads the request and sends the requested data accordingly to the    client.Client then sends the acknowledgement to the server.server upon recieving the ack, will the send the next block of data.Here each block contains 512 bytes of data.



Unix command for starting server:
------------------------------------------
./server SERVER_IP SERVER_PORT

