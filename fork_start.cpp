#include <stdio.h>
#include <unistd.h>

int main() {
    printf("The pid of this program = %d\n", getpid());

    int ipid = fork();

    sleep(1);

    printf("ipid = %d\n", ipid);

    if (ipid != 0) {
        printf("The pid of parent process = %d\n", getpid());
    } else {
        printf("The pid of child process = %d\n", getpid());
    }

    return 0;
}