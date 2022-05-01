/* Patrick Thomasma Duck ID: 951623133 CIS415 Project1
 * This is my own work 
 *
 *
*/






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
static void print(pid_t pid) {
        char buf[BUF_SIZE], str[BUF_SIZE], pid_str[BUF_SIZE] , stats[BUF_SIZE], buf2[BUF_SIZE];
        int i = 0;
        char *stat[50];
        char *statm[50];
        char *io[50];
        char word[50];
        int wsize = 0;
        int fd_cmd = -1;
        int p = 0;
	int sizestat = 0;
	int sizestatm = 0;
	int sizeio = 0;


        p1itoa(pid, pid_str);
        p1strcpy(buf, "/proc/");
        p1strcat(buf, pid_str);
        p1strcpy(buf2, buf);
        p1strcat(buf2 , "/stat");
        if ((fd_cmd = open(buf2,O_RDONLY)) == -1) {
                p1perror(2, "path does not exist");
                close(fd_cmd);
                exit(1);
        }
        p1getline(fd_cmd, str, BUF_SIZE);
        while (wsize != -1) {
                wsize = p1getword(str,wsize,word);
                stat[i] = p1strdup(word);
		sizestat++;
                i++;
        }
        ////////////////////
        wsize = 0;
        i = 0;
        p1strcpy(str, "\0");
        p1strcpy(buf2, buf);
        p1strcat(buf2, "/statm");
        if ((fd_cmd = open(buf2,O_RDONLY)) == -1) {
                p1perror(2, "path does not exist");
                close(fd_cmd);
                exit(1);
        }
        p1getline(fd_cmd, str, BUF_SIZE);
        while (wsize != -1) {
                wsize = p1getword(str,wsize,word);
                statm[i] = p1strdup(word);
		sizestatm++;
                i++;
        }
        wsize = 0;
        i = 0;
        p1strcpy(str, "\0");
        p1strcpy(buf2, buf);
        p1strcat(buf2, "/io");
        if ((fd_cmd = open(buf2,O_RDONLY)) == -1) {
                p1perror(2, "path does not exist");
                close(fd_cmd);
                exit(1);
        }
        //grabbing io values
        p1getline(fd_cmd, str, BUF_SIZE);
        p1getline(fd_cmd, str, BUF_SIZE);
        p1getline(fd_cmd, str, BUF_SIZE);
        while (wsize != -1) {
                wsize = p1getword(str,wsize,word);
                while (true) {
                        if (word[p] == '\n') {
                                word[p] = '\0';
                                break;
                        }
                        p++;
                }
                p = 0;
                io[i] = p1strdup(word);
		sizeio++;
                i++;
        }
        wsize = 0;
        p1getline(fd_cmd, str, BUF_SIZE);
        while (wsize != -1) {
                wsize = p1getword(str,wsize,word);
                while (true) {
                        if (word[p] == '\n') {
                                word[p] = '\0';
                                break;
                        }
                        if (word[p] == '\0')
                                break;
                        p++;
                }
                p = 0;
                io[i] = p1strdup(word);
		sizeio++;
                i++;
        }
        i = 0;
        wsize = 0;
        p1strcpy(str, "\0");
        p1strcpy(buf2, buf);
        p1strcat(buf2, "/cmdline");
        if ((fd_cmd = open(buf2,O_RDONLY)) == -1) {
                p1perror(2, "path does not exist");
                close(fd_cmd);
                exit(1);
        }
        p1getline(fd_cmd, str, BUF_SIZE);
        while (true) {
                if (str[i] == '\0') {
                        i++;
                        if (str[i] == '\0') {
                                break;
                        }
                        str[--i] = ' ';
                }
                i++;
        }

        p1strcat(stat[0], "   ");
        p1strcat(stats, stat[0]);
        p1strcat(stat[2], "   ");
        p1strcat(stats, stat[2]);
        p1strcat(statm[0] , "   ");
        p1strcat(stats, statm[0]);
        p1strcat(statm[1] , "   ");
        p1strcat(stats, statm[1]);
        p1strcat(io[1] , "        ");
        p1strcat(stats, io[1]);
        int large = p1atoi(io[4]);
        if (large > 1000) {
                large = large / 10000;
                p1itoa(large,io[4]);
                p1strcat(io[4], "k     ");
        }
        else
                p1strcat(io[4] ,"         ");

        int usrstat = p1atoi(stat[13]);
        usrstat = usrstat / sysconf(_SC_CLK_TCK);
        p1itoa(usrstat,stat[13]);
        p1strcat(stat[13], "           ");
        p1strcat(io[4], stat[13]);
        p1strcat(stats, io[4]);
        usrstat = p1atoi(stat[14]);
        usrstat = usrstat / sysconf(_SC_CLK_TCK);
        p1itoa(usrstat,stat[14]);
        p1strcat(stat[14], "        ");
        p1strcat(stats,stat[14]);
        p1strcat(stats, str);
        p1strcat(stats, "\n");



        p1putstr(1, "PID    St  VMSZ   RSSZ  syscr    syscw     usrtm     systm      cmd   \n");
        p1putstr(1, stats);
        p1strcpy(stats, "\0");
        close(fd_cmd);

}

void NotEnoughArgumentsError() {
	p1putstr(0, "Too many or too few arguments when executing (Should be four)\n");
	exit(EXIT_FAILURE);
}	


void NotValidFileError() {
	p1putstr(0, "Not a valid file\n");
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
		kill(parent, SIGUSR2);
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
			print(current->pid);
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
	int i = 0;
	int arguments = 3;
	int val = -1;
	struct itimerval interval;
	int bufsize = -1;
	int fd = -1; 
	int wsize = 0; 
	int term = 0;
	int pid[100];
	int progs = 0;
	char buf[100]; 
	/*fix magic numbers here */

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

	while ((bufsize = p1getline(fd,buf,100)) != 0) {
		//get line by line of workload
		char word[100];
		char *programs[100];
		int len = p1strlen(buf);
	        if (len == 0) {
		      break;
		}	      
		//want to know length of the line
		buf[len - 1] = '\0'; 
		//change the end character from a \n to a null
		wsize = 0;
		term = 0;
		progs++;
		while ((wsize = p1getword(buf,wsize,word)) != -1) { 
			//-1 will be thrown when p1getword finds a \0
			programs[term]= p1strdup(word); 
			//copy command into programs
			term++;
		}
		programs[term] = NULL; 
		//last one should be null for later
		PCB *p = array+num_procs;
		pid[i] = fork();
		if (pid[i] == 0) {
			while (!usr1) {
				pause();
			}
			execvp(programs[0], programs);
		}
		else {
			for (int j = 0; j < term; j++) {
				free(programs[j]);
			}
			p ->pid = pid[i];
			p->isalive = true;
			p->usr1 = true;
			q->enqueue(q, ADT_VALUE(p));
		}
		num_procs++;
		i++;
		//increment place as there will be a new command to be run
	}
	//array of processes that are as big as the number of lines

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

