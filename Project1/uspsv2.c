/* Patrick Thomasma Duck ID: 951623133 CIS415 Project1
 * This is my own work 
 *
 *
*/




#include <stdlib.h>
#include <stdio.h>
#include<fcntl.h>
#include"p1fxns.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#include<signal.h>
#define BUF_SIZE 256
#define UNUSED __attribute__((unused))





volatile int usr1 = 0;

void NotEnoughArgumentsError() {
	p1perror(0, "Too many or too few arguments when executing (Should be three)\n");
	exit(EXIT_FAILURE);
}	


void NotValidFileError() {
	p1perror(0, "Not a valid file. \n");
	exit(EXIT_FAILURE);
}


static void handler(UNUSED int sig) {
	usr1++;
}



int main (int argc, char *argv[]) {
	char *p;
	UNUSED int val = -1;
	if ((p = getenv("VARIABLE_NAME")) != NULL)
		val = atoi(p);
	if (argc != 3) {
		NotEnoughArgumentsError();
	}
	int bufsize = 1;
	int fd = -1;
	int wsize = 0;
	int t = 0;
	int term = 0;
	int pid[100];
	int progs = 0;
	char buf[100];
	fd = open(argv[2], O_RDONLY);
	if (fd == -1)
		NotValidFileError();
	signal(SIGUSR1, handler);
	while ((bufsize = p1getline(fd,buf,100)) != 0) {
		char word[100];
		char *programs[100];
		int len = p1strlen(buf);
		buf[len - 1] = '\0';
		wsize = 0;
		progs++;
		term = 0;
		while ((wsize = p1getword(buf,wsize,word)) != -1) {
			programs[term] = p1strdup(word);
			term++;
		}
		programs[term] = NULL;
		pid[t] = fork();
		if (pid[t] == 0) {
			while (!usr1) {
				pause();
			}
			close(fd);
			execvp(programs[0], programs);
		}
		for (int j = 0; j < term; j++) {
			free(programs[j]);
		}
		t++;
	}

      sleep(2);

      for(int i = 0; i < progs; i++) {
	      kill(pid[i], SIGUSR1);
      }
      for (int i = 0; i < progs; i++) {
	      kill(pid[i], SIGSTOP);
      }
      for (int i = 0; i < progs; i++) {
	      kill(pid[i], SIGCONT);
      }
      close(fd);
      for (int i = 0; i < progs; i++) {
	      wait(NULL);
      }


exit(0);

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
