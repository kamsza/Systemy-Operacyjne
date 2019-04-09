#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int received_signals = 0;
int sender_pid = 0;

void count_fun(int sig,siginfo_t *info, void *void_ptr){
    sender_pid = info->si_pid;
    received_signals++;
}

void end_fun(int sig,siginfo_t *info, void *void_ptr) {
    if (sender_pid == 0) {
		printf("== sender pid = 0 ==");
		exit(0);
	}

	for(int i = 0; i < received_signals; i++) {
		kill(sender_pid, SIGUSR1);
	}

	kill(sender_pid, SIGUSR2);
	exit(0);
}	




int main(int argc, char **argv) {
    printf("Catcher pid  %d\n", getpid());
    if (argc != 2) {
      printf("Wrong arguments. Expected: <MODE>");
		return 1;
    }

    char* mode = argv[1];
	

   sigset_t *signalsSet = malloc(sizeof(sigset_t));
	if(signalsSet == NULL) printf("== malloc error ==");
   sigfillset(signalsSet);
	struct sigaction action;

	if(strcmp(mode,"KILL") == 0) {
		  sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  action.sa_flags = SA_SIGINFO;
		  action.sa_sigaction = &count_fun;
		  sigaction(SIGUSR1, &action, NULL);
		  action.sa_flags = 0;
		  action.sa_sigaction = &end_fun;
		  sigaction(SIGUSR2, &action, NULL);
	}
	else if(strcmp(mode,"SIGQUEUE") == 0) {
	     sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  action.sa_flags = SA_SIGINFO;
		  action.sa_sigaction = &count_fun;
		  sigaction(SIGUSR1, &action, NULL);
		  action.sa_flags = 0;
		  action.sa_sigaction = &end_fun;
		  sigaction(SIGUSR2, &action, NULL);
	}
	else if(strcmp(mode,"SIGRT") == 0) {
        sigdelset(signalsSet, SIGRTMIN);
        sigdelset(signalsSet, SIGRTMIN + 1);
		  action.sa_flags = SA_SIGINFO;
		  action.sa_sigaction = &count_fun;
		  sigaction(SIGRTMIN, &action, NULL);
		  action.sa_flags = 0;
		  action.sa_sigaction = &end_fun;
		  sigaction(SIGRTMIN + 1, &action, NULL);
	}
	else {
		printf("Can't find given mode. Please chooose: KILL, SIGQUEUE or SIGRT");
		return -1;
	}

	sigprocmask(SIG_BLOCK, signalsSet, NULL);



   while(1){
        sleep(1);
    }
}
