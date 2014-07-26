#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <wiringPi.h>
#include "sensors/dht11.c"

#define SERVER_SOCKET "./server.sock"

#define REQUEST_DHT11 0

void connection(int sock) {
	while(1) {
		uint8_t request;
		if(read(sock, &request, 1) <= 0) break;
		char callback[51];
		uint8_t callbackIndex = 0;
		while(1) {
			if(callbackIndex >= 50 || read(sock, &callback[callbackIndex], 1) <= 0) break;
			if(callback[callbackIndex] == '\n') {
				callback[callbackIndex] = '\0';
				break;
			}
			callbackIndex++;
		}
		char response[151];
		if(request == REQUEST_DHT11) {
			struct dht11Result result = dht11(0);
			if(!result.ok) snprintf(response, 150, "\"error\": 1");
			else snprintf(response, 150, "\"temperature\": %d, \"humidity\": %d", result.temperature, result.humidity);
		} else continue;
		char json[256];
		snprintf(json, 255, "{\"callback\": %s, %s}", callback, response);
		write(sock, json, strlen(json));
	}
}

int main(int argc, char *argv[]) {
	wiringPiSetup();
	
	int server_sock, server_length;
	struct sockaddr_un server_address;
	if((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) exit(1);

	remove(SERVER_SOCKET);
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sun_family = AF_UNIX;
	strcpy(server_address.sun_path, SERVER_SOCKET);
	server_length = strlen(server_address.sun_path) + sizeof(server_address.sun_family);
	
	if(bind(server_sock, (struct sockaddr *) &server_address, server_length) < 0) exit(1); 

	listen(server_sock, 5);
	
	chmod(SERVER_SOCKET, S_IRWXU|S_IRWXG|S_IRWXO);

	struct sockaddr_un client_address;
	socklen_t client_length = sizeof(client_address);
	
	while(1) {
		int client_sock = accept(server_sock, (struct sockaddr *) &client_address, &client_length);
		if(client_sock < 0) exit(1);
		int pid = fork();
		if(pid < 0) exit(1);
		if(pid == 0) {
			close(server_sock);
			connection(client_sock);
			exit(0);
		} else close(client_sock);
	}

	close(server_sock);
	return 0;
}
