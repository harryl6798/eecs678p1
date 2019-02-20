/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "command.h"
#include "execute.h"
#include "parsing_interface.h"
#include "memory_pool.h"

/**************************************************************************
 * Private Variables
 **************************************************************************/
static QuashState state;

/**************************************************************************
 * Private Functions
 **************************************************************************/
static QuashState initial_state() {
  return (QuashState) {
    true,
    isatty(STDIN_FILENO),
    NULL
  };
}

// Print a prompt for a command
static void print_prompt() {
  bool should_free = true;
  char* cwd = get_current_directory(&should_free);

  assert(cwd != NULL);

  char hostname[HOST_NAME_MAX];

  // Get the hostname
  gethostname(hostname, HOST_NAME_MAX);

  // Remove first period and everything afterwards
  for (int i = 0; hostname[i] != '\0'; ++i) {
    if (hostname[i] == '.') {
      hostname[i] = '\0';
      break;
    }
  }

  char* last_dir = cwd;
  // Show only last directory
  for (int i = 0; cwd[i] != '\0'; ++i) {
    if (cwd[i] == '/' && cwd[i + 1] != '\0') {
      last_dir = cwd + i + 1;
    }
  }

  char* username = getlogin();

  // print the prompt
  printf("[QUASH - %s@%s %s]$ ", username, hostname, last_dir);

  fflush(stdout);

  if (should_free)
    free(cwd);
}

/**************************************************************************
 * Public Functions
 **************************************************************************/
// Check if loop is running
bool is_running() {
  return state.running;
}

// Get a copy of the string
char* get_command_string() {
  return strdup(state.parsed_str);
}

// Check if Quash is receiving input from the command line or not
bool is_tty() {
  return state.is_a_tty;
}

// Stop Quash from requesting more input
void end_main_loop() {
  state.running = false;
}

/**
 * @brief Quash entry point
 *
 * @param argc argument count from the command line
 *
 * @param argv argument vector from the command line
 *
 * @return program exit status
 */
int main(int argc, char** argv) {
  state = initial_state();

  if (is_tty()) {
    puts("Welcome to Quash!");
    puts("Type \"exit\" or \"quit\" to quit");
    puts("---------------------------------");
    fflush(stdout);
  }

  atexit(destroy_parser);
  atexit(destroy_memory_pool);

  // Main execution loop
  while (is_running()) {
    if (is_tty())
      print_prompt();

    initialize_memory_pool(1024);
    CommandHolder* script = parse(&state);

    if (script != NULL)
      run_script(script);

    destroy_memory_pool();
  }

  return EXIT_SUCCESS;
}
