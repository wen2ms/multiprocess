#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void father_exit(int sig) {
    if (sig > 0) {
        signal(sig, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);

        printf("catching the signal(%d)...\n", sig);
    }

    kill(0, SIGTERM);

    printf("Father process exiting...\n");

    exit(0);
}

void child_exit(int sig) {
    if (sig > 0) {
        signal(sig, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);

        printf("catching the signal(%d)...\n", sig);
    }

    printf("Child process exiting...\n");

    exit(0);
}

int main() {
    for (int i = 0; i < 100; ++i) {
        signal(i, SIG_IGN);
    }

    signal(SIGINT, father_exit);
    signal(SIGTERM, father_exit);

    // if (fork() > 0) {
    //     exit(0);
    // }

    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (file_descriptor == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(file_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));

    if (ret == -1) {
        perror("bind");
        return -1;
    }

    ret = listen(file_descriptor, 128);

    if (ret == -1) {
        perror("listen");
        return -1;
    }

    struct sockaddr_in client_address;
    unsigned int client_address_len = sizeof(client_address);

    signal(SIGCHLD, SIG_IGN);

    while (1) {
        int client_file_descriptor = accept(file_descriptor, (struct sockaddr*)&client_address, &client_address_len);

        if (client_file_descriptor == -1) {
            perror("accept");
            return -1;
        }

        char ip[32];
        printf("Client IP: %s, Port: %d\n", inet_ntop(AF_INET, &client_address.sin_addr.s_addr, ip, sizeof(ip)),
               ntohs(client_address.sin_port));

        if (fork() > 0) {
            close(client_file_descriptor);
            continue;
        }

        signal(SIGINT, child_exit);
        signal(SIGTERM, child_exit);

        close(file_descriptor);
        while (1) {
            char buffer[1024];
            int receive_data_len = recv(client_file_descriptor, buffer, sizeof(buffer), 0);

            if (receive_data_len > 0) {
                printf("client say: %s\n", buffer);

                send(client_file_descriptor, buffer, receive_data_len, 0);
            } else if (receive_data_len == 0) {
                printf("client disconnected...\n");

                close(client_file_descriptor);

                exit(0);
            } else {
                perror("recv");
                return -1;
            }
        }
    }

    return 0;
}