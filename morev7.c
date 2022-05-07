
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

static int LINELEN, PAGELEN, TOTAL_LINES = 0;
static int pct;
static double res;
static struct termios old, current;
static char* file_name;//, filename;
   
long int search_in_file(char *, FILE *);
int is_io_redirected(pid_t);
void open_file_in_vim();
char getch(FILE*);
void initTermios(void);
void resetTermios(void);
void set_screensize(int*, int*);
void do_more(FILE *);
int  get_input(FILE*);
int get_percentage(FILE*);

int main(int argc , char *argv[]) {
   int i=0;
   if (argc == 1 && !is_io_redirected(getpid())){
      printf("more: bad usage\nTry 'more --help' for more information.\n");
      exit(1);
   }
   if(argc > 1){
   	file_name = argv[1];
   	argc--;
   	}
   FILE * fp;
   while(i < argc){
   
   if(argc > 2){
   	printf("::::::::::::::\n");
	printf("%s\n", argv[i]);
	printf("::::::::::::::\n");
   }
      fp = fopen(file_name , "r");
      if (fp == NULL){
	 perror("Can't open file");
	 exit (1);
      }
      fseek(fp, 0, SEEK_END);
      res = ftell(fp);
      fseek(fp,0,0);
      do_more(fp);
      fclose(fp);
      i++;
   }
   
   return 0;
}

void set_screensize(int * rows, int * cols){
   struct winsize wbuf;
   ioctl(0,TIOCGWINSZ,&wbuf);
   *cols = wbuf.ws_col;
   *rows = wbuf.ws_row - 1;
}

void do_more(FILE *fp){
   set_screensize(&PAGELEN,&LINELEN);
   int num_of_lines = 0;
   int rv, ret;
   char buffer[LINELEN];
   FILE* fp_tty = fopen("/dev//tty", "r");
   
   while (fgets(buffer, LINELEN, fp)){
      fputs(buffer, stdout);
      pct = get_percentage(fp);
      num_of_lines++;
      if (num_of_lines == PAGELEN){
         rv = get_input(fp_tty);
         if (rv == 0){//user pressed q ( to quit)
            printf("\033[2K \033[1G");
            break;//
         }
         else if (rv == 1){//user pressed space bar ( to print next page )
         // here again we have to set the screen size and also update
         // the number of lines to be displayed.
            set_screensize(&PAGELEN,&LINELEN);
            num_of_lines = 0;
            printf("\033[2K \033[1G");
         }
         else if (rv == 2){//user pressed return/enter ( to print next line )
            printf("\033[2K \033[1G");
	         num_of_lines -= 1; //show one more line
            }
         else if (rv == 3){ //invalid character
            printf("\033[2K \033[1G");
            break; 
         }
         else if (rv == 4){ // user passed v (to open file in vim)
            printf("\033[2K \033[1G");
            printf("file name %s\n",file_name);
            open_file_in_vim();
            num_of_lines -= 1;
            fseek(fp,-4,ftell(fp));
         }
         else if (rv == 5){ // user passed / (to search a string)
            printf("\033[2K \033[1G/");
            char search_string[50];
            int current_file_ptr = ftell(fp);
            fgets(search_string,50,stdin);
            long int pos = search_in_file(search_string,fp);
            if(pos == 0){ // if not found then reset the file pointer.
            	fseek(fp,-1,current_file_ptr);
            }
            if(pos != 0){	
            	fseek(fp,pos,SEEK_SET); // set file_ptr to the next of founded string line.
            	num_of_lines = 2;
            }
         }
      }
  }
}

int get_percentage(FILE* fp){
   fseek(fp, 0, SEEK_CUR);
   double pos = ftell(fp);
   return (pos/res)*100;
}

int get_input(FILE* cmdstream){
   char c;
   printf("\033[7m --more--(%d%%) \033[m",pct);
   // my modified getc function that have non-echo and non-canonical feature
   c = getch(cmdstream);
   if ( c == 'q' )   return 0;
   if ( c == ' ' )   return 1;
   if ( c == '\n' )  return 2;
   if ( c == 'v' )   return 4;
   if ( c == '/' )   return 5;
   return 3;
   return 0;
}

/* Read 1 character without echo and non-canonical mode */
char getch(FILE* cmdstream) {
  char ch;
  initTermios();
  ch = getchar();
  resetTermios();
  return ch;
}

/* Initialize new terminal i/o settings */
void initTermios(void) {
  ioctl(0,TCGETS, &old);         /* grab old terminal i/o settings */
  current = old;           /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  current.c_lflag &= ~ECHO;      /* set no echo mode */
  ioctl(0, TCSETS, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void){
  ioctl(0, TCSETS, &old);
}

/* Create a new child process and then open vim in it.*/
void open_file_in_vim(){
	printf("file name %s\n",file_name);
	char *args[] = { "/usr/bin/vim", file_name, NULL };
    pid_t ch_pid = fork();
    if (ch_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (ch_pid > 0) {
        int wstatus;
         if (waitpid(ch_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
           perror("waitpid");
           exit(EXIT_FAILURE);
         }
         return;
    } else {
        execvp(args[0], args);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    pause();
        return;
}

/* Search searh_string in file fp after the current file pointer. */
long int search_in_file(char * search_str, FILE * fp){
	int line_number = TOTAL_LINES;
	char buffer[LINELEN];
	char * pos;
	while(fgets(buffer, LINELEN, fp)){
		line_number++;
		pos = strstr(buffer,search_str);
		if(pos != NULL){ printf("...skipping\n%s",buffer);return ftell(fp);}
	}
	return 0;
}

/* Check wether the I/O is redirected or not. For this i have compared the opeded file descriptors in proc directory. */
int is_io_redirected(pid_t pid){
	char std[10] = "/dev/pts/";
	char proclink[64];
   sprintf(proclink, "/proc/%d/fd/%d", pid, 0);//get the link of the opened file from fd table..
   char filename[1024];
	ssize_t len;
	if ((len = readlink(proclink, filename, sizeof(filename)-1)) != -1) // read the link and if it is not terminated my NULL the manually do it.
		filename[len] = '\0';
	printf("file name %s\n",filename);
	file_name = filename;
	dup2(1,0); // get back the terminal from fd 1 to fd 0;
   if(strstr(filename,std))
   	return 0;
   else{
   	
   	return 1;
   }
}
