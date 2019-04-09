#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <zconf.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


int running = 1;

void SIGTSTP_fun(int sig_num) {
    if(running) printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu \n");
    running = 1 - running;
    signal(SIGTSTP, SIGTSTP_fun);
    return;
}

void SIGINT_fun(int sig_num) {
    printf("\nSIGINT - kończę wykonywanie programu \n");
    exit(0);
}

int main(int argc, char* argv[]) {
    time_t t;
    char time_str[128];

	struct sigaction sa;
	sa.sa_handler = SIGTSTP_fun;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTSTP, &sa, NULL);

	signal(SIGINT, SIGINT_fun);

    while(1) {
        if(running) {
            t = time(0);
				strftime(time_str, 128, "%Y-%m-%d %H:%M:%S", localtime(&t));
            printf("%s\n", time_str);
            sleep(1);
        }
    }

    return 0;
}
