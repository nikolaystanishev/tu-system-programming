#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// 2) Стартирате два процеса чрез fork. Единият чете и извежда информацията за mp3 в конзолата. Другият променя изпълнител и година (arist/year) във файла. Двата процеса се изпълняват до прекъсване от потребител, т.е. обновявате и принтирате информацията до безкрайност.
// * Направете отново анализ на използваната памет - основно сегменти памет и размер на стека (чрез size - compile time и top - runtime)

// text    data     bss     dec     hex filename
// 3844     688       8    4540    11bc a.out

// PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
// 353 nstanis+  20   0   10412    468    436 S   0.0  0.0   0:00.00 a.out
// 354 nstanis+  20   0   10544    348    264 S   0.0  0.0   0:00.03 a.out

void print_mp3_metadata();
void change_mp3_artist_year();
void random_string(char *, size_t);

int main(void) {
    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("Forking failed!\n");
    }

    while (1) {
        if (pid == 0) {
            print_mp3_metadata();
        } else {
            change_mp3_artist_year();
        }
        
        sleep(1);
    }

    return 0;
}

void print_mp3_metadata() {
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

    close(fd);
}

void change_mp3_artist_year() {
    int fd = open("test.mp3", O_RDONLY);
    lseek(fd, -95, SEEK_END);

    char buf[30];
    memset(buf, '\0', sizeof(buf));

    char artist[30];
    random_string(artist, 30);
    write(fd, artist, sizeof(char) * 30);

    lseek(fd, -35, SEEK_END);

    char year[4];
    random_string(year, 4);
    write(fd, year, sizeof(char) * 4);

    close(fd);
}

void random_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}
