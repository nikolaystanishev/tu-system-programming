#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

int main() {
    int fd = open("test.mp3", O_CREAT | O_RDONLY);

    off_t size = lseek(fd, (size_t)0, SEEK_END);
    printf("%ld\n", size);
    
    return 0;
}