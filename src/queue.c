#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
 
typedef struct position {
    int x;
    int y;
    int type;
} postion;

typedef struct move 
{
  position* actual_position;
  position* next_position;
  move* next;
} move;
 
 
typedef struct queue
{
  struct move* head;
  struct move* tail;
} queue;
 
 
/* Will always return the pointer to queue */
queue* add_element(queue* s, position* actual_position, position* next_position) {
  move* p = (move*) malloc( 1 * sizeof(*p) );
 
  if( NULL == p ) {
        fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
        return s; 
    }

  p->actual_position = actual_position;
  p->next_position = next_position;
  p->next = NULL;

  if( NULL == s ) {
      printf("Queue not initialized\n");
      free(p);
      return s;
  }
  else if( NULL == s->head && NULL == s->tail ) {
      /* printf("Empty list, adding p->num: %d\n\n", p->num);  */
      s->head = s->tail = p;
      return s;
  }
  else if( NULL == s->head || NULL == s->tail ) {
      fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
      free(p);
      return NULL;
  }
  else {
      /* printf("List not empty, adding element to tail\n"); */
      s->tail->next = p;
      s->tail = p;
  }

  return s;
}


/* This is a queue and it is FIFO, so we will always remove the first element */
queue* remove_element( queue* s )
{
    move* h = NULL;
    move* p = NULL;

    if( NULL == s )
    {
        printf("List is empty\n");
        return s;
    }
    else if( NULL == s->head && NULL == s->tail )
    {
        printf("Well, List is empty\n");
        return s;
    }
    else if( NULL == s->head || NULL == s->tail )
    {
        printf("There is something seriously wrong with your list\n");
        printf("One of the head/tail is empty while other is not \n");
        return s;
    }

    h = s->head;
    p = h->next;
    free(h);
    s->head = p;
    if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */

    return s;
}

queue* clean_queue( queue* s ) {
    while( s->head ) {
        remove_element(s);
    }

    return s;
}

queue* create_queue(void) {
    move* p = (move*) malloc( 1 * sizeof(*p));

    if( NULL == p ) {
        fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
    }

    p->head = p->tail = NULL;

    return p;
}
