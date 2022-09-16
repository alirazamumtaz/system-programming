#include "../include/builtin.h"
#include "../include/command.h"

char* builtinCmd[BUILTINS] = {"exit","cd", "jobs", "help"};
int (*builtinCmdPtr[BUILTINS])(int, char**) = {myexit, mycd, myjobs, myhelp};

extern int errno;

int isBuiltin(const char* cmd){
    for(int i = 0; i<BUILTINS; i++){
		if(!strcmp(builtinCmd[i],cmd)){
			return i;
		}
	}
	return -1;
}
int execBuiltin(int cmdNo, char **argv){
    return (*builtinCmdPtr[cmdNo])(cmdNo,argv);
}

int myexit(int argc, char** argv){
	write_history(".history.hist");
	exit(0);
}
int mycd(int argc, char** argv){
	char *buff = (char*)malloc(PATH_MAX);
	bzero(buff,PATH_MAX);
	if(argv[1] == NULL){ // handel 'cd' that will change the pwd to the home dir
		sprintf(buff,"%s",getenv("HOME"));
	}	
	else if(argv[1][0] == '~') // handle 'cd ~/Desktop' or any dir after the home dir
		sprintf(buff,"%s%s",getenv("HOME"),argv[1]+1);
	else
		sprintf(buff,"%s",argv[1]);
	chdir(buff);
	buff = getcwd(buff,PATH_MAX);
	if(errno == ERANGE && buff == NULL) return -1;
	setenv("PWD",buff,1);
	free(buff), buff = NULL;
	return 0;
}
int myjobs(int argc, char **argv){
	return system(argv[0]);
}
int myhelp(int argc, char **argv){
	puts("Sorry! I can't hlep you.\n");
	return 0;
}
