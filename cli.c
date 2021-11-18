/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <Printer.h>
#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"
#include <sys/types.h>
#include <sys/wait.h>
#include<unistd.h>
#include <fcntl.h>

static JOB *jobarray[MAX_JOBS];
pid_t child;

void handler(int sig){
    int test;
    waitpid(child,&test,0);
}

void sigterm(int sig){

}

void sigstop(int sig){

}

void sigcont(int sig){

}



void fork_print(JOB *job, PRINTER* printer,char **a){
    printer->status = PRINTER_BUSY;
    sf_printer_status(printer->name,PRINTER_BUSY);
    job->status = JOB_RUNNING;
     sf_job_status(job->id,JOB_RUNNING);
    signal(SIGCHLD,handler);
    int pid = fork();
    if(pid > 0  ){ //master process
        signal(SIGCONT,sigcont);
        int pidfortesting = getpid();

        sf_job_started(job->pgid,printer->name,getpgid(pid),a);
        setpgid(getpgid(pidfortesting),pidfortesting);




    if(pid == 0){ //child
        int openn = open(job->filename,O_RDONLY);
        int close = imp_connect_to_printer(printer->name,printer->type,PRINTER_NORMAL);
        if(close != -1){
            char *cat[] = {"/bin/cat",NULL};
            dup2(openn,0);
            dup2(close,1);
            if(execv("/bin/cat",cat) != -1){
                exit(EXIT_SUCCESS);
            }
            else{
                exit(EXIT_FAILURE);
            }



        }


    }
    else{
        int testing2 = 0;
        waitpid(child,&testing2,0);
        testing2 = 0;
        if(WIFEXITED(testing2) != EXIT_SUCCESS){
            exit(EXIT_FAILURE);

        }
        else{
            exit(EXIT_SUCCESS);
        }

    }
}
else{
    printer->status= PRINTER_IDLE;
    sf_printer_status(printer->name,PRINTER_IDLE);
    int testing3 = 0;
    if(WIFEXITED(testing3) != EXIT_SUCCESS){
        job->status = JOB_FINISHED;
        sf_job_finished(job->id,waitpid(child,&testing3
            ,0));
    }
    else{
        job->status  = JOB_ABORTED;
        sf_job_aborted(job->id,waitpid(child,&testing3,0));
    }




}


 }


