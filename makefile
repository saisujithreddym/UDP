

#Enable g++ complier
CPP = g++
# Set Wall 
CPPFLAGS=-Wall -g 

server: server.cpp server.h 
	$(CPP) $(CPPFLAGS) server.cpp -o server -lrt
clean:
	rm -rf *o client server
