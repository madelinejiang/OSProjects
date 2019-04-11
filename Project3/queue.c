#include <stdio.h>
#include <stdlib.h>
#include "simos.h"
#include <unistd.h>

node_t *head = NULL;

void enqueue(request_t req)
{ node_t *new_node = (node_t*)malloc(sizeof(node_t));
  if (!new_node) return;

  new_node->request = req;
  new_node->next = head;
  head = new_node;
}


request_t* dequeue()//takes the tail which is the first request
{ node_t *current, *prev = NULL;
  request_t *retreq = NULL;

  if (head == NULL) return NULL;

  current = head;
  while (current->next != NULL)
  { prev = current; current = current->next; }
  retreq = &(current->request);

  if (prev) prev->next = NULL;
  else head = NULL;

  return retreq;
}

void dump_enqueue()
{ node_t *current = NULL;

  printf("Dump Request EnQueue:\n");
  current = head;
  while (current != NULL)
  { printf("%s, %d\n",
           current->request.filename,
            current->request.sockfd );
    current = current->next;
  }
}

