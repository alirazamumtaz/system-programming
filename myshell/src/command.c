#include "../include/command.h"
#include "../include/builtin.h"

char* read_command(char* prompt){
  char buff[256];
  sprintf(buff,"%s:%s$ ", prompt,getenv("PWD"));
  char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
  cmdline = readline(buff);
  if(strlen(cmdline) != 0) add_history(cmdline);
  return cmdline;
}

// for debuging pupose
void print_command(command_t* cmd){
   printf("****** COMMAND *********\n");
     printf("argc = %d\n",cmd->argc);
     for (int i = 0; i < MAXARGS && cmd->argv[i] != NULL; i++){
      printf("argv[%d] = %s\t",i,cmd->argv[i]);
     }
     printf("\ninstream = %d\n",cmd->instream);
     printf("outstream = %d\n",cmd->outstream);
     printf("errstream = %d\n",cmd->errorstream);
    //  printf("\npipedIn = %d\n",cmd->pipein);
    //  printf("pipedOut = %d\n",cmd->pipeout);
     
}

command_t* init_command(void){
  //allocate memory
  command_t* cmd = malloc(sizeof(command_t));
  cmd->argv = (char**)malloc(sizeof(char*)* (MAXARGS+1));
  for(int j=0; j < MAXARGS+1; j++){
    cmd->argv[j] = (char*)malloc(sizeof(char)* ARGLEN);
    bzero(cmd->argv[j],ARGLEN); //it allocates the memory to zeros
  }
  cmd->argc = 0;
  cmd->instream = 0, cmd->outstream = 1, cmd->errorstream = 2;
  cmd->pipein = 0, cmd->pipeout = 0;
  return cmd;
}


char* conditional_command(command_t* command){	
	char condition[ARGLEN] = {'\0'};
	char* result;
	for (int i=1; i< command->argc; i++){
		strcat(condition, command->argv[i]);
		strcat(condition, " "); // append space
	}

	for (int i=0; i< command->argc; i++)
		free(command->argv[i]), command->argv[i] = NULL;
	free(command->argv);

	char* true_state = read_command( "> ");
	char* false_state = read_command( "> ");
	char* fi_command;
  while(1){	
		fi_command = read_command( "> ");
		if (strstr(fi_command, "fi"))
			break;
	}

	int flag = system(condition); // check condition 0 for true 256 for false

	if (!flag)
	{
		char* check = "then\0";
		result = true_state;
		for (int i=0; i<strlen(check); i++)
			if (result[0] != check[i])
			{
				perror("parse error from then");
				return NULL;
			} else result++;
	}
	else
	{
		char* check = "else\0";
		result = false_state;
		for (int i=0; i<strlen(check); i++)
			if (result[0] != check[i])
			{
				perror("parse error from else");
				return NULL;
			} else result++;
	}

	return result;
}

char** split_string(char* string, int* count, char* seperator){
  int len = strlen(string);
  *count = 0;
  int i = 0;
  while (i < len) { 
    while (i < len){ // find strart occurance of the seperator in the string
      if (strchr(seperator, string[i]) == NULL)
        break;
      i++;
    }
    int old_i = i;
    while (i < len){ // find next occurance of the seperartor in the string
      if (strchr(seperator, string[i]) != NULL)
        break;
      i++;
    }
      // if found, count++;
    if (i > old_i) *count = *count + 1;
  }
   // initilize array of seperated strings
  char **strings = malloc(sizeof(char *) * (*count + 1));
  i = 0;
  char buffer[16384];
  int string_index = 0;
  while (i < len){
    while (i < len){ // again find strart occurance
      if (strchr(seperator, string[i]) == NULL)
        break;
      i++;
    }
    int j = 0;
    while (i < len){ // find next occurance
      if (strchr(seperator, string[i]) != NULL){
        break;
      }
      buffer[j] = string[i];
      i++;
      j++;
    }
    if (j > 0){
      buffer[j] = '\0';

      int to_allocate = sizeof(char) * (strlen(buffer) + 1);
      
      strings[string_index] = malloc(to_allocate);
      
      strcpy(strings[string_index], buffer);
      
      string_index++;
    }
  }
  strings[string_index] = NULL;
  return strings;
}

void set_pipe(command_t* command, int j, int pipe_sperterated_count){
  if(pipe_sperterated_count == 1 && j == 0){ // single command
    command->pipein = -1;
    command->pipeout = -1;
  }
  else if(pipe_sperterated_count > 1 && j == 0 ){ // first command in chain
    command->pipein = -1;
    command->pipeout = pipeArray[j][1];
  }
  else if(j == pipe_sperterated_count - 1){ // last command in chain
    command->pipein = pipeArray[j-1][0];
    command->pipeout = -1;
  }
  else { // commands in between chain
    command->pipein = pipeArray[j-1][0];
    command->pipeout = pipeArray[j][1];
  }
}

