// Server side implementation of UDP client-server model
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

#include <signal.h>

#define PORT    33333
#define MAXLINE ((size_t) 1024)

//Global at the moment
static int sockfd = 0;
static int fd     = 0;

void SignalHandler(int sig) {
    printf( "Signal catcher called for signal %d\n", sig );

    close(sockfd);
    close(fd);
}

void InitializeSignalHandling() {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SignalHandler;
    act.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &act, 0) == -1)
    {
        perror("sigaction");
        return;
    }
}

// Driver code
int main() {
    InitializeSignalHandling();

    char buffer[MAXLINE] = {0}; 
    struct sockaddr_in servaddr = {0}, cliaddr = {0};
      
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    // Filling server information
    servaddr.sin_family      = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; //Could it be other?
    servaddr.sin_port        = htons(PORT);
      
    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    socklen_t len = 0;
    int n = sizeof(cliaddr);

    (void) printf("==BEGIN COMMUNICATION==\n");  
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);

    buffer[n] = '\0'; // Probably not needed

    (void) printf("==HANDSHAKE STARTED WITH CLIENT REQUEST==\n");  
    (void) printf("Client: %s\n", buffer);

    fd = open("/Users/nikolaystanishev/Workspace/personal/tu/VI/system-programming/tu-system-programming/2021-05-18/test.mp3", O_RDONLY, 0777);
    
    int file_size = lseek(fd, 0, SEEK_END);
    (void) lseek(fd, 0, SEEK_SET);
    (void) printf("FILE SIZE: %d\n", file_size);
    
    memset(buffer, 0, sizeof(buffer));

    size_t bytes_sent = sendto(sockfd, (const char *)&file_size, sizeof(file_size), 
        0, (const struct sockaddr *) &cliaddr,
        len);

    (void) printf("==HANDSHAKE ENDED WITH SERVER RESPONSE==\n");
    (void) printf("==SENDING PACKETS TO CLIENT==\n");
    while((n = read(fd, buffer, MAXLINE)) > 0)  {
        bytes_sent = sendto(sockfd, (const char *)&buffer, MAXLINE, 
            0, (const struct sockaddr *) &cliaddr,
            len);

        //Will block if socket is full (reached limit)
        usleep(500);
    }
    (void) printf("==SENDING PACKETS TO CLIENT ENDED==\n");
    (void) printf("==EXPECTING ACKNOWLEDGE==\n");
    
    while(1) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);

        if(n > 0) {
            if(strcmp(buffer, "Acknowledge!\0") == 0) break;
        }
    }
    
    (void) close(fd);
    (void) close(sockfd);

    return 0;
}