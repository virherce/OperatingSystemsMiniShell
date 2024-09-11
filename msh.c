//P2-SSOO-23/24

//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 100 /*We wrote the code so it accedpted any number of commands*/
#define STUPID 0



// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

/* myhistory */

/* myhistory */

struct command
{
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;
};

int history_size = 20;
struct command * history;
int n_elem = 0;

void free_command(struct command *cmd)
{
    if((*cmd).argvv != NULL)
    {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++)
        {
            for (argv = *(*cmd).argvv; argv && *argv; argv++)
            {
                if(*argv){
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd)
{
    int num_commands = 0;
    while(argvv[num_commands] != NULL){
        num_commands++;
    }

    for(int f=0;f < 3; f++)
    {
        if(strcmp(filev[f], "0") != 0)
        {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else{
            strcpy((*cmd).filev[f], "0");
        }
    }

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands; i++)
    {
        int args= 0;
        while( argvv[i][args] != NULL ){
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j<args; j++)
        {
            (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]),sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j] );
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}
    /**/

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

	history = (struct command*) malloc(history_size *sizeof(struct command));
	int run_history = 0;

	while (1) 
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history){
            run_history=0;
        }
    else{
        // Prompt 
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if( end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        }
        else if( end != 0 && executed_cmd_lines == end)
            return 0;
        else
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
    }
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/
	    if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
            else if(strcmp(argvv[0][0],"mycalc")==0 && argvv[0][1]!=NULL && argvv[0][2]!=NULL && argvv[0][3]!=NULL){
                //check that the arguments of the command mycalc are correct
                //it is necessary to convert the operators into integers (the commands and its arguments are strings)
                int oper1=atoi(argvv[0][1]);
                int oper2= atoi(argvv[0][3]);
                char aux_acc[100]; //Acc environment variable (we assume that the sum won't exceed 100 chars)

                // addition
                if (strcmp(argvv[0][2], "add")==0){   
                    snprintf(aux_acc, 100, "%d", atoi(aux_acc)+oper1+oper2); //new value of the environment value Acc
                    if (setenv("Acc",aux_acc,1) != 0){
                        perror("Error setting the environment variable!\n");
                    }
                    //update the value of Acc with the new value stored in aux_acc
                    //save the result
                    fprintf(stderr, "[OK] %d+%d=%d; Acc %s\n", oper1, oper2, oper1+oper2, getenv("Acc"));
                }

                //multiplication
                else if (strcmp(argvv[0][2], "mul")==0){ 
                    // now we only have to save it in result 
                    fprintf(stderr,"[OK] %d*%d=%d\n", oper1,oper2, oper1*oper2);
                }

                //division
                else if (strcmp(argvv[0][2],"div")==0){
                    //consider the case in which the divisor is 0
                    if (oper2==0){
                        fprintf(stdout, "[ERROR]Division by 0 is not possible\n");  
                    }
                    else{
                        //again store in result
                        fprintf(stderr, "[OK] %d / %d=%d ; Remainder %d\n", oper1, oper2, oper1/oper2, oper1%oper2);
                    }
                }

                else{
                    //error regarding the command
                    //save again the message in the variable result
                    fprintf(stdout, "[ERROR] The structure of this command is mycalc <operand_1> <add/mul/div> <operand_2>\n"); 
                }
            }else if(strcmp(argvv[0][0],"myhistory") == 0){
                if(argvv[0][1] == NULL){
                /*Show the last 20*/
                    for(int i = 0;i < n_elem;i++){
                        fprintf(stderr,"%d ",i);
                        for(int j = 0;j < history[n_elem-i-1].num_commands;j++){
                            fprintf(stderr,"%s ", history[n_elem-i-1].argvv[j][0]);
                            for(int l = 1;l < history[n_elem-i-1].args[j];l++){
                                fprintf(stderr,"%s ", history[n_elem-i-1].argvv[j][l]);
                            }
                            if(history[n_elem-i-1].num_commands > 1 & j != history[n_elem-i-1].num_commands - 1){
                                fprintf(stderr,"| ");
                            }

                        }
                        /*checkear file y background*/
                        if(strcmp(history[n_elem-i-1].filev[0],"0") != 0){
                            fprintf(stderr,"< %s ",history[n_elem-i-1].filev[0]);
                        }
                        if(strcmp(history[n_elem-i-1].filev[1],"0") != 0){
                            fprintf(stderr,"> %s ",history[n_elem-i-1].filev[1]);
                        }
                        if(strcmp(history[n_elem-i-1].filev[2],"0") != 0) {
                            fprintf(stderr,"!> %s ", history[n_elem-i-1].filev[2]);
                        }
                        if(history[n_elem-i-1].in_background == 1){
                            fprintf(stderr,"&");
                        }
                        
                        fprintf(stderr,"\n");
                    }
                }else{
                    /*Execute the command number argvv[0][1]*/
                    int index = atoi(argvv[0][1]);
                    if(index < 0 || index > 19){
                        fprintf(stdout,"ERROR: Command not found");
                    }else{
                        fprintf(stderr,"Running command %d\n",index);
                        for (int j=0; j < history[n_elem-index-1].num_commands; j++){
                            getCompleteCommand(history[n_elem-index-1].argvv, j);
                            execvp(argv_execvp[0], argv_execvp);
                        }
                    }
                }
            }else {
				// Print command
                pid_t pid;
                int fd0, fd1, fd2;
                int fdpipe[command_counter-1][2];

                for (int i=0; i<command_counter; i++){
                    if(command_counter > 1 && i<command_counter-1){
                        //We need to create a pipe if there are more than one command except for the last one
                        if (pipe(fdpipe[i])<0){
                            perror("\nError creating the pipe");
                            exit(EXIT_FAILURE);
                        }
                    }
                    pid = fork();
                    switch(pid){
                        case -1: /* error */
                            perror ("\nError in fork");
                        case 0: /* child */
                            /*We redirect the error for all different commands*/
                            if(strcmp(filev[2], "0") != 0){
                                    //The error is included here since it has to affect all commands
                                    fd2 = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                    if (fd2<0){
                                        perror("\nError opening the error file");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(STDERR_FILENO)<0) {
                                        perror("\nError closing the standard error");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup(fd2)<0) {
                                        perror("\nError with dup");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(fd2)<0) {
                                        perror("\nError closing the new error file");
                                        exit(EXIT_FAILURE);
                                    }
                            }
                            getCompleteCommand(argvv, i); //We need this to execute the command
                            if(i==0){
                                //First command
                                //We check whether the input fd needs to be modified
                                if (strcmp(filev[0], "0") != 0){
                                    fd0 = open(filev[0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                    if (fd0<0){
                                        perror("\nError opening the input file");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(STDIN_FILENO)<0) {
                                        perror("\nError closing the standard input");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup(fd0)<0) {
                                        perror("\nError with dup");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(fd0)<0) {
                                        perror("\nError closing the old fd for new input file");
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                //Two options for our output
                                if (command_counter>1){
                                    //We need to change the output to the pipe
                                    if(close(STDOUT_FILENO)<0) {
                                        perror("\nError closing");
                                        exit(EXIT_FAILURE);
                                    }
                                    if(dup(fdpipe[i][1])<0) {
                                        perror("\nError with dup");
                                        exit(EXIT_FAILURE);
                                    }
                                    if(close(fdpipe[i][1])<0) {
                                        perror("\nError closing");
                                        exit(EXIT_FAILURE);
                                    }
                                }else{
                                    //We check whether the output file needs to be modified for a single command
                                    if (strcmp(filev[1], "0") != 0){
                                        fd1 = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                        if (fd1<0){
                                            perror("\nError opening the output file");
                                            exit(EXIT_FAILURE);
                                        }
                                        if (close(STDOUT_FILENO)<0) {
                                            perror("\nError closing the standard output");
                                            exit(EXIT_FAILURE);
                                        }
                                        if (dup(fd1)<0) {
                                            perror("\nError with dup");
                                            exit(EXIT_FAILURE);
                                        }
                                        if (close(fd1)<0) {
                                            perror("\nError closing");
                                            exit(EXIT_FAILURE);
                                        }
                                    }
                                }
                            }else if(i<command_counter-1){
                                //If our command is btwn the first and the last one
                                if (close(STDIN_FILENO)<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }
                                if (dup(fdpipe[i-1][0])<0) {
                                    perror("\nError with dup");
                                    exit(EXIT_FAILURE);
                                }
                                if (close(fdpipe[i-1][0])<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }
                                if (close(STDOUT_FILENO)<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }
                                if (dup(fdpipe[i][1])<0) {
                                    perror("\nError with dup");
                                    exit(EXIT_FAILURE);
                                }
                                if (close(fdpipe[i][1])<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }
                            }else{
                                //If our command is the last one
                                //We check whether the output file needs to be modified for a single command
                                if (strcmp(filev[1], "0") != 0){
                                    fd1 = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                    if (fd1<0){
                                        perror("\nError opening the output file");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(STDOUT_FILENO)<0) {
                                        perror("\nError closing the standard output");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup(fd1)<0) {
                                        perror("\nError with dup");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (close(fd1)<0) {
                                        perror("\nError closing");
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                if (close(STDIN_FILENO)<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }
                                if (dup(fdpipe[i-1][0])<0) {
                                    perror("\nError with dup");
                                    exit(EXIT_FAILURE);
                                }
                                if (close(fdpipe[i-1][0])<0) {
                                    perror("\nError closing");
                                    exit(EXIT_FAILURE);
                                }

                            }
                            execvp(argv_execvp[0], argv_execvp);

                        default: /* parent */
                            if (i<command_counter-1){
                                close(fdpipe[i][1]);
                            }
                            if (i>0){
                                close(fdpipe[i-1][0]);
                            }
                            if (in_background==0){
                                wait(NULL);
                            }else{
                                //We execute the process in background
                                if (i==command_counter-1){
                                    printf("[%d]\n", pid);
                                }
                            }
                    }
                }
            }
            if (n_elem>=19){
                int j = 19;
                //We need to move all the elements in history one position and this makes the last one dissapear
                while(j>0){
                    history[j]=history[j-1];
                    j--;
                }
                fprintf(stderr, "He sido capaz de cambiar los comandos\n");
                free_command(history); // Because this frees the first elem in history
                fprintf(stderr, "He sido capaz de liberar el primer elemento\n");
                store_command(argvv, filev, in_background, history); //We store the newly introduced elem as the first elem in history
                fprintf(stderr, "He sido capaz de storear el nuevo command\n");
            }/*else if(n_elem==19){
                //This is the last command/sequence that will be accepted without needing to free space
                int j = 19
                while(j>0){
                    history[j]=history[j-1];
                    j--;
                }
            }*/
            else{
                if (n_elem==0){
                    store_command(argvv, filev, in_background, history);
                }else{
                    int j = n_elem;
                    while(j>=0){
                        history[j+1]=history[j];
                        j--;
                    }
                    store_command(argvv, filev, in_background, history);
                }
                n_elem += 1;
            } 
		}
	}
}
	

