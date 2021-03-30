#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int glbVar;

// 1) Компилирате кода и с команда size анализирате какви са размерите на сегментите памет. Отговорете защо :)

// text    data     bss     dec     hex filename
// 1819     616       8    2443     98b a.out
// 
// text - секциите във програмното пространсво на виртуалните адреси, които съдържат изпълними инструкции.
// data - част от обектния файл или кореспондиращото адресно пространство на програмата, което съдържа инициалициализираните статични променливи, които може да са глобални и локални
// bss - стартиращ символ на блока - част от обектния файл, изпълнимите файлове или асемблер кода, които съдържат статично алокирани променливи, които са декларирани, но още не са инициализирани със стойнот
// dec = data + bss
// hex - стойността на dec в hex
// filename - името на файла

int main(void) {
    int   locVar   = 0;
    pid_t childPid = 0;

    childPid = fork();

    if (childPid >= 0) {
        if (0 == childPid) {
            locVar++;
            glbVar++;
            printf("Child: %d %d\n", locVar, glbVar);
        } else {
            locVar--;
            glbVar--;
            printf("Parent: %d %d\n", locVar, glbVar);
        }
    } else {
        printf("Forking failed!\n");
    }

    return 0;
}
