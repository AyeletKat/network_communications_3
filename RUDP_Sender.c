// #include "RUDP_API.h"

// int main(int argc, char* argv[]) {
//     if (argc != 4) {
//         printf("Usage: %s ip PORT\n", argv[0]);
//         return -1;
//     }

//     const char* ip = argv[1];
//     int port = atoi(argv[2]);

//     rudp_socket_t* sock = rudp_socket(0);  // Bind to any available port

//     //char data[MAX_FILE_SIZE];
//     char* data = util_generate_random_data(MAX_FILE_SIZE);// Simulating a random file
//     if (data == NULL) {
//         perror("error generating data");
//         rudp_close(sock);
//         return 1;
//     }
     
//     if (rudp_send(sock, ip, port, data, sizeof(data)) == 0) {
//         printf("File sent successfully.\n");
//     } else {
//         printf("File sending failed.\n"); // retries are in send, here finall fail
//     }

//     rudp_close(sock);
//     free(data);
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "RUDP_API.h"

#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 2100000
#define EXIT_MSG "EXIT"

void send_file(const char *filename, rudp_socket_t *sock, const char *receiver_ip, int port) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File opening failed");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (rudp_send(sock, receiver_ip, port, buffer, bytes_read) != 0) {
            printf("File sending failed.\n");
            break;
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <Receiver IP> <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const int receiver_ip = atoi(argv[2]);
    const int port = atoi(argv[4]);

    struct sockaddr_in receiver_addr;
    rudp_socket_t* socket = rudp_socket(port);

    // while (1) {
    //     send_file(filename, socket, &receiver_addr);

    //     char response[10];
    //     printf("Do you want to send the file again? (yes/no): ");
    //     fgets(response, sizeof(response), stdin);

    //     if (strncmp(response, "no", 2) == 0) {
    //         rudp_send(socket, EXIT_MSG, strlen(EXIT_MSG), &receiver_addr);
    //         break;
    //     }
    // }
    char* data = util_generate_random_data(MAX_FILE_SIZE); // hidden calloc! remmeber to free
   if (data == NULL) {
      perror("error generating data");
      rudp_close(socket);
      return 1;
   }

   int resend = 1;
   do {
      int bytes_sent = rudp_send(socket, receiver_ip, port, data, sizeof(data));
      if (bytes_sent < 0) {
         perror("error sending data");
         rudp_close(socket);
         return 1;
      }
      printf("Data sent.\n");
      printf("Do you wish to resend the data? (1 for yes, 0 for no): ");
      scanf("%d", &resend);
   } while (resend == 1);

   if (send(socket, "EXIT", strlen("EXIT"), 0) < 0) {//check strlen for bugs here because of ""
      perror("error sending exit message");
      rudp_close(socket);
      return 1;
   }

    rudp_close(socket);
    return 0;
}
