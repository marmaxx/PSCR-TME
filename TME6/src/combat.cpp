#include <iostream>
#include <csignal>

#include "util/rsleep.h"
#include <unistd.h>

volatile sig_atomic_t PV = 3;

void handlerSig(int sig){
    PV = PV - 1;
}

void handler_luke(int sig){
    std::cout << "coup paré" << std::endl;
}

void attaque (pid_t adversaire) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handlerSig;

    if (sigaction(SIGUSR1, &sa, nullptr) < 0) perror("sigaction (attaque)");

    if (kill(adversaire, SIGUSR1) < 0 && errno== ESRCH) {
        std::cout << "Je suis " << getpid() << " et j'ai gagné !!" <<std::endl;
        exit(0);
    } 

    pr::randsleep();
}

void defense() {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGUSR1, &sa, nullptr) < 0) perror("sigaction (defense)");

    pr::randsleep();
}

void defenseLuke() {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler_luke;

    if (sigaction(SIGUSR1, &sa, nullptr) < 0) perror("sigaction (defense luke)");

    sigset_t mask, oldMask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &mask, &oldMask) < 0) perror("sigprocmask (masquage)");

    pr::randsleep();

    sigset_t suspend_mask;
    sigemptyset(&suspend_mask);
    if (sigsuspend(&suspend_mask) == -1 && errno != EINTR) perror("sigsuspend");

    if (sigprocmask(SIG_SETMASK, &oldMask, nullptr) < 0) perror("sigprocmask (restauration)");
}

void combat_luke (pid_t adversaire) {
    while (PV > 0) {
        defenseLuke();
        attaque(adversaire);
        if (kill(adversaire, 0) < 0 && errno == ESRCH) {
            std::cout << "Je suis " << getpid() << " et j’ai gagné !!" << std::endl;
            exit(0);
        }
        std::cout << "PV de " << getpid() << " : " << PV << std::endl;
    }
    std::cout << "Je suis " << getpid() << " et j'ai perdu..." << std::endl;
    exit(0);
}

void combat_vador (pid_t adversaire) {
    while (PV > 0) {
        defense();
        attaque(adversaire);
        if (waitpid(adversaire, NULL, WNOHANG) > 0) {
            std::cout << "Je suis " << getpid() << " et j'ai gagné !!" << std::endl;
            exit(0);
        }
        std::cout << "PV de " << getpid() << " : " << PV << std::endl;
    }
    std::cout << "Je suis " << getpid() << " et j'ai perdu..." << std::endl;
    exit(0);
}

int main() {
    std::cout << "Placeholder for combat" << std::endl;
    
    /*sigset_t mask; 
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);*/

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        std::cout << "Je suis Luke (" << getpid() << ")" << std::endl;
        combat_luke(getppid());
    }
    else {
        std::cout << "Je suis Vador (" << getpid() << ")" << std::endl;
        combat_vador(pid);
    }
    return 0;
}