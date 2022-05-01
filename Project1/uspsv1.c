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
#define BUF_SIZE 100
#define UNUSED __attribute__((unused))


void NotEnoughArgumentsError() {
	p1perror(0,"Too many or too few arugments when executing (Should be three)\n");
	exit(EXIT_FAILURE);
}	


void NotValidFileError() {
	p1perror(0,"Not a valid file");
	exit(EXIT_FAILURE);
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
	int wsize = 0;
	int fd = -1;
	int term = 0;
	int progs = 0;
	char buf[100];
	int pid;
	fd = open(argv[2], O_RDONLY);
	if (fd == -1)
		NotValidFileError(argv[2]);
	while ((bufsize = p1getline(fd,buf,100)) != 0) {
		char word[100];
		char *programs[100];
		//bufsize = p1getline(fd,buf,100);
		int len = p1strlen(buf);
		buf[len - 1] = '\0';
		wsize = 0;
		progs++;
		term = 0;
		while ((wsize = p1getword(buf,wsize,word)) != -1) {
			//put each word in line into progarm array
			programs[term] = p1strdup(word);
			//programs[term] = word;
			term++;
			//fork
			//call execvp on program
			//free each word in program array
		}
		//free(programs[term - 1]);
		programs[term] = NULL;
		pid = fork();
		if (pid == 0) {
			close(fd);
			execvp(programs[0], programs);
		}
			for (int j = 0; j < term; j++) {
				free(programs[j]);
		}
	}
/*
	for (int i = 0; i < place; i++) {
		pid[i] = fork();
		if (pid[i] == 0) {
			//close(fd);
			execvp(programs[i][0], programs[i]);
			exit(1);

			//execvp(programs,args[0]);
		}
		for (int j = 0; j < args[i]; j++)
		free(programs[i]);
}
*/

close(fd);
      for (int i = 0; i < progs; i++) {
	      wait(NULL);
      }
}

/*
 * If (child) {execvp(program[0], program)} else { loop free(program[I])} */




