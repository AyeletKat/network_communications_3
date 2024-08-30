// #ifndef RUDP_API_H
// #define RUDP_API_H

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>

// #define RUDP_ACK 0x1
// #define RUDP_DATA 0x2
// #define TIMEOUT 2
// #define MAX_RETRIES 5
// #define MAX_BUFFER_SIZE 1024
// #define MAX_FILE_SIZE 2100000 // to be more then 2MB

// typedef struct {
//     int sockfd;
//     struct sockaddr_in addr;
// } rudp_socket_t;

// typedef struct {
//     uint8_t type;
//     uint16_t seq_num;
//     uint16_t checksum;
//     char data[MAX_BUFFER_SIZE];
// } rudp_packet_t;

// rudp_socket_t* rudp_socket(int port);
// int rudp_send(rudp_socket_t* sock, const char* ip, int port, const void* data, size_t len);
// int rudp_recv(rudp_socket_t* sock, void* buffer, size_t len);
// void rudp_close(rudp_socket_t* sock);
// uint16_t calculate_checksum(const void* data, size_t len);

// char* util_generate_random_data(unsigned int size);

// #endif
