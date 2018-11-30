// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
//#define PORT 8080

#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define LENGTH 512

char* server_ip;
unsigned short PORT;


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

int get_ip(char * hostname, char* ip) {
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	if ((he = gethostbyname(hostname)) == NULL) {
		herror("gethostbyname");
		return 1;
	}
	addr_list = (struct in_addr **) he->h_addr_list;
	for (i = 0; addr_list[i] != NULL; i++) {
		strcpy(ip, inet_ntoa(*addr_list[i]));
		return 0;
	}
	return 1;
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

	return 1;
}

int writeStrToClient(int sckt, const char *str) {
	return writeDataToClient(sckt, str, strlen(str));
}


void recieveData(int sock, char* file_name)
{

	char response[1024] = { 0 };
	int valread;
	valread = read(sock, response, 1024);
	printf("response of server: %s\n", response);

	char** responseTokens = str_split(response, ' ');
	if (!*(responseTokens) || !*(responseTokens + 1)) {
		printf("server response not recognized\n");
		return;
	}

	if (strcmp(*(responseTokens + 1), "404") == 0) {
		printf("File %s not found on server\n", file_name);
	} else if (strcmp(*(responseTokens + 1), "200") == 0) {
		
		int rcv_bytes = 0;

		FILE *received_file;
		received_file = fopen(file_name, "w");

		char *rcv_buffer;
		rcv_buffer = (char *) malloc(LENGTH*sizeof(char));

		memset(rcv_buffer,'0',sizeof(rcv_buffer));

		while((rcv_bytes = read(sock, rcv_buffer, LENGTH)) > 0)
		{
			 
			if((fwrite(rcv_buffer, sizeof(char), rcv_bytes, received_file)) < rcv_bytes)
			{
				perror("File write failed! ");
				exit(EXIT_FAILURE);
			}

			memset(rcv_buffer,'0',sizeof(rcv_buffer));

			if (rcv_bytes != LENGTH)
			{
				break;
			}
		}
		//fclose(received_file);

		if(rcv_bytes < 0 && errno == EAGAIN)
		{
			printf("Connection time out!\n");		
		}

		else if(rcv_bytes < 0 && errno != EAGAIN)
		{
			fprintf(stderr, "recv() failed due to errno = %d\n", errno);
		}
		else
		{	
			free(rcv_buffer);
			printf("File recieved successfully!\n");
			//fclose(received_file); //by3ml error hna
		}

	} else {
		printf("server response not recognized\n");
	}

	printf("return from recieveData\n");

}



/*
void recieveData(int sock, char* file_name)
{

	char response[1024] = { 0 };
	int valread;
	valread = read(sock, response, 1024);
	printf("response of server: %s\n", response);

	char** responseTokens = str_split(response, ' ');
	if (!*(responseTokens) || !*(responseTokens + 1)) {
		printf("server response not recognized\n");
		return;
	}

	if (strcmp(*(responseTokens + 1), "404") == 0) {
		printf("File %s not found on server\n", file_name);
	} else if (strcmp(*(responseTokens + 1), "200") == 0) {
		FILE *received_file;
		received_file = fopen(file_name, "w");
		valread = read( sock , response, 1024);
		printf("recieved %d\n%s", valread, response);
		while (valread) {
			printf("valread\n");
			if(response[0] == EOF)
				return;

			printf("inside loop\n");
			fwrite(response, sizeof(char), sizeof(response), received_file);
			valread = read(sock, response, 1024);
		}

		printf("file is recieved and saved on %s\n", file_name);
		fclose(received_file);
	} else {
		printf("server response not recognized\n");
	}

}
*/

int isStringEmpty(char* line) {
	if (!line || line[0] == '\0')
		return 1;
	return 0;
}

int setupSocket() {

	//setting up socket
	struct sockaddr_in address;
	int sock = 0;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
		//return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	/*
	char *hostname ="localhost";
	char ip[100];
	get_ip(hostname, ip);
	char* server_ip =ip;
	printf("%s", server_ip);
	printf("\n");
        */
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed \n");
		exit(EXIT_FAILURE);
		//return -1;
	}
	return sock;
}

void POSTCommand(char** request, int sock) {
	strcat(*request, " ");
	strcat(*request, *(request + 1));
	strcat(*request, " HTTP/1.0\r\n");
	printf("%s\n", *request);

	send(sock, *request, strlen(*request), 0);
	printf("request sent\n");

	char response[1024] = { 0 };
	
    int valread;

	valread = read(sock, response, 1024);
	printf("response of server: %s\n", response);
	char* file_to_send = *(request + 1);

	long fsize;
	FILE *fp = fopen(file_to_send, "r");

	printf("The file was opened\n");

	if (fseek(fp, 0, SEEK_END) == -1) {
		perror("The file was not seeked");
		exit(1);
	}

	fsize = ftell(fp);
	if (fsize == -1) {
		perror("The file size was not retrieved");
		exit(1);
	}
	rewind(fp);

	char *msg = (char*) malloc(fsize);
	printf("%s",msg);
	if (!msg) {
		perror("The file buffer was not allocated\n");
		exit(1);
	}

	if (fread(msg, fsize, 1, fp) != 1) {
		perror("The file was not read\n");
		exit(1);
	}
	fclose(fp);

	printf("The file size is %ld\n", fsize);

	//strcat(*(request + 2), " 200 OK\r\n");
	// writeStrToClient(new_socket, *(request + 2));

	writeDataToClient(sock, msg, fsize);	
}

