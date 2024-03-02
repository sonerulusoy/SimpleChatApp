
all: server client
server: server.cpp common.cpp
	g++ -o server server.cpp common.cpp -lpthread

client: client.cpp common.cpp
	g++ -o client client.cpp common.cpp -lpthread

.PHONY: clean

clean: 
	rm server client
