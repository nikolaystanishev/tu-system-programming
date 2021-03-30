#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

int main() {
    int fd = open("test.mp3", O_RDONLY);
    lseek(fd, -128, SEEK_END);

    char buf[30];
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 3);
    printf("Header: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 30);
    printf("Title: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 30);
    printf("Artist: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 30);
    printf("Album: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 4);
    printf("Year: %s\n", buf);
    memset(buf, '\0', sizeof(buf));


    read(fd, buf, sizeof(char) * 28);
    printf("Comment: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 1);
    printf("Zero Byte: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 1);
    printf("Track: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    read(fd, buf, sizeof(char) * 1);
    printf("Genre: %s\n", buf);
    memset(buf, '\0', sizeof(buf));

    return 0;
}