void set_ioredirect(command_t* command, int j, int pipe_sperterated_count){
  // print_command(command);

  for (int i = 0;i < command->argc; i++){
    if( (strstr(command->argv[i],">")) || (strstr(command->argv[i],"1>"))){
      printf("redirecting output to %s\n",command->argv[i+1]);
      command->outstream = open(command->argv[i + 1], O_RDWR|O_CREAT, S_IRWXU|S_IROTH);
      command->argv[i] = NULL;
      command->argc -= 2;
    }
    else if((strstr(command->argv[i],"2>"))){
      command->errorstream = open(command->argv[i + 1], O_RDWR|O_CREAT, S_IRWXU|S_IROTH);
      command->argv[i] = NULL;
      command->argc -= 2;
    }
    else if( (strstr(command->argv[i],"<")) || (strstr(command->argv[i],"0<"))){
      command->instream = open(command->argv[i+1], O_RDONLY);
      command->argv[i] = NULL;
      command->argc -=2;
    }
  }
}

command_t** parse(char* cmdline, int* no_of_commands){
  int term_sperterated_count = 0, pipe_sperterated_count = 0;
  pipeCount = 0;
  int number_of_processes = 0;
  char** pipe_sperterated;
  char** term_sperterated = split_string(cmdline,&term_sperterated_count, ";");
  for (int i = 0; i < term_sperterated_count; i++){
    pipe_sperterated = split_string(term_sperterated[i],&pipe_sperterated_count, "|");
    pipeCount += pipe_sperterated_count - 1;
    for (int j = 0; j < pipe_sperterated_count; j++){
        number_of_processes++; // += 1;
    }
  }
  // printf("PIPE = %d\n",pipeCount);
   // initialize pipe array to manege pipes
  if(pipeCount > 0){
    pipeArray = (int**)malloc(sizeof(int*) * pipeCount);
    for(int i = 0; i < pipeCount; i++){
      pipeArray[i] = (int*)malloc(sizeof(int) * 2);
      pipe(pipeArray[i]);
    }
  }

  command_t** commands = malloc(sizeof(command_t*)*(number_of_processes+1));
  for(int idx = 0; idx < number_of_processes; idx++)
    commands[idx] = init_command();
  commands[number_of_processes] = NULL;

// printf("number of processes = %d\n",number_of_processes);
  char** ioseperated, ioseperated_count = 0;
  int cmd_no = 0;
  for (int i = 0; i < term_sperterated_count; i++){
    pipe_sperterated = split_string(term_sperterated[i],&pipe_sperterated_count, "|");
    // int in = 0, out = 1, err = 2;

    for (int j = 0; j < pipe_sperterated_count; j++, cmd_no++){
      commands[cmd_no]->argv = split_string(pipe_sperterated[j],&commands[cmd_no]->argc," ");
      
      //replace $envvariable with it's value
      char *dollarPos;
      for(int pos = 0; pos < commands[cmd_no]->argc; pos++){
        dollarPos = strchr(commands[cmd_no]->argv[pos],'$');
        if(dollarPos != NULL){
          char* buff;
          buff = getenv(dollarPos+1);
          if(buff == NULL)  break;
          int temp;
          for (temp = 0;buff[temp] != '\0' ; temp++){
            commands[cmd_no]->argv[pos][temp] = buff[temp];
          }
          // for (int temp_new = temp; temp_new < ARGLEN; temp_new++){
            commands[cmd_no]->argv[pos][temp] = '\0';
          // }
        }
      }
      // set pipe values
      set_pipe(commands[cmd_no],j,pipe_sperterated_count);
      
      // set io redirect
      set_ioredirect(commands[cmd_no],j,pipe_sperterated_count);

      // print_command(commands[cmd_no]);

    }
  }

  *no_of_commands = number_of_processes;
  return commands;
  //  return NULL;
}

int execute(command_t *cmd){
  if (strstr(cmd->argv[0], "=")){
    cmd->argv = split_string(cmd->argv[0], &cmd->argc, "=");
    setenv(cmd->argv[0], cmd->argv[1], 1); // 3rd argument 1 means it should be overwritten
    return 0;
  }

  int status, rv;
  rv = isBuiltin(cmd->argv[0]);
  if(rv != -1){
  execBuiltin(rv, cmd->argv);
  if(rv == 0)	return 0;
  }
  else{
  int cpid = fork();
  switch(cpid){
      case -1:
        perror("fork failed: ");
        exit(1);
      case 0:
        if(cmd->instream != 0)      dup2(cmd->instream, 0);
        else if(cmd->pipein != -1)  dup2(cmd->pipein, 0);
        
        if(cmd->outstream != 1)     dup2(cmd->outstream, 1);
        else if(cmd->pipeout != -1) dup2(cmd->pipeout, 1);

        if(cmd->errorstream != 2)   dup2(cmd->errorstream, 2);


        execvp(cmd->argv[0], cmd->argv);
        perror("command 404: ");
        exit(1);
      default:
        waitpid(cpid, &status, 0);
        // 	printf("child exited with status %d \n", status >> 8);
        return status;
    }
  }
  return 0;
}