void GETCommand(char** tokens, int sock, int HTTPversion) {
	strcat(*tokens, " ");
	strcat(*tokens, *(tokens + 1));
	if(HTTPversion == 0)
		strcat(*tokens, " HTTP/1.0\r\n");
	else
		strcat(*tokens, " HTTP/1.1\r\n");
	printf("%s\n", *tokens);

	send(sock, *tokens, strlen(*tokens), 0);
	printf("request sent\n");
}

void sendRequest(char* request) {

	char** tokens = str_split(request, ' ');

	if (!*tokens || !*(tokens + 1) || !*(tokens + 2)) {
		printf("Command not recognized\n");
		return;
	}

	if (strcmp(*tokens, "GET") == 0) {
		printf("it is get command\n");
		int sock = setupSocket();

		GETCommand(tokens, sock, 0);
		recieveData(sock, *(tokens + 1));

		close(sock);
	} else if (strcmp(*tokens, "POST") == 0) {
		printf("it is post command\n");
		int sock = setupSocket();
        POSTCommand(tokens, sock);
        close(sock);
	} else {
		printf("Command not recognized\n");
	}

	/*
	 send(sock , request , strlen(request) , 0 );
	 printf("request sent\n");
	 valread = read( sock , response, 1024);

	 FILE *received_file;
	 received_file = fopen("outfile.html", "w");
	 fwrite(response, sizeof(char), sizeof(response), received_file);
	 fclose(received_file);

	 printf("%s\n",response );
	 close(sock);
	 */
}

char* sendPerRequest(char* request, int sock) {

	char** tokens = str_split(request, ' ');

	if (!*tokens || !*(tokens + 1) || !*(tokens + 2)) {
		printf("Command not recognized\n");
		return;
	}

	if (strcmp(*tokens, "GET") == 0) {
		printf("it is get command\n");
		GETCommand(tokens, sock, 1);
		return *(tokens + 1);
	} else {
		printf("Command not recognized\n");
	}
}

void excute(char** commands, int counter, int sock)
{
	printf("excute: %d\n", counter);

	for(int i = 0; i < counter; i++)
	{
		printf("excute %s\n", *(commands + i));
		strcpy(*(commands + i), sendPerRequest(*(commands + i), sock));
	}
	
	for(int i = 0; i < counter; i++)
	{
		printf("recieve %s\n", *(commands + i));
		recieveData(sock, *(commands + i));
	}
}

void PersRequest()
{
	int maxCommands = 10;
	
	char **commands;
	commands = malloc(maxCommands * sizeof(char*));

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("clientRequests.txt", "r");

	if (fp == NULL) {
		printf("Cannot find input file");
		exit(EXIT_FAILURE);
	}

	int counter = 0;

	int sock = setupSocket();

	while ((read = getline(&line, &len, fp)) != -1) {

		commands[counter] = malloc((sizeof(line)+1) * sizeof(char));
		strcpy(commands[counter], line);
		counter ++;
 
		char** tokens = str_split(line, ' ');

		if (!*tokens || !*(tokens + 1) || !*(tokens + 2)) {
			printf("Command not recognized\n");
			return;
		}

		if (strcmp(*tokens, "GET") == 0);
		else if (strcmp(*tokens, "POST") == 0)
		{
	        excute(commands, counter - 1, sock);
	        POSTCommand(commands[counter], sock);
			counter = 0;
		}
		else {
			printf("Command not recognized\n");
			return;
		}

		if(counter == maxCommands)
		{
			excute(commands, counter, sock);
			counter = 0;
		}
	}

	if(counter > 0)
	{
		excute(commands, counter, sock);
	}

	fclose(fp);

	if (line)
		free(line);

	close(sock);

}

void NonPersRequest()
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("clientRequests.txt", "r");

	if (fp == NULL) {
		printf("Cannot find input file");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		//printf("Retrieved line of length %zu :\n", read);
		printf("send Request: %s ", line);
		sendRequest(line);
	}

	fclose(fp);

	if (line)
		free(line);
}


int main(int argc, char const *argv[]) {

	if(argc < 3) 
	{
		printf("error: parameters needed\n");
		exit(EXIT_FAILURE);
	}
	server_ip =  argv[1] ;
    PORT = atoi( argv[2] );
    int HTTPversion =  atoi(argv[3]);


    if(HTTPversion == 1) 
    {
    	PersRequest();
    }
    else if (HTTPversion == 0)
    {
    	NonPersRequest();
    } 
    else 
    {
    	printf("Error in specifying http version\n");
    }

	exit(EXIT_SUCCESS);

	return 0;
}

