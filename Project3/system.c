#include <stdio.h>
#include "simos.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void initialize_system ()
{ FILE *fconfig;
  char str[60];

  fconfig = fopen ("config.sys", "r");
  fscanf (fconfig, "%d %d %d %s\n",
          &maxProcess, &cpuQuantum, &idleQuantum, str);
  fscanf (fconfig, "%d %d %s\n", &pageSize, &memPages, str);
  fscanf (fconfig, "%d %d %d %s\n", &loadPpages, &maxPpages, &OSpages, str);
  fscanf (fconfig, "%d %d %d %s\n",
          &periodAgeScan, &termPrintTime, &diskRWtime, str);
  fscanf (fconfig, "%d %s\n", &Debug, str);
  fclose (fconfig);

  // all processing has a while loop on systemActive
  // admin with T command can stop the system
  systemActive = 1;

  initialize_timer ();
  initialize_cpu ();
  //initialize_memory_manager ();
  initialize_process ();
}

// initialize system mainly intialize data structures of each component.
// start_terminal, process_client_submission, process_admin_command are
// system operations and are started in main.
void main (int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided. Usage: .exe portno\n");
		exit(1);
	}
  initialize_system ();

  //printf("Admin-Computer, %d\n", getpid());
  start_terminal ();   // term.c
  start_swap_manager ();
  //create submission thread manager
  start_client_reqhandler(argv[1]);
  process_admin_command ();   // admin.c


  // admin terminated the system, wait for other components to terminate
 
  end_terminal();   // term.c
  end_client_submission ();   // submit.c
  end_swap_manager ();
}
