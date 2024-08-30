// // #include "RUDP_API.h"

// // int main(int argc, char* argv[]) {
// //     if (argc != 3) {
// //         printf("Usage: %s PORT\n", argv[0]);
// //         return -1;
// //     }

// //     int port = atoi(argv[1]);

// //     rudp_socket_t* sock = rudp_socket(port);

// //     char buffer[MAX_FILE_SIZE];
// //     if (rudp_recv(sock, buffer, sizeof(buffer)) > 0) {
// //         printf("File received successfully.\n");
// //     } else {
// //         printf("File receiving failed.\n");
// //     }

// //     rudp_close(sock);
// //     return 0;
// // }

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
// #include "RUDP_API.h"
// //#include <linux/time.h>

// #define BUFFER_SIZE 1024
// #define EXIT_MSG "EXIT"

// void receive_file(int socket, struct sockaddr_in *sender_addr, int *transmission_count, double *total_time, double *total_bandwidth) {
//     char buffer[BUFFER_SIZE];
//     FILE *file = fopen("received_file", "wb");
//     if (!file) {
//         perror("File opening failed");
//         return;
//     }

//     int start, end;
//     start = clock_gettime();

//     while (1) {
//         ssize_t bytes_received = rudp_recv(socket, buffer, BUFFER_SIZE); // deleted sender_addr
//         if (bytes_received < 0) {
//             perror("Error receiving data");
//             break;
//         }

//         // Check if it's an exit message
//         if (strncmp(buffer, EXIT_MSG, strlen(EXIT_MSG)) == 0) {
//             fclose(file);
//             return;
//         }

//         fwrite(buffer, 1, bytes_received, file);
//     }

//     end = clock_gettime();
//     fclose(file);

//     double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
//     double bandwidth = (ftell(file) * 8) / (elapsed_time / 1000.0); // in bits per second

//     *total_time += elapsed_time;
//     *total_bandwidth += bandwidth;
//     (*transmission_count)++;

//     printf("Time to receive the file: %.2f ms\n", elapsed_time);
//     printf("Bandwidth: %.2f bits per second\n", bandwidth);
// }

// int main(int argc, char *argv[]) {
//     if (argc != 3) {//changed from 2 to 3
//         fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     int port = atoi(argv[1]);
//     struct sockaddr_in sender_addr;
//     rudp_socket_t* socket = rudp_socket(port);

//     int transmission_count = 0;
//     double total_time = 0.0;
//     double total_bandwidth = 0.0;

//     while (1) {
//         receive_file(socket, &sender_addr, &transmission_count, &total_time, &total_bandwidth);

//         // After receiving the file, wait for the next packet or exit message
//         char buffer[BUFFER_SIZE];
//         ssize_t bytes_received = rudp_recv(socket, buffer, BUFFER_SIZE);// deleted sender_addr

//         if (bytes_received > 0 && strncmp(buffer, EXIT_MSG, strlen(EXIT_MSG)) == 0) {
//             break;
//         }
//     }

//     if (transmission_count > 0) {
//         double avg_time = total_time / transmission_count;
//         double avg_bandwidth = total_bandwidth / transmission_count;
//         printf("Average time: %.2f ms\n", avg_time);
//         printf("Average bandwidth: %.2f bits per second\n", avg_bandwidth);
//     }

//     rudp_close(socket);
//     return 0;
// }


#include "RUDP_API.c"

struct timeval start, end;
void print_stats(struct timeval start, struct timeval end, int totalReceived) {
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    double bandwidth = (totalReceived / 1024.0) / time_taken; 
    static double avgBandwidth = 0;
    avgBandwidth += bandwidth;
    static double avgTime = 0;
    avgTime += time_taken;
    static int counter = 0;
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Average Bandwidth: %.2f KB/s\n", bandwidth);
    printf("--------------------------------\n");
    printf("Average Time: %.2f seconds\n", avgTime / ++counter);
    printf("Average Bandwidth: %.2f KB/s\n", avgBandwidth / counter);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: -p <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int sockfd = rudp_socket();
    struct sockaddr_in serverAddr, clientAddr;
    struct RUDP_Header packetHeader;

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    // Bind socket to address
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", port);
    int amount = FILESIZE / MAX_PAYLOAD_SIZE;
        if(FILESIZE % MAX_PAYLOAD_SIZE != 0)
        {
            amount++;
        }
    

    if (receiveHandshake(sockfd, &clientAddr) <= 0) {
        printf("Handshake failed.\n");
        close(sockfd);
        exit(EXIT_FAILURE);

    }
    gettimeofday(&start, NULL);
    
    while (1) {
        int i = 1;
        gettimeofday(&start, NULL);
        while(i <= amount)
        {
            while (1)
            {
                uint16_t receivedChecksum = rudp_recv(sockfd, &clientAddr, &packetHeader);
                if (receivedChecksum == i)
                {
                    send_receiveAck(sockfd, &clientAddr, 1);//sending ack
                    i++;
                    break;
                }
            }
            
        }
        gettimeofday(&end, NULL);
        printf("Received data\n");
        print_stats(start, end, FILESIZE);
        
        
    }
    close(sockfd);
    
    return 0;
}
