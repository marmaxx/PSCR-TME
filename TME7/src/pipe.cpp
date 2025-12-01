#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char **argv) {

  // Parse command line arguments: find the pipe separator "|"
  // Format: ./pipe cmd1 [args...] | cmd2 [args...]
  int separator = 0;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "|") == 0) {
      separator = i;
      break;
    }
  }

  if (separator == 0) {
    std::cout << "Usage : " << argv[0] << " cmd1 [args...] | cmd2 [args...]" << std::endl;
    return 1;
  }

  char **cmd1 = new char*[separator];
  char **cmd2 = new char*[argc - separator];

  for (int i = 0; i < separator - 1; ++i) {
    cmd1[i] = argv[i+1];
  }
  cmd1[separator-1] = nullptr;

  for (int i = separator + 1; i < argc; ++i) {
    cmd2[i-separator-1] = argv[i];
  }
  cmd2[argc - separator - 1] = nullptr;

  const char *temp1 = cmd1[0];
  std::cout << "cmd1 : ";
  int i = 0;
  while(temp1 != nullptr) {
    std::cout << temp1 << " ";
    i++;
    temp1 = cmd1[i];
  }
  std::cout << std::endl;

  const char *temp2 = cmd2[0];
  std::cout << "cmd2 : ";
  i = 0;
  while(temp2 != nullptr) {
    std::cout << temp2 << " ";
    i++;
    temp2 = cmd2[i];
  }
  std::cout << std::endl;


  // Create a pipe for inter-process communication
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return 1;
  }

  // Fork the first child process ; child redirects out to write end of pipe, then exec
  pid_t p1 = fork();
  if (p1 == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    execvp(cmd1[0], cmd1);
    std::cout << "Erreur lors du premier recouvrement" << std::endl;
    exit(1);
  }
  else if (p1 == -1) {
    perror("premier fork");
    return 1;
  }

  // Fork the second child process ; child redirects in from read end of pipe, then exec
  pid_t p2 = fork();
  if (p2 == 0) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    execvp(cmd2[0], cmd2);
    std::cout << "Erreur lors du deuxieme recouvrement" << std::endl;
    exit(1);
  }
  else if (p2 == -1) {
    perror("deuxième fork");
    return 1;
  }

  close(pipefd[0]);
  close(pipefd[1]);

  // Wait for both children to finish
  wait(nullptr);
  wait(nullptr);
  return 0;
}
