// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
//#define PORT 8080

#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pthread.h>

#include <time.h>
char* server_ip;
unsigned short PORT;

int active_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char** str_split(char* a_str, const char a_delim) {
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	 knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result) {
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token) {
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

int writeDataToClient(int sckt, const void *data, int datalen) {

	const char *pdata = (const char*) data;
	while (datalen > 0) {
		int numSent = send(sckt, pdata, datalen, 0);
		if (numSent <= 0) {
			if (numSent == 0) {
				printf("The client was not written to: disconnected\n");
			} else {
				perror("The client was not written to");
			}
			return 0;
		}
		pdata += numSent;
		datalen -= numSent;
	}
	printf("here 3\n");
	send(sckt, EOF, 1024, 0);
	printf("here 4\n");
	return 1;
}

int writeStrToClient(int sckt, const char *str) {
	printf("write string to client\n");
	return writeDataToClient(sckt, str, strlen(str));
	printf("finish\n");
}

void GETRespond(char** request, int new_socket, char httpNum) {

	char* file_to_send = *(request + 1);

	long fsize;
	FILE *fp = fopen(file_to_send, "r");
	if (!fp) {
		strcat(*(request + 2), " 404 Not Found\r\n");
		writeStrToClient(new_socket, *(request + 2));
		return;
		//perror("The file was not opened");
	}

	printf("The file was opened\n");

	if (fseek(fp, 0, SEEK_END) == -1) {
		perror("The file was not seeked");
		pthread_exit(3);
	}

	fsize = ftell(fp);
	if (fsize == -1) {
		perror("The file size was not retrieved");
		pthread_exit(3);
	}
	rewind(fp);

	char *msg = (char*) malloc(fsize);
	if (!msg) {
		perror("The file buffer was not allocated\n");
		pthread_exit(3);
	}

	if (fread(msg, fsize, 1, fp) != 1) {
		perror("The file was not read\n");
		pthread_exit(3);
	}
	fclose(fp);

	printf("The file size is %ld\n", fsize);

	strcat(*(request + 2), " 200 OK\r\n");
	writeStrToClient(new_socket, *(request + 2));

	if (!writeDataToClient(new_socket, msg, fsize)) {
		close(new_socket);
	}

}

void POSTRespond(char** tokens, int sock) {

	//char data[1024] = { 0 };
	char response[1024] = { 0 };
	int valread;

	//writeStrToClient(sock, "OK\r\n");
        
	strcat(*(tokens + 2), " 200 OK\r\n");
	writeStrToClient(sock, *(tokens + 2));

	valread = read(sock, response, 1024);

	printf("response of server: %s\n", response);

	char** responseTokens = str_split(response, ' ');
	
		FILE *received_file;
		received_file = fopen(*(tokens + 1), "w");
		//valread = read( sock , response, 1024);
		while (valread) {
			fwrite(response, sizeof(char), sizeof(response), received_file);
			valread = read(sock, response, 1024);
		}

		printf("file is recieved and saved on %s\n", *(tokens + 1));
		fclose(received_file);


	close(sock);

}


int Respond(char* request, int new_socket)
{
	char** tokens;
	tokens = str_split(request, '\r');

	int httpVersion = ParseAndRespond(*(tokens));
	while(httpVersion == 1)
	{	
		tokens++;
		if(tokens)
		{
			*tokens++;
			if(tokens[0] != '\0')
				httpVersion = ParseAndRespond(*(tokens));					
			else 
				return httpVersion;
		}
		else
			return httpVersion;
	}
}


int ParseAndRespond(char* request, int new_socket) {
	char** tokens;
	tokens = str_split(request, ' ');

	if (!*tokens || !*(tokens + 1) || !*(tokens + 2) || *tokens[0] == '\0') {
		return -1;
	}

	char* method = *tokens;
	char* httpType = *(tokens + 2);
	printf("http num is %s", httpType);
	int httpNum;
	if (strcmp(httpType, "HTTP/1.0\r\n") == 0) {
		httpNum = 0;
	} else if (strcmp(httpType, "HTTP/1.1\r\n") == 0) {
		httpNum = 1;
	} else if (strcmp(httpType, "HTTP/1.1\n") == 0) {
		httpNum = 1;
	} else {
		//printf("cannot identify http version\n");
	        //	return -1;
                httpNum = 1;
	}
	if (strcmp(method, "GET") == 0) {
		printf("it is GET request\n");
		//print
		GETRespond(tokens, new_socket, httpNum);


	} else if (strcmp(method, "POST") == 0) {
		printf("it is POST request\n");
		POSTRespond(tokens, new_socket);
	} else {
		printf("cannot identify request method\n");
		return -1;
	}

	/*print splitted request*/
	/*
	 if (tokens)
	 {
	 int i;
	 for (i = 0; *(tokens + i); i++)
	 {
	 printf("%s\n", *(tokens + i));
	 free(*(tokens + i));
	 }
	 printf("\n");
	 free(tokens);
	 }
	 */
	return httpNum;

}

struct client_info {
	int sock;
	float maxSeconds;
	char ip[INET_ADDRSTRLEN];
};

void ClientRequest(int args) 
{
	struct client_info cl = *((struct client_info *)args);
	int sock = cl.sock;

	printf("Thread started with socket number %d\n", sock);
	int valread;
	char request[1024] = { 0 };

	clock_t startTimer = clock();

	// LINUX timeout
	struct timeval tv;
	tv.tv_sec = cl.maxSeconds;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	while((float)(clock() - startTimer) / CLOCKS_PER_SEC < cl.maxSeconds)
	{
		valread = read(sock, request, 1024);
		if(valread == 0)
			continue;
		printf("HTTP request message:\n%s\n", request);
		if (request[0] != '\0') {
			int httpNum = ParseAndRespond(request, sock);
			printf("Response sent\n");
			if (httpNum == 0) {
				break;
			} else if (httpNum == -1) {
				writeStrToClient(sock, "Error Recognizing request");
				break;
			}
		} else
			break;	
	}
	
	close(sock);
	//close sock and return 
	pthread_mutex_lock(&mutex);
	printf("%d disconnected\n", sock);
	active_clients--;
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char const *argv[]) {



	if(argc < 2)
		PORT = 8080;
	else 
		PORT = atoi( argv[1] );
	 
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char *response = "server response";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
			sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("server is ready\n");
	pthread_t recvt;
	while (1) {

		if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
				(socklen_t*) &addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		pthread_mutex_lock(&mutex);
		active_clients++;
	
		struct client_info *args = malloc(sizeof *args);
		args->sock = new_socket;
		args->maxSeconds = 10 / (active_clients);
		pthread_create(&recvt,NULL,ClientRequest, args);
	
		pthread_mutex_unlock(&mutex);

	}
	return 0;
}

