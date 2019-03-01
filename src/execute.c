/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#define _GNU_SOURCE
#include "execute.h"
#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include "quash.h"
#include "deque.h"
#include <sys/wait.h> //Uses the waitpid() call

#include <limits.h> //For PATH_MAX when allocating a char[]

#include <sys/types.h>  //
#include <fcntl.h>      //    For use with open()
#include <sys/stat.h>   //

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

//Defines the queue for Queue
IMPLEMENT_DEQUE_STRUCT (pid_queue, pid_t);
IMPLEMENT_DEQUE (pid_queue, pid_t);
PROTOTYPE_DEQUE(pid_queue, pid_t);

typedef struct job_t{
  int job_id;
  pid_queue pq;
  char* cmd;


}job_t;

IMPLEMENT_DEQUE_STRUCT(pid_job, job_t);
IMPLEMENT_DEQUE(pid_job, job_t);
PROTOTYPE_DEQUE(pid_job, job_t);

pid_job jobs;
bool job_run = false;
static int id_num = 1;

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  char* test = get_current_dir_name();
  *should_free = true;
  return test;
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
    return getenv(env_var);

}

// Check the status of background jobs
void check_jobs_bg_status() {
  if(!job_run) //Inits the job deque if this is the first run script
  {
    job_run = true;
    jobs = new_pid_job(1);
  }
  if (is_empty_pid_job(&jobs))
  {
    return; //No Jobs are currently happening
  }
  // bool should_delete = false;

  for(int i =0; i < length_pid_job(&jobs); i++)
  {
    //bool should_delete = true;
    //Get First JOBS
    job_t front_value_job = pop_front_pid_job(&jobs);
    pid_t front_temp_queue = peek_front_pid_queue(&front_value_job.pq);
    for(int r =0 ; r< length_pid_queue(&front_value_job.pq);r++)
    {

      pid_t temp_process = pop_front_pid_queue(&front_value_job.pq);
      int status;
        if(waitpid(temp_process, &status, WNOHANG) == 0) //Iterate through all processes
        {
          push_back_pid_queue(&front_value_job.pq, temp_process);
          //should_delete = false; //If any processes are running, don't delete the job
        }
    }

  if(is_empty_pid_queue(&front_value_job.pq))
    {
      print_job_bg_complete(front_value_job.job_id, front_temp_queue, front_value_job.cmd);
      free(front_value_job.cmd);
      destroy_pid_queue(&front_value_job.pq);
      //TODO: We may have to delete the char* cmd of front_value_job for no leak here
    }
    else //If we don't delete, add the job back to the jobs dequeue
    {
      //print_job_bg_complete(front_value_job.job_id, front_temp_queue, front_value_job.cmd);

      push_back_pid_job(&jobs, front_value_job);
    }
  }
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;
  if((execvp  (exec, args)) < 0){
    perror("ERROR: Failed to execute program");
  }
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  int i = 0;
  while(str[i] != NULL){
    printf("%s ",str[i]);
    i++;
  }
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);

}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;
  if(setenv(env_var,val,1) == -1)
  {
    perror("ERROR: Unable to set Environment Variable");
    return;
  }
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* new_dir = cmd.dir;
  const char* new_var = "PWD";
  const char* old_dir = lookup_env(new_var); //Get the old environment var
  const char* old_var = "OLD_PWD";
  // Check if the directory is valid
  if (new_dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }
  //Set OLDPWD to the previous dir
  if(setenv(old_var,old_dir,1) == -1)
  {
    perror("ERROR: Unable to change environment var $OLD_PWD");
    return;
  }
  //Set PWD to the new dir
  if(setenv(new_var,new_dir,1) == -1)
  {
    perror("ERROR: Unable to change environment var $PWD");
    return;
  }
  chdir(new_dir);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  int currentLength = length_pid_job(&jobs);
  for (int i =0; i< currentLength; i++)
  {
    job_t temp_front = pop_front_pid_job(&jobs);
    if(job_id == temp_front.job_id)
    {

      //Gotta kill all the Processes
      //int num_proccesses = length_pid_queue(&temp_front.pq);
      while(!is_empty_pid_queue(&temp_front.pq))
      {
        pid_t kill_pid = pop_front_pid_queue(&temp_front.pq);

        kill(kill_pid, signal);
        push_back_pid_queue(&temp_front.pq, kill_pid);
      }

    }

      push_back_pid_job(&jobs, temp_front); // pushes it back because we need to kill the job

  }
}


