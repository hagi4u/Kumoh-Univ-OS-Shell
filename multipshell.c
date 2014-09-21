#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define STR_LEN 32
#define MAX_TOKENS 32
#define MAX_SIZE 1024

int exitCode = 0;		// Call by quit (Default = 0)
int cmdNumber;
char delim1[] = "|";
char delim2[] = " \n";

char cmdStr[STR_LEN];
char exitCheck = 0;
char *cmdTokenList[MAX_TOKENS];

int pdes[2];
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
	
  if(pipe(pdes)){
    die("pipe()");
  };
	
	if(argc == 1){ // interactiveMode
		while(1){
			isEnd = interactiveMode();
			if(isEnd){
				fputs("Exit MultiPShell..\n", stdout);
				break;
			}
		}
	} else if (argc == 2){
		//isEnd = batchMode(argv[1]);
		isEnd = 0;
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

	while(t.token1){
		t.divideToken[cCnt++] = t.token1;
		t.token1 = strtok(NULL, delim1);
			
	}
	t.divideToken[cCnt++] = NULL;

	if(cCnt == 1){
		printf("No commands\n");
		exit(1);
	}

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
		if((strcmp(t.afterToken[i][0] ,"quit") ==0 )){
			exitCheck = 1;
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

int interactiveMode(){
	char cmdStr[STR_LEN];
	int exitCode = 0;
	int status = 0;
	int i = 0;

	pid_t pid;

	fputs("prompt> ", stdout);
	fgets(cmdStr, STR_LEN, stdin);	
	
	tokenInit();
	cmdNumber = get_token(cmdStr);

	i = 0;

	
// 명령어를 실행하는 핵심코드
	while(i < cmdNumber) {
		
		pid = fork();
		if(pid == 0) {
			execvp(t.afterToken[i][0], t.afterToken[i]);
			exit(1);
			return 0;
		}
		i++;
	}

	i = 0;
// wait 함수.
	while(i < cmdNumber) {
		wait(&status);
		i++;
	}
	if(exitCheck == 1)
		exitCode = 1;

	return exitCode;	
}
/*
int batchMode(char **filepath){
	printf("HelloWorld From batchMode\n");
}*/
