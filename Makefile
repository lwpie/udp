.phony: all clean

all: udp_server udp_client

udp_server: udp_server.cpp udp_pack.hpp
	g++ -o udp_server udp_server.cpp -lboost_system -lboost_thread -lpthread -lboost_serialization -O3

udp_client: udp_client.cpp udp_pack.hpp
	g++ -o udp_client udp_client.cpp -lboost_system -lboost_thread -lpthread -lboost_serialization -O3

clean:
	rm -f udp_server udp_client
