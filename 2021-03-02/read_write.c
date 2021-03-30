#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

int main() {
    char buf[50];
    int fd = open("test.bin", O_CREAT | O_RDWR);

    char to_write[30] = "write to the file\nsecond line";
    size_t bytes_write = write(fd, to_write, sizeof(char) * 30);

    lseek(fd, 0, SEEK_SET);

    while (read(fd, buf, sizeof(char) * 50) != 0) {
        printf("%s", buf);
    }

    close(fd);

    return 0;
}