// Prints the current working directory to stdout
void run_pwd() {
  char cwd[PATH_MAX];
   if (getcwd(cwd, PATH_MAX) != NULL) {
       printf("%s\n", cwd);
   } else {
       perror("run_pwd() error");
   }
  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs

    for(int i =0; i< length_pid_job(&jobs); i++)
    {
      job_t temp_front = pop_front_pid_job(&jobs);
      print_job(temp_front.job_id, peek_front_pid_queue(&temp_front.pq), temp_front.cmd);
      push_back_pid_job(&jobs, temp_front );
    }



  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, job_t* job, int* pipes, int fd_in, int fd_out, int count) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  if(r_app && !(r_out)){
    perror("ERROR: r_app = true and r_out = false");
  }
  //fprintf(stdout, "p_in: %d  p_out: %d  r_in: %d  r_out: %d  r_app: %d\n\n",p_in,p_out,r_in,r_out,r_app);
  fflush(stdout);
  pid_t pid = fork();
  if(pid == 0){ //Child - close
    //fprintf(stdout, "Entering child\n");
    for(int i = 0; i < count;i++){
      if(i != fd_in && i != fd_out){ //Close the pipe if it's not used
        //fprintf(stdout, "Closing pipe %d: %d child\n",i,pipes[i]);
        if(close(pipes[i]) < 0){
          perror("Failed to closed pipe in child process");
        }
      }
    }
    fflush(stdout);
    if(p_in){
      //fprintf(stdout, "%d to STDIN, child\n", pipes[fd_in]);
      if(dup2(pipes[fd_in],STDIN_FILENO) < 0){
        perror("Could not duplicate pipe");
      }
    }
    if(p_out){
      //fprintf(stdout, "%d to STDOUT, child\n", pipes[fd_out]);
      if(dup2(pipes[fd_out],STDOUT_FILENO) < 0){
        perror("Couldn't create duplicate pipe");
      }
    }
    if(r_in){
      int flags = O_RDONLY;
      fd_in = open(holder.redirect_in, flags);
      dup2(fd_in,STDIN_FILENO);
    }

    if(r_out && !r_app){ //Overwrite mode for r_out
      int flags = O_RDWR | O_CREAT;
      int file_out = open(holder.redirect_out, flags, 0666);
      if(dup2( file_out ,STDOUT_FILENO) < 0){
        perror("Failed to open file in append mode");
      }
    }
    else if(r_out && r_app) { //Append mode for r_out
      int flags = O_RDWR | O_APPEND | O_CREAT;
      int file_out = open(holder.redirect_out, flags, 0666);
      if(dup2( file_out, STDOUT_FILENO) < 0){
        perror("Failed to open file in append mode");
      }
    }
    child_run_command(holder.cmd);  //Pipes would be closed here after the execv
    exit(0); 
  }
  else{ //Parent
    push_front_pid_queue(&job->pq,pid); //TODO: should this be push_back?
    //fprintf(stdout, "Entering parent\n");
    fflush(stdout);
    parent_run_command(holder.cmd);
  }
}

// Run a list of commands
void run_script(CommandHolder* holders) {

  if(!job_run) //Inits the job deque if this is the first run script
  {
    job_run = true;
    jobs = new_pid_job(1);
  }

  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;
  job_t running_job;
  running_job.job_id = id_num;
  running_job.cmd = get_command_string();
  running_job.pq = new_pid_queue(1);

  int processes = 0;//Count the number of processes
  for (;(type = get_command_holder_type(holders[processes])) != EOC; ++processes){

  }

  int pipes[(processes-1) * 2];
  for(int i = 0; i < processes-1; i++){//init pipes
    pipe(pipes + (i*2) );
  }
  for(int i = 0; i < (processes-1) * 2; i++){
    //fprintf(stdout, "Pipe %d: %d\n",i, pipes[i]);
  }

  int fd_in, fd_out; // -1 for stdin or stdout
  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; i++){
    if(i == 0){ //first process
      fd_in = -1;
    }
    else{
      fd_in = (i-1)*2;
    }
    if(i == processes - 1){ // last process
      fd_out = -1;
    }
    else{
      fd_out = i*2 + 1;
    }
    /*
    * Process:  0 1 2 3 ...  n
    * fd_in:   -1 0 2 4 ...  2(n-1)
    * fd_out:   1 3 5 7 ... -1
    */
    //fprintf(stdout, "\nCreate process %d with fd_in:%d \t fd_out:%d \n", i, fd_in, fd_out);
    fflush(stdout);
    create_process(holders[i] , &running_job, pipes, fd_in, fd_out, (processes-1)*2);
  }
  // 
  // Close all the pipes in the parent process
  //   
  for(int i = 0; i < (processes-1) * 2; i++){
    //fprintf(stdout, "Closing pipe: %d parent\n",pipes[i]);
    if(close(pipes[i]) < 0){
      perror("Failed to clse pipe in parent process");
    }
  }
  fflush(stdout);

  if (!(holders[0].flags & BACKGROUND)) { // Not a background Job
    while(!is_empty_pid_queue(&running_job.pq)){
      pid_t front_pid = pop_front_pid_queue(&running_job.pq);
      int status;
      waitpid(front_pid, &status, 0);
      //fprintf(stdout, "PROCESSES FINISHED");
    }
      //Have to use this to delete usage of get_command_string
    destroy_pid_queue(&running_job.pq);
    free(running_job.cmd);
  }

  else {
    // A background job.

      id_num++;
    //test

    push_back_pid_job(&jobs, running_job);
    // TODO: Once jobs are implemented, uncomment and fill the following line
    print_job_bg_start(running_job.job_id, peek_back_pid_queue(&running_job.pq), running_job.cmd);
  }
}
