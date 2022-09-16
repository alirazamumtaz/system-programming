#include "../include/builtin.h"
#include "../include/command.h"

#define PROMPT "mysh"

int main(){
   command_t **commands;
   int no_of_commands = 0;
   char* cmdline;
   char* prompt = PROMPT;
   read_history(".history.hist");
   while((cmdline = read_command(prompt)) != NULL){
      if((commands = parse(cmdline,&no_of_commands)) != NULL){
         for( int cno = 0; cno < no_of_commands; cno++){
            execute(commands[cno]);
            
            // close opened file descriptors
            if(commands[cno]->instream != 0)       close(commands[cno]->instream);
            else if(commands[cno]->pipein != -1)   close(commands[cno]->pipein);

            if(commands[cno]->outstream != 1)      close(commands[cno]->outstream);
            else if(commands[cno]->pipeout != -1)  close(commands[cno]->pipeout);
            
            if(commands[cno]->errorstream != 2)    close(commands[cno]->errorstream);
         }
      }
      // need to free pipeArray
      if(pipeCount > 0){
         for (int pno = 0; pno < pipeCount; pno++){
            free(pipeArray[pno]), pipeArray[pno] = NULL;
         }
         free(pipeArray), pipeArray = NULL;
      }
      
      // need to free commands_t structs
      for(int cno=0; cno < no_of_commands; cno++){
         if(commands[cno]->argv != NULL){
            for(int ano=0; ano < commands[cno]->argc; ano++){
               free(commands[cno]->argv[ano]), commands[cno]->argv[ano] = NULL;
            }
            free(commands[cno]->argv), commands[cno]->argv = NULL;
         }
         free(commands[cno]->argv), commands[cno]->argv = NULL; 
         free(commands[cno]), commands[cno] = NULL;
      }
      free(commands), commands = NULL;
      free(cmdline), cmdline = NULL;

  }//end of while loop

   printf("\n");
   return 0;
}
