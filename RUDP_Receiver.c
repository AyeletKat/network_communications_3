
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
    printf("Current Time taken: %.2f seconds\n", time_taken);
    printf("Current Average Bandwidth: %.2f KB/s\n", bandwidth);
    printf("--------------------------------\n");
    printf("Average Time: %.2f seconds\n", avgTime / ++counter);
    printf("Average Bandwidth: %.2f KB/s\n", avgBandwidth / counter);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
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
        rudp_close(sockfd);
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
    rudp_close(sockfd);
    
    return 0;
}
