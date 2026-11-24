#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int listenfd, acceptfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t pid;

    signal(SIGCHLD, SIG_IGN);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    for (;;) {
        acceptfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
        if (acceptfd < 0) {
            perror("accept error");
            continue;
        }

        pid = fork(); 

        if (pid < 0) {
            perror("fork error");
            close(acceptfd);
        } else if (pid == 0) {
            close(listenfd);

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            int bytes_read = read(acceptfd, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0) {
                buffer[strcspn(buffer, "\r\n")] = 0;

                int fd = open(buffer, O_RDONLY);
                if (fd < 0) {
                    char *err_msg = "Error: File not found.\n";
                    write(acceptfd, err_msg, strlen(err_msg));
                } else {
                    int n;
                    char file_buf[BUFFER_SIZE];
                    while ((n = read(fd, file_buf, BUFFER_SIZE)) > 0) {
                        write(acceptfd, file_buf, n);
                    }
                    close(fd);
                }
            }
            close(acceptfd);
            exit(0);
        } else {
            close(acceptfd);
        }
    }
    
    close(listenfd);
    return 0;
}

// second commit