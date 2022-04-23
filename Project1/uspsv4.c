#include "ADTs/queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include<fcntl.h>
#include"p1fxns.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#include<signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#define BUF_SIZE 4096
#define MAX 128
#define MIN_QUANTUM 20
#define MAX_QUANTUM 2000
#define MS_PER_TICK 20
#define UNUSED __attribute__((unused))

typedef struct pcb {
	pid_t pid;
	int ticks;
	bool isalive;
	bool usr1;
} PCB;

PCB array[MAX];
int num_procs = 0;
volatile int active_processes = 0;
volatile int usr1 = 0;
pid_t parent;
const Queue *q = NULL;
PCB *current = NULL;
int ticks_in_quantum = 0;


void NotEnoughArgumentsError() {
	char msg[] = "Too many or too few arguments when executing (Should be three)\n";
	int fd = open("/dev/tty", O_WRONLY);
	write(fd, msg, sizeof(msg));
	exit(EXIT_FAILURE);
}	


void NotValidFileError() {
	char msg[] = "Not a valid file.\n";
	int fd = open("/dev/tty", O_WRONLY);
	write(fd, msg, sizeof(msg));
	exit(EXIT_FAILURE);
}
static int pid2index(pid_t pid) {
	int i ;
	for (i = 0; i < num_procs; ++i) {
		if(array[i].pid == pid)
			return i;
	}
	return -1;
}

static void handler(UNUSED int sig) {
	usr1++;
}

static void usr2_handler(UNUSED int sig) {

}

static void chld_handler(UNUSED int sig) {
	pid_t pid;
	int status;
	pid = waitpid(-1, &status , WNOHANG);

	if (pid > 0) {
		active_processes--;
		array[pid2index(pid)].isalive = false;
		signal(SIGUSR2, parent);
	}


}

static void alrm_handler(UNUSED int sig) {

	if (current != NULL) {
		if (current->isalive) {
			--current->ticks;
			if (current->ticks > 0) {
				return;
			}
			kill(current->pid, SIGSTOP);
			q->enqueue(q, ADT_VALUE(current));
		}
		current = NULL;
	}
	while (q->dequeue(q, ADT_ADDRESS(&current))) {
		if (!current ->isalive)
			continue;
		current->ticks = ticks_in_quantum;

		if (current->usr1) {
			current->usr1 = false;
			kill(current->pid, SIGUSR1);
		} else {
			kill(current->pid, SIGCONT);
		}
	return;
	}
}

int main (int argc, char *argv[]) {
	char *p;
	int arguments = 3;
	int val = -1;
	struct itimerval interval;
	int bufsize, fd, wsize, term;
	bufsize = -1;
	int place = 0;
	char buf[100]; 
	/*fix magic numbers here */
	char word[50];
	char *programs[10][50];
	//quantum function given to us by professor
	if ((p = getenv("USPS_QUANTUM_MSEC")) != NULL)
                val = atoi(p);
	if (argc > 3) {
		val = atoi(argv[2]);
		arguments++;
	}
        if (argc != arguments) {
                NotEnoughArgumentsError();
        }
        if (val < MIN_QUANTUM || val > MAX_QUANTUM) {
		/*fix this to a system call */
                return 1;
        }

	fd = open(argv[3], O_RDONLY);
	//if file doesn't exist then we throw an error
	if (fd == -1) 
		NotValidFileError();
	while (bufsize != 0) {
		bufsize = p1getline(fd,buf,100); 
		//get line by line of workload
		int len = p1strlen(buf);
	        if (len == 0) {
		      break;
		}	      
		//want to know length of the line
		buf[len - 1] = '\0'; 
		//change the end character from a \n to a null
		wsize = 0;
		term = 0;
		while (wsize != -1) { 
			//-1 will be thrown when p1getword finds a \0
			wsize = p1getword(buf,wsize,word);
			programs[place][term]= p1strdup(word); 
			//copy command into programs
			term++;
		}
		programs[place][term-1] = NULL; 
		//last one should be null for later
		place++; 
		//increment place as there will be a new command to be run
	}
	int pid[place]; 
	//array of processes that are as big as the number of lines
	signal(SIGUSR1, &handler); 
	signal(SIGUSR2, usr2_handler);
	signal(SIGCHLD, chld_handler);
	signal(SIGALRM, alrm_handler);
	//bind signals to the handler functions
        	
	parent = getpid();
	//want to keep track of the big parent

	val = MS_PER_TICK * ((val + 1) / MS_PER_TICK);
	ticks_in_quantum = val / MS_PER_TICK;

	q = Queue_create(NULL);

	for (int i = 0; i < place; i++) {
		PCB * p = array+num_procs;
		pid[i] = fork();
	        if (pid[i] == 0) {
			while (!usr1) {
				pause();
			}
			execvp(programs[i][0], programs[i]);
		}
		else {
			p->pid = pid[i];
			p->isalive = true;
			p->usr1 = true;
			q->enqueue(q,ADT_VALUE(p));
		}
		num_procs++;
			//when signal is sent then childrne will execute the command and be terminated
		
        }

	for (int i = 0; i < num_procs; i++)
		printf("Pid Array %d\n", array[i].pid);
        active_processes = num_procs;
        interval.it_value.tv_sec = MS_PER_TICK/1000;//seconds
        interval.it_value.tv_usec = (MS_PER_TICK*1000) % 1000000;//micrseconds
        interval.it_interval = interval.it_value; //we can straight up copy this block of code

        if (setitimer(ITIMER_REAL, &interval, NULL) == -1) {
                for (int i = 0; i < num_procs; ++i)
                        kill(array[i].pid, SIGKILL);
                goto cleanup;
	  }

	alrm_handler(SIGALRM);

        while (active_processes > 0) {
		//now we wait until all children are terminated
	        pause();
       }

cleanup:
	close(fd);
	if(q->isEmpty(q))
		printf("Empty?\n");
	q->destroy(q);
	return 0;

}


/*
for i in 0 .. numprograms-1
 pid[i] = fork();
 if (pid[i] == 0)
  	prepare argument structure;
  	execvp(program[i], args[i])
for i in 0 .. numprograms-1
     wait(pid[i])

 */

