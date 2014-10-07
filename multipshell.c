#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define STR_LEN 32
#define MAX_TOKENS 32
#define MAX_SIZE 1024

int exitCode = 0;		// Call by quit (Default = 0)
int cmdCnt;
char delim1[] = "|";
char delim2[] = " \n";

char cmdStr[STR_LEN];
char exitCheck = 0;
char *cmdTokenList[MAX_TOKENS];

int interactiveMode();
int batchMode(char * filepath);
int get_token(char * cmd);

typedef struct token {
	char *divideToken[MAX_SIZE];
	char *afterToken[MAX_SIZE][MAX_SIZE];
	char *token1;
	char *token2;
} token;
token t;

void die(const char *msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
	int isEnd;
	if(argc == 1){ // interactiveMode
		while(1){
			isEnd = interactiveMode();
			if(isEnd){
				fputs("Exit MultiPShell..\n", stdout);
				break;
			}
		}
	} else if (argc == 2){
		isEnd = batchMode(argv[1]);
		if(isEnd)
			return 0;
	} else {
		fprintf(stderr, "Invalid commands.\nCheck your commands.\n");
	}
	return 0;
}

int get_token(char *cmd){
	int cCnt = 0;
	int tNum = 0;
	int i = 0;
	
	t.token1 = strtok(cmd, delim1);
	if(t.token1 == NULL){
		return 0;
	}

	while(t.token1){
		t.divideToken[cCnt++] = t.token1;
		t.token1 = strtok(NULL, delim1);			
	}
	
	t.divideToken[cCnt++] = NULL;

	while(i < cCnt) {
		t.token2 = strtok(t.divideToken[i], delim2);
		while(t.token2){
			t.afterToken[i][tNum++] = t.token2;
			t.token2 = strtok(NULL, delim2);
		}
		tNum = 0;
		i++;
	}

	for(i=0; i < cCnt - 1; i++){
		if(!strcmp(t.afterToken[i][0], "quit")){
			return 0;
		} 
	}
	
	return cCnt;
}

void tokenInit(){	
	int i = 0, j=0;

	for( i = 0; i < MAX_SIZE; i++) 
		t.divideToken[i] = NULL;

	for( i = 0; i < MAX_SIZE; i++) 
		for( j = 0; j < MAX_SIZE; j++) 
			t.afterToken[i][j] = NULL;
}

int cmdProc(int cnt){
	int i=0, status = 0;
	int pdes[2];
	pid_t pid;
/*
  cnt - 1 = 1 이면 Pipe X
  i == cnt - 1 이면 output Pipe x 
*/
	if(pipe(pdes)){
                die("pipe()");
                exit(0);
        };
	
	while(i < (cnt - 1)) {
		pid = fork();
		if(pid < 0)
			die("fork()!!");
    
		if(pid == 0) { // isChild
			if( i == 0 ){
				if((cnt-1) != 1){
					close(pdes[0]);
					if(dup2(pdes[1], 1) == -1)
						die("dup2()");
					close(pdes[1]);
				}
			}
			if(i == 1){
				close(pdes[1]);
				if(dup2(pdes[0],0) == -1)
                                	die("dup2()");
                        	close(pdes[0]);
			}
			execvp(t.afterToken[i][0], t.afterToken[i]);
			_exit(1);
			return 0;
		}
		i++; 
  	}
	wait(&status);
	usleep( 7000 );
	return 0;
}

int interactiveMode(){
	char cmdStr[STR_LEN];
	int exitCode = 0;
	
	fputs("prompt> ", stdout);
	fgets(cmdStr, STR_LEN, stdin);	
	
	if((int)strlen(cmdStr) == 1){
	  return exitCode;
	}
	tokenInit();
  	if(!(cmdCnt = get_token(cmdStr))){
  	 	exitCode = 1;
		return exitCode;
 	 }
  	cmdProc(cmdCnt);

	return exitCode;	
}

int batchMode(char *filepath){
	char cmdStr[STR_LEN];
	FILE *fp;
	int cmdCnt = 0;
	int i = 0;

	fp = fopen(filepath, "r");
	if(fp == NULL) {
		printf("파일이 존재하지 않습니다.\n");
		exit(1);;
	}
	
	while(1){
		if(fgets(cmdStr, sizeof(cmdStr), fp) == NULL){
			break;
		}
		printf("%d %s", i+1, cmdStr);
		if(!(cmdCnt = get_token(cmdStr))) {
			exitCode = 1;
			return exitCode;
		};
		cmdProc(cmdCnt);
		i++;
	}

	fclose(fp);
}
