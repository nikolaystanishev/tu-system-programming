// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT    8088
#define MAXLINE ((size_t) 1024)

struct sockaddr_in InitializeServerAddr() {
    struct sockaddr_in   servaddr = {0};

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY; //Could it be other?

    return servaddr;
}

int OpenSocket() {
    int sockfd           = 0;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void ReceiveFile(int sockfd, struct sockaddr_in servaddr, char* file) {
    char buffer[MAXLINE] = {0};
    const char *hello    = "Request file!\0";
    const char *ack      = "Acknowledge!\0";

    socklen_t len     = 0;
    int file_size     = 0; 

    (void) printf("==BEGIN COMMUNICATION==\n");
    (void) sendto(sockfd, (const char *)hello, strlen(hello),
                0, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));

    (void) printf("==HANDSHAKE STARTED WITH NUMBER OF BYTES EXPECTED==\n");          
    size_t bytes_recieved = recvfrom(sockfd, (char *)&file_size, sizeof(file_size), 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);

    (void) printf("Expected file size : %d\n", file_size);
    (void) printf("==HANDSHAKE ENDED! TRANSFERRING!==\n");
    
    (void) printf("==RECEIVING MP3 FILE==\n");
    bytes_recieved           = 0;
    size_t bytes_curr_status = 0;

    int fd = open(file, O_CREAT | O_WRONLY, 0777);
    
    (void) printf("==RECEIVING PACKETS FROM SERVER==\n");
    while(1) {
        bytes_curr_status = recvfrom(sockfd, (char *)&buffer, MAXLINE,
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);

        bytes_recieved += bytes_curr_status;
        (void) write(fd, buffer, MAXLINE);

        (void) printf("bytes_recieved: %d ", bytes_recieved);

        //Is the following statement correct?
        if(bytes_recieved == file_size || bytes_curr_status == 0)
        {
            (void)sendto(sockfd, (const char *)ack, strlen(ack),
                0, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));
            break;
        }
    }
    (void) printf("==MP3 FILE RECEIVED==\n");

    (void) close(fd);
}
  
// Driver code
int main() {
    int sockfd = OpenSocket();
    struct sockaddr_in servaddr = InitializeServerAddr();

    ReceiveFile(sockfd, servaddr, "/Users/nikolaystanishev/Workspace/personal/tu/VI/system-programming/tu-system-programming/2021-05-18/send/test.mp3");

    (void) close(sockfd);

    return 0;
}