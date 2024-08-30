# include "RUDP_API.h"
# include <time.h>
  /* * @brief A random data generator function based on srand() and rand(). 
  * @param size The size of the data to generate (up to 2^32 bytes). 
  * @return A pointer to the buffer. */ 
 // CONTAINS CALLOC! NEEDS TO BE FREED
char* util_generate_random_data(unsigned int size) { 
   char *buffer = NULL; 
   if (size == 0) return NULL; // Argument check. 
   buffer = (char *)calloc(size, sizeof(char));  
   if (buffer == NULL) return NULL;  // Error checking.
   srand(time(NULL)); // Randomize the seed of the random number generator.
   for (unsigned int i = 0; i < size; i++) {*(buffer + i) = ((unsigned int)rand() % 256);}
   return buffer; 
}
// rudp_socket_t* rudp_socket(int port) {
//     rudp_socket_t* sock = (rudp_socket_t*)malloc(sizeof(rudp_socket_t));
//     sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
//     if (sock->sockfd < 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     memset(&sock->addr, 0, sizeof(sock->addr));
//     sock->addr.sin_family = AF_INET;
//     sock->addr.sin_addr.s_addr = INADDR_ANY;
//     sock->addr.sin_port = htons(port);

//     if (bind(sock->sockfd, (const struct sockaddr*)&sock->addr, sizeof(sock->addr)) < 0) {
//         perror("Bind failed");
//         exit(EXIT_FAILURE);
//     }

//     return sock;
// }

rudp_socket_t* rudp_socket(int port) {
    rudp_socket_t* sock = (rudp_socket_t*)malloc(sizeof(rudp_socket_t));
    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // Create a UDP socket
    
    if (sock->sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&sock->addr, 0, sizeof(sock->addr));
    sock->addr.sin_family = AF_INET;
    sock->addr.sin_addr.s_addr = INADDR_ANY;
    sock->addr.sin_port = htons(port);

    if (bind(sock->sockfd, (const struct sockaddr*)&sock->addr, sizeof(sock->addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Handshake process
    if (port == 0) {  // Sender's side
        struct sockaddr_in receiver_addr;
        memset(&receiver_addr, 0, sizeof(receiver_addr));
        receiver_addr.sin_family = AF_INET;
        receiver_addr.sin_port = htons(0);  // Placeholder port
        receiver_addr.sin_addr.s_addr = INADDR_ANY;

        rudp_packet_t handshake_packet;
        handshake_packet.type = RUDP_DATA;  // Indicating handshake start
        handshake_packet.seq_num = 0;
        strcpy(handshake_packet.data, "HANDSHAKE_INIT");
        handshake_packet.checksum = calculate_checksum(handshake_packet.data, strlen(handshake_packet.data));

        int n;
        socklen_t addr_len = sizeof(receiver_addr);
        
        // Send initial handshake packet
        sendto(sock->sockfd, &handshake_packet, sizeof(handshake_packet), 0, (const struct sockaddr*)&receiver_addr, addr_len);

        // Wait for acknowledgment from the receiver
        rudp_packet_t ack_packet;
        n = recvfrom(sock->sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&receiver_addr, &addr_len);
        if (n > 0 && ack_packet.type == RUDP_ACK) {
            printf("Handshake successful. Communication ready.\n");
        } else {
            perror("Handshake failed");
            exit(EXIT_FAILURE);
        }
    } else {  // Receiver's side
        rudp_packet_t recv_packet;
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);

        // Receive initial handshake packet
        int n = recvfrom(sock->sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&sender_addr, &addr_len);
        if (n > 0 && recv_packet.type == RUDP_DATA && strcmp(recv_packet.data, "HANDSHAKE_INIT") == 0) {
            // Send acknowledgment
            rudp_packet_t ack_packet;
            ack_packet.type = RUDP_ACK;
            ack_packet.seq_num = recv_packet.seq_num;
            strcpy(ack_packet.data, "HANDSHAKE_ACK");
            ack_packet.checksum = calculate_checksum(ack_packet.data, strlen(ack_packet.data));

            sendto(sock->sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&sender_addr, addr_len);
            printf("Handshake successful. Communication ready.\n");
        } else {
            perror("Handshake failed");
            exit(EXIT_FAILURE);
        }
    }

    return sock;
}

int rudp_send(rudp_socket_t* sock, const char* ip, int port, const void* data, size_t len) {
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest_addr.sin_addr);

    rudp_packet_t packet;
    packet.type = RUDP_DATA;
    packet.seq_num = 0;
    memcpy(packet.data, data, len);
    packet.checksum = calculate_checksum(data, len);

    int retries = 0;
    while (retries < MAX_RETRIES) {
        sendto(sock->sockfd, &packet, sizeof(packet), 0, (const struct sockaddr*)&dest_addr, sizeof(dest_addr));

        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sock->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        rudp_packet_t ack;
        socklen_t addr_len = sizeof(dest_addr);
        int n = recvfrom(sock->sockfd, &ack, sizeof(ack), 0, (struct sockaddr*)&dest_addr, &addr_len);

        if (n > 0 && ack.type == RUDP_ACK && ack.seq_num == packet.seq_num) {
            return 0; // Success
        }

        retries++;
    }

    return -1; // Failure
}

int rudp_recv(rudp_socket_t* sock, void* buffer, size_t len) {
    rudp_packet_t packet;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    int n = recvfrom(sock->sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&sender_addr, &addr_len);

    if (n > 0 && packet.type == RUDP_DATA) {
        uint16_t checksum = calculate_checksum(packet.data, len);
        if (checksum == packet.checksum) {
            memcpy(buffer, packet.data, len);

            // Send ACK
            rudp_packet_t ack;
            ack.type = RUDP_ACK;
            ack.seq_num = packet.seq_num;
            sendto(sock->sockfd, &ack, sizeof(ack), 0, (const struct sockaddr*)&sender_addr, addr_len);

            return n;
        }
    }

    return -1;
}

void rudp_close(rudp_socket_t* sock) {
    close(sock->sockfd);
    free(sock);
}

uint16_t calculate_checksum(const void* data, size_t len) {
    uint16_t* ptr = (uint16_t*)data;
    uint32_t sum = 0;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len > 0) {
        sum += *(uint8_t*)ptr;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}
