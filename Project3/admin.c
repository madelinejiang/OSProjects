#include <stdio.h>
#include "simos.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h>



void process_admin_command ()
{ char action[10];
  char fname[100];

  while (systemActive)
  { printf ("command> ");
    scanf ("%s", &action);
    if (Debug) printf ("Command is %c\n", action[0]);
    // only first action character counts, discard remainder
    switch (action[0])
    { case 's':  // take a request from enqueue and submit 
		{
			request_t *req=NULL;
		if ((req = dequeue()) != NULL)
			{
			//transfer information into PCB
			submit_process(req->filename, req->sockfd);

			}
		else printf("no submissions from clients\n");
		}
         break;
      case 'x':  // execute
        execute_process ();
        break;
      case 'r':  // dump register
        dump_registers ();
        break;
      case 'q':  // dump ready queue and list of processes completed IO
        dump_ready_queue ();
        dump_endWait_list ();
        break;
      case 'p':   // dump PCB
        dump_PCB_list ();
        break;
      case 'm':   // dump Memory
        dump_PCB_memory ();
        break;
      case 'e':   // dump events in timer
        dump_events ();
        break;
      case 'd':   // dump terminal IO queue
        dump_termio_queue ();
        dump_swapQ ();
        break;
      case 'T':  // Terminate, do nothing, terminate in while loop
        systemActive = 0;
        break;
      default:   // can be used to yield to client submission input
        printf ("Incorrect command!!!\n");
		break;
    }
  }
  printf ("Admin command processing loop ended!\n");
}


