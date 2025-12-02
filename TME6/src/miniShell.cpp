#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>

char* mystrdup(const char* src);

volatile pid_t child_pgid = 0;

void handle_sigint(int) {
    const char *msg = "\nNous passons dans le handler\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    if (child_pgid > 0) {
        kill(-child_pgid, SIGINT);
    }
}

int main() {
    struct sigaction sa{};
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);

    std::string line;

    while (true) {
        std::cout << "mini-shell> " << std::flush;

        if (!std::getline(std::cin, line)) {
            std::cout << "\nExiting on EOF (Ctrl-D)." << std::endl;
            break;
        }
        if (line.empty()) continue;

        // Parsing simple
        std::istringstream iss(line);
        std::vector<std::string> args;
        std::string token;
        while (iss >> token) args.push_back(token);
        if (args.empty()) continue;

        // Préparer argv pour execvp
        char** argv = new char*[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = mystrdup(args[i].c_str());
        argv[args.size()] = nullptr;

        if (args[0] == "exit") {
            std::cout << "Ciao !" << std::endl;
            exit(0);
        }

        else {

            // --- fork() ---
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");
                return 1;
            }
            if (pid == 0) {
                // --- PROCESSUS FILS ---
                setpgid(0, 0);
                signal(SIGINT, SIG_DFL);

                execvp(argv[0], argv);
                perror("exec");
                exit(1);
            } else {
                // --- PROCESSUS PARENT (SHELL) ---

                setpgid(pid, pid);
                child_pgid = pid;

                int status;

                // Attendre la fin du fils
                waitpid(pid, &status, 0);

                if (WIFEXITED(status)) std::cout << "Terminé normalement avec code de sortie : " << status << std::endl;
                else if (WIFSIGNALED(status)) {
                    int sig = WTERMSIG(status);
                    std::cout << "Terminé après un signal de numéro : " << sig << std::endl;
                }

                // Aucun processus foreground
                child_pgid = 0;
            }

        }

        // cleanup
        for (size_t i = 0; i < args.size(); ++i) delete[] argv[i];
        delete[] argv;
    }

    return 0;
}

char* mystrdup(const char* src) {
    if (!src) return nullptr;
    size_t len = strlen(src) + 1;
    char* dest = new char[len];
    memcpy(dest, src, len);
    return dest;
}
