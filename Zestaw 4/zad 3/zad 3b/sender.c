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

int sent = 2;
int signals_to_send;
int catcher_pid;
char* mode;
int signal_num = 0;

void send_sig(int signum) {
	if(sent == signals_to_send) {
		if(strcmp(mode,"SIGRT") == 0) { kill(catcher_pid, SIGRTMIN + 1); }
		else { kill(catcher_pid, SIGUSR2); }
	}
	
	sent++;  
	if(strcmp(mode,"KILL") == 0) {
		kill(catcher_pid, SIGUSR1);
	}
	else if(strcmp(mode,"SIGQUEUE") == 0) {
      union sigval val;
		sigqueue(catcher_pid, SIGUSR1, val);
	}
	else if(strcmp(mode,"SIGRT") == 0) {
		kill(catcher_pid, SIGRTMIN);
	}
	else
		printf("Wrong argument in send_sig");
}

void end_program(int signum) {
    exit(0);
}


int main(int argc, char* argv[])
{
	if(argc != 4) {
		printf("Wrong arguments. Expected: <CATCHER PID> <NUM OF SIGNALS> <MODE>");
		return 1;
	}

	catcher_pid = atoi(argv[1]);
	signals_to_send = atoi(argv[2]);
	mode = argv[3];


   sigset_t *signalsSet = malloc(sizeof(sigset_t));
   sigfillset(signalsSet);

	if(strcmp(mode,"KILL") == 0) {
		  sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  signal(SIGUSR1, send_sig);
 		  signal(SIGUSR2, end_program);
		  sigprocmask(SIG_BLOCK, signalsSet, NULL);
		  kill(catcher_pid, SIGUSR1);
	}
	else if(strcmp(mode,"SIGQUEUE") == 0) {
	     sigdelset(signalsSet, SIGUSR1);
        sigdelset(signalsSet, SIGUSR2);
		  signal(SIGUSR1, send_sig);
 		  signal(SIGUSR2, end_program);
		  sigprocmask(SIG_BLOCK, signalsSet, NULL);
		  kill(catcher_pid, SIGUSR1);
	}
	else if(strcmp(mode,"SIGRT") == 0) {
        sigdelset(signalsSet, SIGRTMIN);
        sigdelset(signalsSet, SIGRTMIN + 1);
		  signal(SIGRTMIN, send_sig);
 		  signal(SIGRTMIN + 1, end_program);
		  sigprocmask(SIG_BLOCK, signalsSet, NULL);
		  kill(catcher_pid, SIGRTMIN);
	}
	else {
		printf("Can't find given mode. Please chooose: KILL, SIGQUEUE or SIGRT");
		return -1;
	}





   while(1){
        sleep(1);
    }

    return 0;
}