JOB* create_job(char *file,char *type,int printerset){
    char* filee = malloc(strlen(file)+1);
    memcpy(filee,file,strlen(file)+1);

    char *typee = malloc(strlen(type)+1);
    memcpy(typee,type,strlen(type)+1);
    JOB* first = malloc(sizeof(JOB));
    for(int i =0; i < MAX_JOBS;i++){
        if(jobarray[i] ==0 ){
            first->id = i;
            break;
        }

    }
    first->status = JOB_CREATED;
    first->filename = file;
    first->filetype = type;
    first->pgid = 0;
    first->printersset = printerset;
    sf_job_created(first->id,file,type);
    for(int i =0; i < MAX_JOBS;i++){
        if(jobarray[i]== NULL){
            jobarray[i]=first;
            break;
        }

    }
    return first;
}
int run_cli(FILE *in, FILE *out)
{
    int exit = 1;
    while(exit){
    char *value = sf_readline("imp> ");
    int i =0;
    char *result = strtok(value," ");
    char* array[20];
    int allargsize = 0;
    static PRINTER *printers[MAX_PRINTERS];
    memset(array,0,sizeof(array));
     	while (result != NULL){
     		array[i++] = result;
     		result = strtok(NULL," ");
     		allargsize = allargsize + 1;
     	} // FILLS ARRAY WITH THE INPUT VALUES



    if (array[0] == 0){
        array[0] = "noneentered";

    }
    else if(!strcmp(array[0],"help")){
        if(array[1] != 0 || array[2] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");
        } //if supplies more than help as an argument.
        else{
    	printf("Available Commands:\n");
    	printf("MISCELLANEOUS COMMANDS\n");
    	printf("help\n");
    	printf("quit\n");
    	printf("CONFIGURATION COMMANDS\n");
    	printf("type\n");
    	printf("printer\n");
    	printf("conversion\n");
    	printf("INFORMATIONAL COMMANDS\n");
    	printf("printers\n");
    	printf("jobs\n");
    	printf("SPOOLER COMMANDS\n");
    	printf("print\n");
    	printf("cancel\n");
    	printf("pause\n");
    	printf("resume\n");
    	printf("disable\n");
    	printf("enable\n");
         sf_cmd_ok(); // call command to show command is done
     }
    }


     else if(!strcmp(array[0],"quit")){
         if(array[1] != 0 || array[2] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");
        } //if supplies more than help as an argument.
        else{
        free(value);
     	return 0;
     }
     }

    else if(!strcmp(array[0],"type")){
        if(array[1] == 0){
            printf("FILE TYPE NOT DEFINED\n");
            sf_cmd_error("FILE TYPE NOT DEFINED");

        }
        else if(array[2] != 0 || array[3] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");

        }
     	else{
            if(define_type(array[1]) == NULL){
     		printf("Could not save type, please enter a new one.\n");
            sf_cmd_error("FILE TYPE NOT DEFINED");}
            else{
                sf_cmd_ok();
            }
     	}




     }

     else if(!strcmp(array[0],"printer")){
        if(array[1] == 0 || array[2] == 0){ // if they dont give a printer or a other thing
            printf("PRINTER COMMAND NOT ENTERED CORRECTLY PLEASE ENTER A PRINTERNAME AND FILE TYPE.\n");
             sf_cmd_error("PRINTER COMMAND NOT ENTERED CORRECTLY PLEASE ENTER A PRINTERNAME AND FILE TYPE.");
        }
        else if(array[3] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");
        }
     	else {
        if(find_type(array[2]) != NULL){
     	char *name = malloc(strlen(array[1])+1);
     	memcpy(name,array[1],strlen(array[1])+1);
     	char* type = malloc(strlen(array[2])+1);
     	memcpy(type,array[2],strlen(array[2])+1);

     	PRINTER *p = malloc(sizeof(PRINTER));// size of max printers

     	p->name = name;
     	p->type = type;
     	p->status = PRINTER_DISABLED;
        sf_printer_defined(name,type);
     	for  (int i = 0; i < MAX_PRINTERS;i++){
     		if(printers[i] == NULL){
     			printers[i] = p;
     			break;
     		}
     	}

        sf_cmd_ok();
     }

     else{
     	printf("INVALID FILE TYPE.\n");
        sf_cmd_error("INVALID FILE TYPE");

     }}}

     else if(!strcmp(array[0],"conversion")){
        if(array[1] == 0 || array[2] == 0 || array[3] == 0){ // required args not filled in
            printf("CONVERSION REQURES AT LEAST 3 ARGS. Refer to HELP for more information");
            sf_cmd_error("CONVERSION REQURES AT LEAST 3 ARGS. Refer to HELP for more information");
        }




     else if(find_type(array[1]) == NULL || find_type(array[2]) == NULL){ //if any of the files are not found
     	printf("Files not found.\n");
         sf_cmd_error("Files not found.");
     }

     else{
     int argcounter = 0;
     for (int i =4; i<allargsize ; i++){
     	if(array[i] != NULL)
     		argcounter = argcounter + 1; //loop through token array to check if there are any extra args if so keep a count of how many
   		else{
   			break;
   		}
     }
     char* optionsargs[argcounter+2]; // size of additional args + 1
     if(argcounter > 0){ //if there are additional args
     int argindex = 4; //starting index for additional args
     for(int i =0;i<argcounter;i++){
     	optionsargs[i] = array[argindex];
     	argindex++; //increment arg index
     }
	 }
     optionsargs[0] = array[3];
     char *from = malloc(strlen(array[1])+1);
     memcpy(from,array[1],strlen(array[1])+1);
     char *to = malloc(strlen(array[2])+1);
     memcpy(to,array[2],strlen(array[2])+1);
     optionsargs[argcounter + 1]= NULL;
     CONVERSION *converting = define_conversion(from,to,optionsargs);
     sf_cmd_ok(); // CONVERSION IS MADE
     if(converting == NULL){
     	printf("CONVERSION UNSUCESSFULL\n");
        sf_cmd_error("CONVERSION UNSUCESSFULL");
     }
 }
	}

	else if(!strcmp(array[0],"printers")){
        if(array[1] != 0 || array[2] != 0){
            printf("TOO MANY ARGUMENTS");
            sf_cmd_error("TOO MANY ARGS");
        }
        else{
		for(int i =0; i< MAX_PRINTERS ;i++){
			if(printers[i] == 0){
				break;
			}
			printf("PRINTER: id = %d,name = %s,type = %s,status = %u \n",i,printers[i]->name,printers[i]->type,printers[i]->status);
		}
        sf_cmd_ok();// PRINTED ALL THE PRINTERS!
    }
	}

	else if(!strcmp(array[0],"jobs")){
        if(array[1] != 0 || array[2] != 0){
            printf("TOO MANY ARGUMENTS\n");
            sf_cmd_error("TOO MANY ARGS");
        }
        for(int i =0; i< MAX_JOBS ;i++){
            if(jobarray[i] == 0){
                break;
            }
            printf("JOBS[%d]: type = %s,creation = %d,status = %u, eligible = %d, file = %s \n",jobarray[i]->id,jobarray[i]->filetype,jobarray[i]->creationtime,jobarray[i]->status,jobarray[i]->printersset,jobarray[i]->filename);
        }
        sf_cmd_ok();//PRINTED ALL JOBS

	}

    else if(!strcmp(array[0],"print")){

        if(array[1] == 0){
            printf("FILE NAME REQUIRED!\n");
            sf_cmd_error("FILE NAME REQUIRED");
        }
        else{
        JOB* new;
        CONVERSION** a;
        char period = '.';
        char *periodlocation = strchr(array[1],period);
        char *file = periodlocation +1;
        int printersetint = 0;
        if(periodlocation == NULL){
        printf("PLEASE ENTER A VALID FILE.\n");
        sf_cmd_error("FILE NOT VALID");
        break;
        }
        if (find_type(file) == NULL){
        printf("PLEASE ENTER A VALID FILE.\n");
        sf_cmd_error("FILE NOT VALID");
        break;
        }
        int inputcounter=2;
        while(array[inputcounter]!= NULL){
            inputcounter++;
        }
        inputcounter = inputcounter -2;//TOTAL
         // NUMBER OF ARGUMENTS TO CHECK
        if(inputcounter != 0){
        for (int j=0;j<inputcounter;j++){
            for(int i =0; i<MAX_PRINTERS;i++){
                if(printers[i] == 0 || printers[i] == NULL){
                    printf("PLEASE ENTER VALID PRINTER\n");
                    break;
                }
                if((printers[i] != NULL ||printers[i] != 0) &&(!strcmp(printers[i]->name,array[j+2])))
                    break;
            }
        }

         for(int j =0;j<inputcounter;j++){
            for (int i =0;i<MAX_PRINTERS;i++){
                if(printers[i] == 0 || printers[i] == NULL){ //doesnt find
                    break;
                }
                if(!strcmp(printers[i]->name,array[j+2])){
                    printersetint = printersetint | (1 << i);
                    break;}

            }
        }
        new = create_job(array[1],file,printersetint);
        }
        else{
            for(int i =0;i<MAX_PRINTERS;i++){
               if(printers[i] != NULL || printers[i] != 0)
                printersetint = printersetint | (1 << i);
            }
            new = create_job(array[1],file,printersetint); // ALL ELIGIBLE

        }
        //PRINTER *second;

        int sizeofsmallestarray = -1;
        int counter = 0;
        int z=0;
        PRINTER* printertoreturn;
        //PRINTER *nameofvalidprinter[MAX_PRINTERS];
        for(int i =0;i<MAX_PRINTERS;i++){
            if(printers[i] == 0){
                break; //NO MORE PRINTERS LEFT
            }
            if((((printersetint & ( 1 << i )) >> i) == 1) && printers[i]->status == PRINTER_IDLE){ // THE PRINTER MUST BE ENABLED FIRST
             a = find_conversion_path(file,printers[i]->type);//find conversion path of all eligible printers
            while(a[z] != NULL){ //while there is a legit conversion path,
                counter++;
                z++;}

            if(sizeofsmallestarray == -1){
                sizeofsmallestarray = counter;
                 printertoreturn= printers[i];
            }
            else{
                if(counter < sizeofsmallestarray){
                    sizeofsmallestarray = counter; // IF THE ARRAY OF CONVERSION IS SMALLER THAN LAST THEN ITS PATH IS  SHORTER!
                    printertoreturn = printers[i]; //UPDATE THE FIRST ELEMENT OF THE ARRAY WITH THE VALID PRINTER
                }


            }


            fork_print(new,printertoreturn,a[0]->cmd_and_args);

        }

        }


        sf_cmd_ok();//forked process
    }
    }



    else if(!strcmp(array[0],"cancel")){
        if(array[1] == 0){
            printf("NOT ENOUGH ARGUMENTS");
            sf_cmd_error("NOT ENOUGH ARGUMENTS");
        }
        else if(array[2] != 0 ){
            printf("TOO MANY ARGUMENTS");
            sf_cmd_error("TOO MANY ARGUMENTS");
        }
        else{
            int convertedint = atoi(array[1]);
            for (int i =0;i<MAX_JOBS;i++){
                if(jobarray[i] == 0){
                    printf("PRINTER NOT FOUND\n");
                    sf_cmd_error("PRINTER NOT FOUND");
                    break;
                }
                if(jobarray[i]->id == convertedint ){
                    jobarray[i]->status = JOB_ABORTED;
                    sf_job_status(jobarray[i]->id,JOB_ABORTED);
                    sf_cmd_ok();
                } //if job id is the converted  int


            }

        }

    }

    else if(!strcmp(array[0],"pause")){
        if(array[1] == 0){
            printf("NOT ENOUGH ARGUMENTS");
            sf_cmd_error("NOT ENOUGH ARGUMENTS");
        }
        else if(array[2] != 0 ){
            printf("TOO MANY ARGUMENTS");
            sf_cmd_error("TOO MANY ARGUMENTS");
        }
        else{
            int convertedint = atoi(array[1]);
            for (int i =0;i<MAX_JOBS;i++){
                if(jobarray[i] == 0){
                    printf("PRINTER NOT FOUND\n");
                    sf_cmd_error("PRINTER NOT FOUND");
                    break;
                }
                if(jobarray[i]->id == convertedint ){
                    jobarray[i]->status = JOB_PAUSED;
                    sf_job_status(jobarray[i]->id,JOB_PAUSED);
                    sf_cmd_ok();
                } //if job id is the converted  int


            }

        }
    }

    else if(!strcmp(array[0],"resume")){
        if(array[1] == 0){
            printf("NOT ENOUGH ARGUMENTS");
            sf_cmd_error("NOT ENOUGH ARGUMENTS");
        }
        else if(array[2] != 0 ){
            printf("TOO MANY ARGUMENTS");
            sf_cmd_error("TOO MANY ARGUMENTS");
        }

        else{
            int convertedint = atoi(array[1]);
            for (int i =0;i<MAX_JOBS;i++){
                if(jobarray[i] == 0){
                    printf("PRINTER NOT FOUND\n");
                    sf_cmd_error("PRINTER NOT FOUND");
                    break;
                }
                if(jobarray[i]->id == convertedint ){
                    jobarray[i]->status = JOB_RUNNING;
                    sf_job_status(jobarray[i]->id,JOB_RUNNING);
                    sf_cmd_ok();
                } //if job id is the converted  int


            }

        }
    }

    else if(!strcmp(array[0],"disable")){
        if(array[1] == 0){
            printf("PRINTER NAME REQUIRED\n");
            sf_cmd_error("PRINTER NAME REQUIRED");
        }
        else if(array[2] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");
        }
        else{

            for(int i =0 ; i<MAX_PRINTERS;i++){
                if(printers[i] == NULL || printers[i] == 0){
                    printf("PRINTER NOT FOUND\n");
                    sf_cmd_error("PRINTER NOT FOUND");
                    break;
                }
                if(!strcmp(printers[i]->name,array[1])){
                    printers[i]->status = PRINTER_DISABLED;
                    sf_printer_status(printers[i]->name,PRINTER_DISABLED);
                    sf_cmd_ok();
                    break;
                }

            }
        }
        }

        else if(!strcmp(array[0],"enable")){
            if(array[1] == 0){
            printf("PRINTER NAME REQUIRED\n");
            sf_cmd_error("PRINTER NAME REQUIRED");
        }
        else if(array[2] != 0){
            printf("TOO MANY ARGS\n");
            sf_cmd_error("TOO MANY ARGS");
        }
        else{
            for(int i =0 ; i<MAX_PRINTERS;i++){
                if(printers[i] == NULL || printers[i] == 0){
                    printf("PRINTER NOT FOUND\n");
                    sf_cmd_error("PRINTER NOT FOUND");
                    break;
                }
                if(!strcmp(printers[i]->name,array[1])){
                    printers[i]->status = PRINTER_IDLE;
                    sf_printer_status(printers[i]->name,PRINTER_IDLE);
                    sf_cmd_ok();
                    break;
                }
            }
            int  printerss;
            PRINTER *eligible;
            CONVERSION** a;


            for(int i =0;i<MAX_JOBS;i++){ //find any jobs that are queued
                if(jobarray[i] != 0){
                    printerss = jobarray[i]->printersset; // SAVE THE ELIGIBLE PRINTERS
                     for(int j =0;i<MAX_PRINTERS;j++){ // go through printers and if the current printer status is enabled  and its an eligible printer for current job find conversion path
                         if(printers[j] == 0){
                            break; //NO MORE PRINTERS LEFT
                             }
            if((((printerss & ( 1 << j )) >> j) == 1) && printers[j]->status == PRINTER_IDLE){ // THE PRINTER MUST BE ENABLED FIRST
             a = find_conversion_path(jobarray[i]->filetype,printers[j]->type);
             eligible = printers[j];
                }
            }

            fork_print(jobarray[i],eligible,a[0]->cmd_and_args);
        }
        else{
            break; // PRINTER NOT FOUND
        }
        }}}

        else{
            printf("PLEASE ENTER CORRECT ARGS,refer to HELP for more information\n");

        }



















    }




    fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    abort();
}
