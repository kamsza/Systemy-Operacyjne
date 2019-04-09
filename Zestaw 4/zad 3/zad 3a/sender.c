#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h> 
#include <libgen.h>

int received_signals = 0;
int signals_to_send;

void count_fun(int signum) { received_signals++;  }

void end_program(int signum) {
    printf("Received signals %d\n", received_signals);
    printf("Sent signals %d\n", signals_to_send);
	 printf("Result: %s\n", received_signals == signals_to_send ? "OK" : "== DIFFERENCE ==");
    exit(0);
}


int main(int argc, char* argv[])
{
	if(argc != 4) {
		printf("Wrong arguments. Expected: <CATCHER PID> <NUM OF SIGNALS> <MODE>");
		return 1;
	}

	int catcher_pid = atoi(argv[1]);
	signals_to_send = atoi(argv[2]);
	char* mode = argv[3];


   sigset_t *signalsSet = malloc(sizeof(sigset_t));
   sigfillset(signalsSet);

	if(strcmp(mode,"KILL") == 0) {
		  sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  signal(SIGUSR1, count_fun);
 		  signal(SIGUSR2, end_program);
	}
	else if(strcmp(mode,"SIGQUEUE") == 0) {
	     sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  signal(SIGUSR1, count_fun);
 		  signal(SIGUSR2, end_program);
	}
	else if(strcmp(mode,"SIGRT") == 0) {
        sigdelset(signalsSet, SIGRTMIN);
        sigdelset(signalsSet, SIGRTMIN + 1);
		  signal(SIGRTMIN, count_fun);
 		  signal(SIGRTMIN + 1, end_program);
	}
	else {
		printf("Can't find given mode. Please chooose: KILL, SIGQUEUE or SIGRT");
		return -1;
	}

	sigprocmask(SIG_BLOCK, signalsSet, NULL);

	// SENDING
	if(strcmp(mode,"KILL") == 0) {
		for(int i = 0; i < signals_to_send; i++) 
			kill(catcher_pid, SIGUSR1);

	   kill(catcher_pid, SIGUSR2);
	}
	else if(strcmp(mode,"SIGQUEUE") == 0) {
		union sigval val;

		for(int i = 0; i < signals_to_send; i++)
			sigqueue(catcher_pid, SIGUSR1, val);
		sigqueue(catcher_pid, SIGUSR2, val);
	}
   else if(strcmp(mode,"SIGRT") == 0) {
 		for(int i = 0; i < signals_to_send; i++)
			kill(catcher_pid, SIGRTMIN);
		kill(catcher_pid, SIGRTMIN + 1);
	}

   while(1){
        sleep(1);
    }

    return 0;
}
