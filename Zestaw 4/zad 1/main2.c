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


int pid = -1;

void SIGTSTP_fun(int sig_num) {
	if(pid != -1) {
		printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu \n");
		kill(pid, SIGTSTP);
		pid = -1;
	}
	else {
		pid = fork();
		if(pid == 0) execlp("./shell_date", "./shell_date", NULL);
   }
	if(pid != 0) signal(SIGTSTP, SIGTSTP_fun);
	return;
}

void SIGINT_fun(int sig_num) {
    printf("\nSIGINT - kończę wykonywanie programu \n");
    if(pid != -1) kill(pid, SIGINT);
	 exit(0);
}

int main(int argc, char* argv[]) {
	struct sigaction sa;
	sa.sa_handler = SIGTSTP_fun;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTSTP, &sa, NULL);

	signal(SIGINT, SIGINT_fun);

	pid = fork();
	if(pid == 0) execlp("./shell_date", "./shell_date", NULL);

	while(pid != 0) 
		sleep(1);
   
    
    return 0;
}
