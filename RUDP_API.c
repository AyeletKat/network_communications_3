#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define MAX_PAYLOAD_SIZE 1024
#define FILESIZE 2100000 // 2MB = 2^21 bytes = 2097152 bytes. therefore buffer is rounded up to be more then 2MB as required
#define TIMEOUT_SEC 2
#define MAX_RETRIES 3

// different packets flags defining:
#define REGFLAG 0x01
#define SYNFLAG 0x02
#define ACKFLAG 0x03
#define FIN 0x04


char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
    return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
    return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

struct RUDP_Header {
    uint16_t length;  // Length of the packet
    uint16_t checksum; // Checksum for error detection
    uint8_t flags;    // Flags for various control information 
    char value[MAX_PAYLOAD_SIZE];      // Data payload
};

// Function to build an RUDP packet with data
void packetConstruct(struct RUDP_Header *header, char *data, uint16_t dataLength, uint16_t checksum, uint8_t flags) {
    header->length = htons(sizeof(struct RUDP_Header)); // Total length of packet
    header->checksum = htons(checksum); // Convert checksum to network byte order
    header->flags = flags; // 8 bit single byte no need for htons conversion
    memcpy(header->value, data, dataLength);
}

int send_receiveAck(int sockfd, struct sockaddr_in *addr, int isSend) {
    struct RUDP_Header ackHeader;

    if (isSend) {
        packetConstruct(&ackHeader, "ACK", sizeof("ACK"), 0, ACKFLAG);
        if (sendto(sockfd, &ackHeader, sizeof(ackHeader), 0, (const struct sockaddr *)addr, sizeof(*addr)) < 0) {
            perror("sendto failed");
            return -1;
        }
        printf("Acknowledge sent.\n");
    } else {
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

        char ackBuffer[sizeof(struct RUDP_Header)];
        socklen_t addrLen = sizeof(*addr);
        int numBytesReceived = recvfrom(sockfd, ackBuffer, sizeof(ackBuffer), 0, (struct sockaddr *)addr, &addrLen);
        if (numBytesReceived < 0) {
            printf("Acknowledge not received. Retransmitting...\n");
            return 0; // Acknowledge not received
        }
        printf("Acknowledge received.\n");
    }

    return 1; // Acknowledge sent or received successfully
}

int performHandshake(int sockfd, struct sockaddr_in *serverAddr) {
    struct RUDP_Header handshakePacket;

    // Send handshake packet to the receiver
    packetConstruct(&handshakePacket, "SYN", sizeof("SYN"), 0, SYNFLAG); 
    if (sendto(sockfd, &handshakePacket, ntohs(handshakePacket.length), 0, (const struct sockaddr *)serverAddr, sizeof(*serverAddr)) < 0) {
        perror("sendto failed");
        return -1; // Error sending handshake packet
    }
    printf("Handshake packet sent.\n");

    // Wait for acknowledgment from the receiver
    if (!send_receiveAck(sockfd, serverAddr, 0)) {
        printf("Acknowledgment for handshake not received. Handshake failed.\n");
        return 0; // Handshake failed
    }

    send_receiveAck(sockfd, serverAddr,1);//sendind ack

    printf("Handshake successful.\n");
    return 1; // Handshake successful
}


int receiveHandshake(int sockfd, struct sockaddr_in *clientAddr) {
    struct RUDP_Header handshakePacket;

    // Wait for handshake packet from the sender
    socklen_t clientAddrLen = sizeof(*clientAddr);
    int numBytesReceived = recvfrom(sockfd, &handshakePacket, sizeof(handshakePacket), 0, (struct sockaddr *)clientAddr, &clientAddrLen);
    if (numBytesReceived < 0) {
        perror("recvfrom failed");
        return -1; // Error receiving handshake packet
    }

    // Send acknowledgment back to the sender
    send_receiveAck(sockfd, clientAddr,1); // sending ack

    printf("Handshake packet received and acknowledged.\n");
    printf("Waiting for acknowledgment...\n");
    if(!send_receiveAck(sockfd, clientAddr,0)) {
        printf("Acknowledgment for handshake not received. Handshake failed.\n");
        return 0; // Handshake failed
    }
    return 1; // Handshake successful
}

int rudp_socket() {
    int sockfd;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}
void rudp_close(int sockfd) {
    close(sockfd);
}

void rudp_send(int sockfd, struct sockaddr_in *serverAddr, struct RUDP_Header *packetHeader) {
    int retries = 0;
    while (retries < MAX_RETRIES) {
        if (sendto(sockfd, packetHeader, ntohs(packetHeader->length), 0, (const struct sockaddr *)serverAddr, sizeof(*serverAddr)) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);
        }
        printf("Packet sent.\n");
        if (send_receiveAck(sockfd, serverAddr,0)) {
            break; // Packet sent successfully
        }
        retries++;
    }
}

uint16_t rudp_recv(int sockfd, struct sockaddr_in *clientAddr, struct RUDP_Header *packetHeader) {
    socklen_t clientAddrLen = sizeof(*clientAddr);
    int numBytesReceived = recvfrom(sockfd, packetHeader, sizeof(*packetHeader), 0, (struct sockaddr *)clientAddr, &clientAddrLen);
    if (numBytesReceived < 0) {
        perror("recvfrom failed");
  
        return 0xFFFF; // Error receiving packet
        //exit(EXIT_FAILURE);
    }
    printf("Packet received.\n");
    printf("Packet checksum: %d\n", ntohs(packetHeader->checksum));
    return ntohs(packetHeader->checksum);
}