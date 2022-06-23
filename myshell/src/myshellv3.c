/******************
 *   myshellv3.c: *
 ******************
 *   complete version2 and some extra functionalities of I/O redirection
 *   and use of pipe | for commands
 *
 *  Limitations:
 *   if user press enter without any input the program gives sigsegv 
 *   if user give only spaces and press enter it gives sigsegv
 *   if user press ctrl+D it give sigsegv
 *   however if you give spaces and give a cmd and press enter it works
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "mysh"
#define BUILTINS 2

int execute(char* arglist[]);
extern int errno;
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
const char* builtinCmd[BUILTINS] = {"exit","cd"};

int myexit(int argc, char** argv){
	printf("myexit\n");
	exit(argc);
}

int mycd(int argc, char** argv){
	int len = strlen(argv[1]);
	char *buff = (char*)malloc(PATH_MAX);
	bzero(buff,PATH_MAX);
	if(argv[1] == NULL){ // handel 'cd' that will change the pwd to the home dir
		sprintf(buff,"%s",getenv("HOME"));
		printf("Hi, %s\n",buff);
	}	
	if(argv[1][0] == '~') // handle 'cd ~/Desktop' or any dir after the home dir
		sprintf(buff,"%s%s",getenv("HOME"),argv[1]+1);
	else
		sprintf(buff,"%s",argv[1]);
	chdir(buff);
	buff = getcwd(buff,PATH_MAX);
	if(errno == ERANGE && buff == NULL) return -1;
	setenv("PWD",buff,1);
	free(buff);
	return 0;
}

const int (*builtinCmdPtr[BUILTINS])(int, char**) = {myexit, mycd};

int execBuiltin(int i,int argc, char **argv){
	int rv =  (*builtinCmdPtr[i])(i,argv);
	return 0;
}

int isBuiltin(const char* cmd){
	for(int i = 0; i<BUILTINS; i++){
		if(!strcmp(builtinCmd[i],cmd)){
			return i;
		}
	}
	return -1;
}

int main(){
   char *cmdline;
   char** arglist;
   char* prompt = PROMPT;   
   while((cmdline = read_cmd(prompt,stdin)) != NULL){
      if((arglist = tokenize(cmdline)) != NULL){
            execute(arglist);
       //  need to free arglist
         for(int j=0; j < MAXARGS+1; j++)
	         free(arglist[j]);
         free(arglist);
         free(cmdline);
      }
  }//end of while loop
   printf("\n");
   return 0;
}
int execute(char* arglist[]){
   int status, rv;
   rv = isBuiltin(arglist[0]);
   if(rv != -1){
	execBuiltin(rv, arglist);
	if(rv == 0)	return 0;
   }
   else{
   	int cpid = fork();
   	switch(cpid){
      	case -1:
         	perror("fork failed");
	      	exit(1);
      	case 0:
		execvp(arglist[0], arglist);
 	      	perror("Command not found...");
	      	exit(1);
      	default:
	      	waitpid(cpid, &status, 0);
         // 	printf("child exited with status %d \n", status >> 8);
         	return 0;
   	}
   }
}
char** tokenize(char* cmdline){
//allocate memory
   char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
   for(int j=0; j < MAXARGS+1; j++){
	   arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      bzero(arglist[j],ARGLEN); //it allocates the memory to zeros
    }
   if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
      return NULL;
   int argnum = 0; //slots used
   char*cp = cmdline; // pos in string
   char*start;
   int len;
   while(*cp != '\0'){
      while(*cp == ' ' || *cp == '\t') //skip leading spaces
          cp++;
      start = cp; //start of the word
      len = 1;
      //find the end of the word
      while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
         len++;
      strncpy(arglist[argnum], start, len);
      arglist[argnum][len] = '\0';
      argnum++;
   }
   arglist[argnum] = NULL;
   return arglist;
}      

char* read_cmd(char* prompt, FILE* fp){
   printf("%s:%s$ ", prompt,getenv("PWD"));
  int c; //input character
   int pos = 0; //position of character in cmdline
   char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
   while((c = getc(fp)) != EOF){
       if(c == '\n')
	  break;
       cmdline[pos++] = c;
   }
//these two lines are added, in case user press ctrl+d to exit the shell
   if(c == EOF && pos == 0) 
      return NULL;
   cmdline[pos] = '\0';
   return cmdline;
}
