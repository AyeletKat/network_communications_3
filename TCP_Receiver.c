#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <time.h>

#define BUFFER_SIZE 1024

//void error(const char *msg) {
//    perror(msg);
//    return 1;
//}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        return 1;
    }

    const int PORT = atoi(argv[2]); // from string to int
    const char* ALGO = argv[4];

    if (PORT <= 0 || PORT > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", PORT);
        return 1;
    }

    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc < 0){
        return 1;
    }
    
    struct sockaddr_in server; // internet socket adress struct has three fields: sin_family = ipv4 or ipv6, sin_port - port numbrt, sin_addr - internet ip address
    memset(&server, 0, sizeof(server)); // first fill in memory with 0
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); // port we want to connect to (16-bit int convert to network byte order)
    if (inet_pton(AF_INET, argv[2], &server.sin_addr) <0){ // convert target ipv4 address to binary form
        close(soc);
        return 1;
    }
    
    if(bind(soc, (struct sockaddr*)&server, sizeof(server)) < 0){ // bind: give the socket soc the adress of this server
        close(soc);
        return 1;
    }
    if (listen(soc, 1) < 0){ // prepare to accept incoming connections on the socket soc, 1 - max number of waiting connections queued
        close(soc);
        return 1;
    }
    printf("Starting Receiver...\n");
    printf("Waiting for TCP connection...\n");
    struct sockaddr_in client; //(client = sender)
    memset(&client, 0, sizeof(client)); // zero filling in case memory was used before
    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    //Await a connection on socket soc. When it arrives, open new socket to communicate with it, set *CLIENT-ADDRESS to the address of the connecting peer and return the new socket's descriptor.
    int client_sender_soc = accept(soc, (struct sockaddr*)&client, &client_addr_len);
    if (client_sender_soc < 0){
        close(soc);
        return 1; 
    }
    // close socket soc?!!!!!!!!!!!
    printf("TCP connection established\n");

    if (strcmp(ALGO, "reno") != 0 && strcmp(ALGO, "cubic") != 0) {
        perror("Invalid algorithm\n");
        close(soc);
        close(client_sender_soc);
        return 1;
    }
    // Set TCP congestion control algorithm
    if (setsockopt(soc, IPPROTO_TCP, TCP_CONGESTION, ALGO, strlen(ALGO)) < 0) {
        perror("Setting TCP congestion control algorithm failed");
        close(soc);
        close(client_sender_soc);
        return 1;
    }
    ////while()
    ////recieve file and make stats
    //clock_t start, end;
    //start = clock();
    ////while()//read file
    //end = clock();

}
