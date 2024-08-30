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

//#define MAX_SIZE 1024
#define FILE_BUFFER_SIZE 2100000 // 2MB = 2^21 bytes = 2097152 bytes. therefore buffer is rounded up to be more then 2MB as required

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

int main(int argc, char *argv[]) {
   if (argc < 7) {
      fprintf(stderr, "Usage: %s -ip <IP> -p <PORT> -algo <ALGO>\n", argv[0]);
      return 1;
   }
   const char* IP = argv[2];// CHECK NUMBERS CORECTNESS // added atoi
   const int PORT = atoi(argv[4]);
   const char* ALGO = argv[6];

   if (PORT <= 0 || PORT > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", PORT);
        return 1;
    }

   int soc = socket(AF_INET, SOCK_STREAM, 0);
   if (soc < 0){
      perror("error opening socket");
      return 1;
   }
   struct sockaddr_in receiver;
   memset(&receiver, 0, sizeof(receiver));
   receiver.sin_family = AF_INET;
   receiver.sin_port = htons(PORT);
   if (inet_pton(AF_INET, IP, &receiver.sin_addr) <0){//IP=argv[2]. here: convert ip adress from text to binary form
      close(soc);
      return 1;
   }
   if (strcmp(ALGO, "reno") != 0 && strcmp(ALGO, "cubic") != 0) {
      close(soc);
      return 1;
   }
   if (setsockopt(soc, IPPROTO_TCP, TCP_CONGESTION, ALGO, strlen(ALGO)) < 0) {
      close(soc);
      return 1;
   }
   if (connect(soc, (struct sockaddr*)&receiver, sizeof(receiver)) < 0){ // establishes TCP connection to the receiver
      close(soc);
      return 1;
   }

   printf("Sender Connected To Receiver...\n");

   char* data = util_generate_random_data(FILE_BUFFER_SIZE); // hidden calloc! remmeber to free
   if (data == NULL) {
      perror("error generating data");
      close(soc);
      return 1;
   }

   int resend = 1;
   do {
      int bytes_sent = send(soc, data, FILE_BUFFER_SIZE, 0);
      if (bytes_sent < 0) {
         perror("error sending data");
         close(soc);
         return 1;
      }
      printf("Data sent.\n");
      printf("Do you wish to resend the data? (1 for yes, 0 for no): ");
      scanf("%d", &resend);
   } while (resend == 1);

   if (send(soc, "EXIT", strlen("EXIT"), 0) < 0) {//check strlen for bugs here because of ""
      perror("error sending exit message");
      close(soc);
      return 1;
   }

   free(data);
   close(soc);
   return 0;
   
}
