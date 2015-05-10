

#include "job_control.h"   // remember to compile with module job_control.c 



#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */



job * job_list;







// -----------------------------------------------------------------------

//                            MAIN          

// -----------------------------------------------------------------------



int main(void)

{

	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */

	int background;             /* equals 1 if a command is followed by '&' */

	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */

	// probably useful variables:

	int pid_fork, pid_wait; /* pid for created and waited process */

	int status;             /* status returned by wait */

	enum status status_res; /* status processed by analyze_status() */

	int info;				/* info processed by analyze_status() */



	

	ignore_terminal_signals();//this way we make sure that our shell leaves the terminal to foreground tasks



	job_list = new_list("job list");



	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/

	{   		

		printf("COMMAND->");

		fflush(stdout);



		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

		

		if(args[0]==NULL) continue;   // if empty command

		

		if(strcmp(args[0],"cd")==0&&args[1]!=NULL){

			chdir(args[1]);

			continue;

		}

	

		// (1) fork a child process using fork()		



		pid_fork = fork(); 

		if (pid_fork < 0){ 

			printf("Error t fork\n");

			exit(-1);//then the shell exits

		}else if (pid_fork == 0) { //child

			//(2) the child process will invoke execvp()	

			// we have to create the child in an independent group

			pid_t mypid = getpid();

			new_process_group(mypid);

			if(!background){

				set_terminal(mypid);// the procces is in foreground so it has to be the unique owner of the terminal

			}

			restore_terminal_signals();//we restore the defaoult beheabour



			execvp(args[0], args);

			printf("the command %s wasnt executed in a right way  \n", args[0]);

			exit(-1);//execvp only returns if an error ocurr 

		}else{//father

			//(3) if background == 0, the parent will wait, otherwise continue 

			new_process_group(pid_fork);	

			if(!background){

				set_terminal(pid_fork);

			/* we make the father wait for the child in waitpid

				also we add the option WUNTRACED 

			*/

				pid_t wait_pid = waitpid(pid_fork, &status, WUNTRACED);//this way we take into account if the child proccess its suspended 

				set_terminal(getpid()); //the father retake the ownership of the terminal

				

				int info;

				enum status st = analyze_status(status, &info);

				//(4) Shell shows a status message for processed command 

				printf("Foreground pid: %d, command: %s, %s, info: %d\n", pid_fork, args[0], status_strings[st], info);

				fflush(stdout);

				

				if (st == SUSPENDED){

					

					job * aux = new_job(pid_fork, args[0],STOPPED);

					add_job(job_list, aux);

													

				}



			}else{ // Background process

				

				

				job * aux = new_job(pid_fork, args[0],BACKGROUND);

				add_job(job_list, aux);					

				printf("Background job runnning...pid: %d, command: %s\n", pid_fork, args[0]);

				fflush(stdout);



			}	



		}

			

	} // end whil

}

