#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static volatile sig_atomic_t sig_received = 0;

void handler(int sig) {
    sig_received = sig;
}

int wait_till_pid(pid_t pid, int sec) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGCHLD, &sa, nullptr);
    sigaction(SIGALRM, &sa, nullptr);

    alarm(sec);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    sigset_t suspend_mask = oldmask;
    sigdelset(&suspend_mask, SIGCHLD);
    sigdelset(&suspend_mask, SIGALRM);

    sig_received = 0;

    while (1) {

        sigsuspend(&suspend_mask);

        if (sig_received == SIGCHLD) {

            int status;
            pid_t w;

            while ((w = wait(&status)) > 0) {
                if (w == pid) {
                    alarm(0);
                    sigprocmask(SIG_SETMASK, &oldmask, nullptr);
                    return pid;
                }
            }

            if (errno == ECHILD) {
                alarm(0);
                sigprocmask(SIG_SETMASK, &oldmask, nullptr);
                return -1;
            }
        }

        else if (sig_received == SIGALRM) {
            sigprocmask(SIG_SETMASK, &oldmask, nullptr);
            return 0;
        }
    }
}
