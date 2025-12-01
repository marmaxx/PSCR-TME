#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#define BUFFER_SIZE 4096


void copy_fd(int fd_in) {
    char buffer[BUFFER_SIZE];
    ssize_t r;

    while ((r = read(fd_in, buffer, BUFFER_SIZE)) > 0) {
        ssize_t w = write(STDOUT_FILENO, buffer, r);
        if (w == -1) {
            perror("write");
            exit(1);
        }
    }

    if (r == -1) {
        perror("read");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    
    if (argc == 1) {
        copy_fd(STDIN_FILENO);
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "cat: impossible d'ouvrir '%s': %s\n", argv[i], strerror(errno));
            continue;
        }

        copy_fd(fd);

        close(fd);
    }

    return 0;
}