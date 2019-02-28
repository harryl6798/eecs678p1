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
bool is_job_run = false;

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
  env_var = getenv(env_var);
  return env_var;
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.

  //Harry - We'll have to set up the deque in deque.h before we're able to have
  //        jobs running in the background.
  IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
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
  int status = 0;
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

  // TODO: Remove warning silencers
  // (void) env_var; // Silence unused variable warning
  // (void) val;     // Silence unused variable warning

  if(setenv(env_var,val,1) == -1)
  {
    perror("ERROR: Unable to set Environment Variable");
    return;
  }

  // TODO: Implement export.
  // HINT: This should be quite simple.
  //IMPLEMENT_ME();
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

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();
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
  if(!is_empty_job_queue())
  {
    for(int i =0; i< length_job_queue(); i++)
    {
      pid_t temp_front = pop_front_job_queue();
    }
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
void create_process(CommandHolder holder, job_t* job) {
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

  int p[2];
  int fd_in, fd_out;
  if(pipe(p) < 0){
    perror("Could not create pipe");
  }

  if(p_in){

  }
  else{
    close(p[0]);
  }

  if(r_in){
    int flags = O_RDONLY;
    fd_in = open(holder.redirect_in, flags);
    dup2(fd_in,STDIN_FILENO);
  }

  if(p_out){

  }
  else{
    close(p[1]);
  }

  if(r_out && !r_app){ //Overwrite mode for r_out
    int flags = O_WRONLY;
    fd_out = open(holder.redirect_out, flags);
    dup2(fd_out,STDIN_FILENO);
  }
  else{ //Append mode for r_out
    int flags = O_WRONLY | O_APPEND;
    fd_out = open(holder.redirect_out, flags);
    dup2(fd_out,STDIN_FILENO);
  }

  pid_t pid = fork();
  if(pid == 0){ //Child
    child_run_command(holder.cmd);
  }
  else{ //Parent
    // TODO Insert the pid in the process deque for the job here
    parent_run_command(holder.cmd);
  }
}

// Run a list of commands
void run_script(CommandHolder* holders) {

  if(!is_job_run)
  {
    is_job_run = true;
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
  running_job.cmd = get_command_string();
  running_job.pq = new_pid_queue(1);


  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i] , &running_job);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    while(!is_empty_pid_queue(&running_job.pq))
    {
      pid_t front_pid = pop_front_pid_queue(&running_job.pq);
      int status;
      waitpid(front_pid, &status, 0);
    }
    destroy_pid_queue(&running_job.pq);
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    IMPLEMENT_ME();

    // TODO: Once jobs are implemented, uncomment and fill the following line
    // print_job_bg_start(job_id, pid, cmd);
  }
}
