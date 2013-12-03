#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define RED_GEN 0
#define BLK_GEN 1

#define EMPTY 0
#define SQUIRREL 1
#define WOLF 2
#define TREE 3
#define ICE 4
#define SQUIRREL_TREE 5

#define CHUNK_SIZE 0
#define CHUNK_MSG 1
#define END_SIZE 2
#define END_MSG 3
#define CONFLICT_SIZE 4
#define CONFLICT_MSG 5

#define BUFFER 1000

typedef struct world_cell {
  int type;
  int breeding;
  int starvation;
  int is_breeding;
} cell;

typedef struct position {
  int x;
  int y;
  cell* cell;
} position;

cell* world_array;
cell** world_indexer;

cell* world_array_read;
cell** world_indexer_read;

char* top_ghost_line;
char* bottom_ghost_line;

int world_size;
int chunk_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;
int num_processes;

void clear_ghost_line() {
  free(top_ghost_line);
  free(bottom_ghost_line);

  bottom_ghost_line = (char*) malloc(1); *bottom_ghost_line = '\0';
  top_ghost_line = (char*) malloc(1); *top_ghost_line = '\0';
}

void print_world(int pid, int gen) { 
  int i, j, size;

  if(pid == 0 || pid == 1 ) 
    size = chunk_size + 1;
  else 
    size = chunk_size + 2;


  printf("WORLD %d, size %d: PID %d at generatio %d\n", world_size, size, pid, gen);
  for(i = -1; i < size ; i++) {
    if(i != -1) printf("|%d", i);
    for(j = 0; j < world_size ; j++) {
      if( i == -1) {
        if(j == 0)
          printf("  ");
        printf("|%d", j % 10);
      } else {
        printf("%c", '|');
        switch(world_indexer[i][j].type) {
          case SQUIRREL:
            printf("%c", 's');
            break;
          case WOLF:
            printf("%c", 'w');
            break;
          case TREE:
            printf("%c", 't');
            break;
          case ICE:
            printf("%c", 'i');
            break;
          case SQUIRREL_TREE:
            printf("%c", '$');
            break;
          default:
            printf("%c", ' ');
            break;
        }
      }
    }
    printf("|\n");
  }
}

/** Allocates space for the world. */
void initialization(int world_size, int chunk_size) {
  int i;

  world_array = (cell*) calloc( chunk_size *world_size , sizeof(cell));   
  world_indexer = (cell**) calloc( chunk_size, sizeof(cell*));
  world_array_read = (cell*) calloc( chunk_size*world_size , sizeof(cell));   
  world_indexer_read = (cell**) calloc( chunk_size, sizeof(cell*));

  for (i = 0; i < chunk_size; ++i) {
    world_indexer[i] = &world_array[i*world_size];
    world_indexer_read[i] = &world_array_read[i*world_size];
  }

  top_ghost_line = (char*) malloc(1); *top_ghost_line = '\0';
  bottom_ghost_line = (char*) malloc(1); *bottom_ghost_line = '\0';
}

/** Read's file and populates the world */
void genesis(char *input, int pid, int num_processes) { 
  char* save;
  char* token = strtok_r(input, "\n", &save);
  int chunk;

  world_size = atoi(token);
  chunk_size = world_size / num_processes;
  int end_size; 

  if( pid == 0 )  {
    chunk = num_processes;
    end_size = world_size - (chunk - 1) * chunk_size;
    end_size++;
    initialization(world_size, end_size);
  }
  else {
    chunk = pid;
    ( pid == 1 ) ? initialization(world_size, chunk_size + 1) : initialization(world_size, chunk_size + 2);
  }


  while ( (token = strtok_r(NULL, "\n", &save)) != NULL ) {
    int x, y, type_num;
    char type;

    sscanf( token, "%d %d %c", &x, &y, &type);
    if(chunk != 1) {
      x =  x - ((chunk - 1) * chunk_size - 1);
    }
    switch(type) {
      case 's':
        type_num = SQUIRREL;
        world_indexer[x][y].breeding = squirrel_breeding;
        world_indexer[x][y].starvation = 0;
        world_indexer_read[x][y].breeding = squirrel_breeding;
        world_indexer_read[x][y].starvation = 0;
        break;
      case 'w':
        type_num = WOLF;
        world_indexer[x][y].breeding = wolf_breeding;
        world_indexer[x][y].starvation =  wolf_starvation;
        world_indexer_read[x][y].breeding = wolf_breeding;
        world_indexer_read[x][y].starvation =  wolf_starvation;
        break;
      case 't':
        type_num = TREE;
        world_indexer[x][y].breeding = 0;
        world_indexer[x][y].starvation = 0;
        world_indexer_read[x][y].breeding = 0;
        world_indexer_read[x][y].starvation = 0;
        break;
      case 'i':
        type_num = ICE;
        world_indexer[x][y].breeding = 0;
        world_indexer[x][y].starvation = 0;
        world_indexer_read[x][y].breeding = 0;
        world_indexer_read[x][y].starvation = 0;
        break;
    }
    world_indexer[x][y].type = type_num;
    world_indexer_read[x][y].type = type_num;
  }
}

int is_free_position(int x, int y, int type){
  if( type == WOLF) {
    return ( world_indexer_read[x][y].type == EMPTY || world_indexer_read[x][y].type == SQUIRREL) && world_indexer_read[x][y].type != type;
  } else {
    return ( world_indexer_read[x][y].type == EMPTY || world_indexer_read[x][y].type == TREE) && world_indexer_read[x][y].type != type;
  }
}

int* find_free_positions(position* actual) {
  int *array = (int*) calloc(4, sizeof(int));
  int x = actual->x, y = actual->y, type = actual->cell->type;

  array[UP] = x - 1 >= 0 && is_free_position(x-1, y, type);
  array[RIGHT] = y + 1 < world_size && is_free_position(x, y+1, type);
  array[DOWN] = x + 1 < chunk_size && is_free_position(x+1, y, type);
  array[LEFT] = y - 1 >= 0 && is_free_position(x, y-1, type);

  return array;
}

int* find_squirrels(position *actual) {
  int *array = (int*) calloc(4, sizeof(int));
  int x = actual->x, y = actual->y;

  array[UP] = x - 1 >= 0 && world_indexer_read[x-1][y].type == SQUIRREL;
  array[RIGHT] = y + 1 < world_size && world_indexer_read[x][y+1].type == SQUIRREL;
  array[DOWN] = x + 1 < chunk_size && world_indexer_read[x+1][y].type == SQUIRREL;
  array[LEFT] = y - 1 >= 0 && world_indexer_read[x][y-1].type == SQUIRREL;

  return array;
}

int count_free_positions(int* free_positions) {
  int i, counter = 0;

  for ( i = 0; i < 4 ; i++){
    counter += free_positions[i] ? 1 : 0; 
  }

  return counter;
}

int calculate_direction(int *possible_positions, int next){
  int direction;

  for ( direction = 0; direction < 4 ; direction++){
    if( possible_positions[direction] ) { 
      next--;
    }
    if ( next == -1 ) {
      return direction;
    }
  }

  return -1;
}

int find_next_positon(int pid, position *actual) {
  int* free_positions = find_free_positions(actual);
  int* free_squirrels = find_squirrels(actual);
  int c;
  //int c = actual->x * world_size + actual->y;
  int chunk;

  if( pid == 0 ) {
    chunk = num_processes;
  }
  else {
    chunk = pid;
  }

  if(chunk != 1) {
    c = (actual->x + ((chunk - 1) * chunk_size - 1)) * world_size + actual->y;
  } else {
    c = actual->x * world_size + actual->y;
  }

  int free_counter = count_free_positions(free_positions);
  int squirrel_counter = count_free_positions(free_squirrels);
  int direction = -1;

  if( actual->cell->type == WOLF &&  squirrel_counter > 0) {
    int next = c % squirrel_counter;
    direction = calculate_direction(free_squirrels, next);
  } else if ( free_counter > 0 ) {
    int next = c % free_counter;
    direction = calculate_direction(free_positions, next);
  }

  free(free_positions); 
  free(free_squirrels);

  return direction;
}

void wolf(position* actual, position* next) {
  int x = next->x, y = next->y;

  int type = world_indexer[x][y].type;
  int actual_breeding = world_indexer[actual->x][actual->y].breeding;
  int actual_starvation = world_indexer[actual->x][actual->y].starvation;

  switch(type) {
    case WOLF:
      if( world_indexer[x][y].starvation < actual_starvation ) {
        world_indexer[x][y].starvation = actual_starvation;
        world_indexer[x][y].breeding = actual_breeding;
      }
      else if( world_indexer[x][y].starvation == actual_starvation) {
        if( world_indexer[x][y].breeding < actual_breeding) {
          world_indexer[x][y].breeding = actual_breeding;
        }
      }
      break;
    case EMPTY:
      world_indexer[x][y].starvation = actual_starvation;
      world_indexer[x][y].breeding = actual_breeding;
      break;
    case SQUIRREL:
      world_indexer[x][y].starvation = wolf_starvation;
      world_indexer[x][y].breeding = actual_breeding;
      break;
  }

  world_indexer[x][y].type = WOLF;
}

void squirrel(position* actual, position* next) {
  int x = next->x, y = next->y;
  int type = world_indexer[x][y].type;
  int actual_breeding = world_indexer[actual->x][actual->y].breeding;

  switch(type) {
    case WOLF:
      world_indexer[x][y].starvation = wolf_starvation;
      break;
    case EMPTY:
      world_indexer[x][y].type = SQUIRREL;
      world_indexer[x][y].starvation = 0;
      world_indexer[x][y].breeding = actual_breeding;
      break;
    case TREE:
      world_indexer[x][y].type = SQUIRREL_TREE;
      world_indexer[x][y].starvation = 0;
      world_indexer[x][y].breeding = actual_breeding;
      break;
    case SQUIRREL_TREE:
      world_indexer[x][y].type = SQUIRREL_TREE;
      world_indexer[x][y].starvation = 0;
      if(  world_indexer[x][y].breeding <= actual_breeding ) {
        world_indexer[x][y].breeding = actual_breeding;
      } 
      break;
    case SQUIRREL:
      world_indexer[x][y].starvation = 0;
      if(  world_indexer[x][y].breeding <= actual_breeding ) {
        world_indexer[x][y].breeding = actual_breeding;
      } 
      break;
  }
}

void normal_move(position* actual, position* next) {
  int actual_type = actual->cell->type;

  switch(actual_type) {
    case WOLF:
      wolf(actual, next);
      break;
    case SQUIRREL:
      squirrel(actual, next);
      break;
    case SQUIRREL_TREE:
      squirrel(actual, next);
      break;
  }
}

void die(position* actual) {
  int x = actual->x, y = actual->y;

  world_indexer[x][y].type = EMPTY;
  world_indexer[x][y].starvation = 0;
  world_indexer[x][y].breeding = 0;
  world_indexer[x][y].is_breeding = 0;
}

void breed(position *actual) {
  int x = actual->x, y = actual->y;
  if( actual->cell->is_breeding ) {
    if( actual->cell->type == WOLF ) {
      world_indexer[x][y].starvation = wolf_starvation;
      world_indexer[x][y].breeding = wolf_breeding;
    } else {
      world_indexer[x][y].breeding = squirrel_breeding;
    }
  }
}

void breed_move(position* actual, position* next) {
  breed(actual);
  normal_move(actual, next);
}

void clear(position* actual) {
  int x = actual->x, y = actual->y, type = actual->cell->type;

  world_indexer[x][y].type = ( type == SQUIRREL_TREE ) ? TREE : EMPTY;
  world_indexer[x][y].starvation = 0;
  world_indexer[x][y].breeding = 0;
  world_indexer[x][y].is_breeding = 0;
}

void move_element(position* actual, position* next) {
  if( actual->cell->is_breeding ) {
    breed_move(actual, next);
  } else {
    normal_move(actual, next);
    clear(actual);
  }
}

void go_up(position *actual) {
  position* next = (position*) calloc( 1, sizeof(position) );

  next->x = actual->x - 1;
  next->y = actual->y;
  next->cell = &world_indexer_read[next->x][next->y];

  move_element( actual, next);
  free(next);
}

void go_right(position *actual) {
  position* next = (position*) calloc( 1, sizeof(position) );

  next->x = actual->x;
  next->y = actual->y + 1;
  next->cell = &world_indexer_read[next->x][next->y];

  move_element( actual, next);
  free(next);
}

void go_down(position *actual) {
  position* next = (position*) calloc( 1, sizeof(position) );

  next->x = actual->x + 1;
  next->y = actual->y;
  next->cell = &world_indexer_read[next->x][next->y];

  move_element( actual, next);
  free(next);
}

void go_left(position *actual) {
  position* next = (position*) calloc( 1, sizeof(position) );

  next->x = actual->x;
  next->y = actual->y - 1;
  next->cell = &world_indexer_read[next->x][next->y];

  move_element( actual, next);
  free(next);
}

void add_tmp_line(int pid, position *position, int direction){
  char buffer[BUFFER];
  int x = position->x;
  int y = position->y;
  int type = position->cell->type;
  int breed = position->cell->breeding;
  int starvation = position->cell->starvation;
  int is_breeding = position->cell->is_breeding;

  x = ( UP  == direction) ? x - 1 : x + 1;

  if( pid == 1 && x == chunk_size ) {
    // add to tmp ghost bottom
    sprintf(buffer, "%d %d %d %d %d %d\n", x, y, type, breed, starvation, is_breeding);
    char *tmp = malloc( strlen(buffer) + strlen(bottom_ghost_line) + 1);

    strcpy(tmp, bottom_ghost_line);
    strcat(tmp, buffer);

    char *aux = bottom_ghost_line;
    bottom_ghost_line = tmp;
    free(aux);

  } else if ( pid == 0 && x == 0 ) {
    // add to tmp ghost top
    sprintf(buffer, "%d %d %d %d %d %d\n", x, y, type, breed, starvation, is_breeding);
    char *tmp = malloc( strlen(buffer) + strlen(top_ghost_line) + 1);

    strcpy(tmp, top_ghost_line);
    strcat(tmp, buffer);

    char *aux = top_ghost_line;
    top_ghost_line = tmp;
    free(aux);
  } else if ( pid != 0 && pid != 1 && x == 0 ) {
    // add to top ghost of 2 ghost chunk
    sprintf(buffer, "%d %d %d %d %d %d\n", x, y, type, breed, starvation, is_breeding);
    char *tmp = malloc( strlen(buffer) + strlen(top_ghost_line) + 1);

    strcpy(tmp, top_ghost_line);
    strcat(tmp, buffer);

    char *aux = top_ghost_line;
    top_ghost_line = tmp;
    free(aux);
  } else if ( pid != 0 && pid != 1 && x == chunk_size ) {
    // add to bottom ghost of 2
    sprintf(buffer, "%d %d %d %d %d %d\n", x, y, type, breed, starvation, is_breeding);
    char *tmp = malloc( strlen(buffer) + strlen(bottom_ghost_line) + 1);

    strcpy(tmp, bottom_ghost_line);
    strcat(tmp, buffer);

    char *aux = bottom_ghost_line;
    bottom_ghost_line = tmp;
    free(aux);
  }

}

void go(int pid, position *actual) {
  int next = find_next_positon(pid, actual);

  switch(next) {
    case UP:
      add_tmp_line(pid, actual, UP);
      go_up( actual );
      break;
    case RIGHT:
      go_right( actual );
      break;
    case DOWN:
      add_tmp_line(pid, actual, DOWN);
      go_down( actual );
      break;
    case LEFT:
      go_left( actual );
      break;
    default:
      // can be breeding in the location without moving
      breed( actual );
      break;
  }
}

void exodus(int pid, int x, int y) {
  position* actual = (position*) calloc( 1, sizeof(position) );
  actual->x = x;
  actual->y = y;
  actual->cell = &world_indexer_read[x][y];

  int type = actual->cell->type;

  if( SQUIRREL == type || SQUIRREL_TREE == type || WOLF == type ) {
    if( actual->cell->starvation == 0 && WOLF == type) {
      die(actual);
      return;
    }

    if( actual->cell->breeding == 0 ) {
      actual->cell->is_breeding = 1;
    }

    go(pid, actual);
  }

  free(actual);
}

void sub_generation(int is_black_gen, int pid){
  int i, j, size;

  // Don't process ghost lines
  if(pid == 0) {
    i = 1;
    // final chunk size of pid 0 + 1 4 ghost
    size =  world_size - (num_processes- 1) * chunk_size + 1;
  }
  else if( pid == 1 ) {
    i = 0;
    size = chunk_size + 1;
  } else {
    i = 1;
    size = chunk_size + 2;
  }

  for(; i < size; i++) {
    int k;

    if( is_black_gen ) {
      k = ( i % 2 == 0) ? 1 : 0;
    } else {
      k = ( i % 2 == 0) ? 0 : 1;
    }

    for( j = k ; j < world_size; j = j + 2) {
      exodus(pid, i, j);
    }
  }
}

void update_generation() {
  int i, j;

  for( i = 0 ; i < chunk_size ; i++ ) {
    for( j = 0 ; j < world_size ; j++ ) {
      int type = world_indexer[i][j].type;

      if ( type == WOLF ) {
        if( world_indexer[i][j].starvation > 0 )  world_indexer[i][j].starvation--;
        if( world_indexer[i][j].breeding > 0 )  world_indexer[i][j].breeding--;
      } else if( type == SQUIRREL || type == SQUIRREL_TREE ) {
        if( world_indexer[i][j].breeding > 0 )  world_indexer[i][j].breeding--;
      }
    }
  }
  memcpy(world_array_read, world_array, chunk_size*world_size*sizeof(cell));
  for (i = 0; i < chunk_size ; ++i) {
    world_indexer_read[i] = &world_array_read[i*world_size];
  }
}

void duplicate() {
  memcpy(world_array_read, world_array, chunk_size*world_size*sizeof(cell));
  int i;

  for (i = 0; i < chunk_size; ++i) {
    world_indexer_read[i] = &world_array_read[i*world_size];
  }
}


/* Sends from process 0 to all other nodes the input */
char* send_input(FILE* fp, int num_processes) {
  char buffer[BUFFER];
  char *sendBuffer = (char*) malloc(strlen(buffer)+1);
  char *world_size_c = (char*) malloc(strlen(buffer)+1);

  // read world size
  fgets(buffer, BUFFER, fp);
  memcpy(sendBuffer, buffer, strlen(buffer)+1);
  memcpy(world_size_c, buffer, strlen(buffer)+1);

  // calculate chunk size
  world_size = atoi(buffer);
  chunk_size = world_size / num_processes;
  int offset = chunk_size, pid;

  char* tmp_line_after = (char*) malloc(1);
  char* tmp_line_before = (char*) malloc(1);
  *tmp_line_after = '\0';
  *tmp_line_before = '\0';

  for( pid = 1 ; pid < num_processes + 1; pid++){
    while( fgets(buffer, BUFFER, fp) != NULL) {
      char *save;
      char *token = strtok_r(buffer, " ", &save);
      int x = atoi(token);

      if( x <= offset ) {
        char *result = (char*) malloc( strlen(sendBuffer) + strlen(token) + strlen(save) + 2);

        strcpy(result, sendBuffer);
        strcat(result, token);
        strcat(result, " ");
        strcat(result, save);

        if( x == offset ) {
          char *new_tmp_line_after = (char*) malloc( strlen(tmp_line_after) + strlen(token) + strlen(save) + 2 );

          strcpy(new_tmp_line_after, tmp_line_after);
          strcat(new_tmp_line_after, token);
          strcat(new_tmp_line_after, " ");
          strcat(new_tmp_line_after, save);

          char* tmp = tmp_line_after;
          tmp_line_after = new_tmp_line_after;
          free(tmp);
        }
        if( x == offset - 1 ) {
          char *new_tmp_line_before = (char*) malloc( strlen(tmp_line_before) + strlen(token) + strlen(save) + 2 );
          strcpy(new_tmp_line_before, tmp_line_before);
          strcat(new_tmp_line_before, token);
          strcat(new_tmp_line_before, " ");
          strcat(new_tmp_line_before, save);

          char* tmp = tmp_line_before;
          tmp_line_before = new_tmp_line_before;
          free(tmp);
        }

        char* tmp = sendBuffer;
        sendBuffer = result;
        free(tmp);
      } 
      else {
        if(pid != num_processes) {
          int length = strlen(sendBuffer) + 1;
          MPI_Send(&length, 1, MPI_INT, pid, CHUNK_SIZE, MPI_COMM_WORLD);
          MPI_Send(sendBuffer, length, MPI_CHAR, pid, CHUNK_MSG, MPI_COMM_WORLD);
          offset += chunk_size;
          free(sendBuffer);

          sendBuffer = (char*) malloc(strlen(world_size_c)+strlen(tmp_line_before)+strlen(tmp_line_after)+2);
          strcpy(sendBuffer, world_size_c);
          strcat(sendBuffer, tmp_line_before);
          strcat(sendBuffer, tmp_line_after);
          strcat(sendBuffer, token);
          strcat(sendBuffer, " ");
          strcat(sendBuffer, save);

          free(tmp_line_after);
          free(tmp_line_before);
          tmp_line_before = (char*) malloc(1);
          *tmp_line_before = '\0';
          tmp_line_after = (char*) malloc(1);
          *tmp_line_after = '\0';
        }
        if( pid == num_processes - 1 ) offset = world_size + 1;
        break;
      }
    }
  }

  fclose(fp);

  return sendBuffer;
}

char* receive_input() {
  MPI_Status status[2];
  int length;
  char *receive_buffer;
  MPI_Recv(&length, 1, MPI_INT, 0, CHUNK_SIZE, MPI_COMM_WORLD, &status[0]);
  receive_buffer = (char *) malloc(length);
  MPI_Recv(receive_buffer, length, MPI_CHAR, 0, CHUNK_MSG, MPI_COMM_WORLD, &status[1]);
  return receive_buffer;
}

char* get_movements(int chunk, int start, int end){
  int i, j;
  char* movements = (char*) malloc(1); 
  *movements = '\0';

  for( i = start ;  i < end; i++) {
    for( j = 0 ;  j < world_size ; j++) {
      int x =  i + ((chunk - 1) * chunk_size);
      if( chunk != 1 ) x--;
      int type = world_indexer[i][j].type;
      char sym;
      char* move;

      switch(type) {
        case SQUIRREL:
          sym = 's';
          break;
        case WOLF:
          sym = 'w';
          break;
        case TREE:
          sym = 't';
          break;
        case ICE:
          sym = 'i';
          break;
        case SQUIRREL_TREE:
          sym = '$';
          break;
        default:
          continue;
      }


      move = (char*) malloc(BUFFER);
      sprintf( move, "%d %d %c\n", x, j, sym);

      char* tmp_movements = (char*) malloc( strlen(move) + strlen(movements) + 1);
      strcpy(tmp_movements, movements);
      strcat(tmp_movements, move);

      char* aux = movements;
      movements = tmp_movements;
      free(aux);
    }
  }

  //printf("\n\nMOVEMENTS at pid %d, %s\n\n", chunk, movements);
  return  movements;
}

char* end_result(int pid) {
  if( pid == 0 ) {
    int end_size = world_size - (num_processes- 1) * chunk_size;
    return get_movements( num_processes, 2, end_size + 1);
  } else if( pid == 1 ) {
    return get_movements( pid, 0, chunk_size + 1);
  } else {
    return get_movements( pid, 2, chunk_size + 1);
  }
}

/** Send output to master process */
void final_send(int pid) {
  char* sendBuffer = end_result(pid);
  int length = strlen(sendBuffer) + 1;
  MPI_Send(&length, 1, MPI_INT, 0, END_SIZE, MPI_COMM_WORLD);
  MPI_Send(sendBuffer, length, MPI_CHAR, 0, END_MSG, MPI_COMM_WORLD);
}

/** Process 0 receives the output from all nodes */
char* final_receive() {
  int pid;
  char *result = (char*) malloc (1);
  *result = '\0';

  for( pid = 1 ; pid < num_processes ; pid++) {
    MPI_Status status[2];
    int length;
    char *receive_buffer;

    MPI_Recv(&length, 1, MPI_INT, pid, END_SIZE, MPI_COMM_WORLD, &status[0]);
    receive_buffer = (char *) malloc(length);
    MPI_Recv(receive_buffer, length, MPI_CHAR, pid, END_MSG, MPI_COMM_WORLD, &status[1]);

    char* result_tmp = (char*) malloc( strlen(result) + strlen(receive_buffer) + 1);
    strcpy(result_tmp, result);
    strcat(result_tmp, receive_buffer);

    char* aux = result;
    result = result_tmp;
    free(aux);
  }


  return result;
}


char* calc_line(int line) {
  int i;
  char *result = (char*) malloc(1); *result = '\0';

  for( i = 0 ; i < world_size ; i++ ) {
    int type = world_indexer[line][i].type;
    if( EMPTY != type ) {
      char buffer[BUFFER];
      int breed = world_indexer[line][i].breeding;
      int starve = world_indexer[line][i].starvation;
      int is_breeding = world_indexer[line][i].is_breeding;
      sprintf(buffer, "%d %d %d %d %d %d\n", 0, i, type, breed, starve, is_breeding);

      char* tmp_result = (char*) malloc( strlen(result) + strlen(buffer) + 1);

      strcpy( tmp_result, result);
      strcat( tmp_result, buffer);

      char* tmp = result;
      result = tmp_result;
      free(tmp);
    }
  }

  return  result;
}

void wolf_conflicts(position* position) {
  int x = position->x, y = position->y;
  int type = world_indexer[x][y].type;

  int position_breeding = position->cell->breeding;
  int position_starvation = position->cell->starvation;

  switch(type) {
    case WOLF:
      if( world_indexer[x][y].starvation < position_starvation ) {
        world_indexer[x][y].starvation = position_starvation;
        world_indexer[x][y].breeding = position_breeding;
      }
      else if( world_indexer[x][y].starvation == position_starvation) {
        if( world_indexer[x][y].breeding < position_breeding) {
          world_indexer[x][y].breeding = position_breeding;
        }
      }
      break;
    case EMPTY:
      world_indexer[x][y].starvation = position_starvation;
      world_indexer[x][y].breeding = position_breeding;
      break;
    case SQUIRREL:
      world_indexer[x][y].starvation = wolf_starvation;
      world_indexer[x][y].breeding = position_breeding;
      break;
  }

  world_indexer[x][y].type = WOLF;

}

void squirrel_conflicts(position* position) {
  int x = position->x, y = position->y;
  int type = world_indexer[x][y].type;
  int position_breeding = position->cell->breeding;

  switch(type) {
    case WOLF:
      world_indexer[x][y].starvation = wolf_starvation;
      break;
    case EMPTY:
      world_indexer[x][y].type = SQUIRREL;
      world_indexer[x][y].starvation = 0;
      world_indexer[x][y].breeding = position_breeding;
      break;
    case TREE:
      world_indexer[x][y].type = SQUIRREL_TREE;
      world_indexer[x][y].starvation = 0;
      world_indexer[x][y].breeding = position_breeding;
      break;
    case SQUIRREL_TREE:
      world_indexer[x][y].type = SQUIRREL_TREE;
      world_indexer[x][y].starvation = 0;
      if(  world_indexer[x][y].breeding <= position_breeding ) {
        world_indexer[x][y].breeding = position_breeding;
      } 
      break;
    case SQUIRREL:
      world_indexer[x][y].starvation = 0;
      if(  world_indexer[x][y].breeding <= position_breeding ) {
        world_indexer[x][y].breeding = position_breeding;
      } 
      break;
  }
}

void solve_conflict(position* position) {
  int type = position->cell->type;

  switch(type) {
    case WOLF:
      wolf_conflicts(position);
      break;
    case SQUIRREL:
      squirrel_conflicts(position);
      break;
    case SQUIRREL_TREE:
      squirrel_conflicts(position);
      break;
  }
}

void apply_conflicts(int pid, char * received, int line_number) {
  char *token, *save;

  if ( received == NULL ) return;

  while ( (token = strtok_r(received, "\n", &save)) != NULL ) {
    received = NULL;
    int x, y, type, breed, starve, is_breeding;

    sscanf(token, "%d %d %d %d %d %d", &x, &y, &type, &breed, &starve, &is_breeding);

    position *new_position = (position*) malloc(sizeof(position));
    cell *new_cell = (cell*) malloc(sizeof(cell));

    new_cell->type = type;
    new_cell->breeding = breed;
    new_cell->starvation = starve;
    new_cell->is_breeding = is_breeding;

    new_position->y = y;
    new_position->cell = new_cell;
    new_position->x = line_number;

    solve_conflict(new_position);

    free(new_cell);
    free(new_position);
  }
}

void substitute(int pid, char* line, int line_number) {
  int i;

  // clear the ghost line
  for( i = 0 ; i < world_size ; i++ ) {
    world_indexer[line_number][i].type = 0;
    world_indexer[line_number][i].breeding = 0;
    world_indexer[line_number][i].starvation = 0;
    world_indexer[line_number][i].is_breeding = 0;
  }

  // update the ghost line with most recent values
  char* token, *save;
  while ( (token = strtok_r(line, "\n", &save)) != NULL ) {
    line = NULL;
    int x, y, type, breed, starve, is_breeding;

    sscanf(token, "%d %d %d %d %d %d", &x, &y, &type, &breed, &starve, &is_breeding);

    world_indexer[line_number][y].type = type;
    world_indexer[line_number][y].breeding = breed;
    world_indexer[line_number][y].starvation = starve;
    world_indexer[line_number][y].is_breeding = is_breeding;
  }

}

void apply_received(int pid, char* received) {
  char *line, *movement_array = NULL;
  if( *received == '|' ) { 
    char *save;
    line = strtok_r(received, "|", &save);
    movement_array = NULL;
  }
  else {
    movement_array = strtok_r(received, "|", &line);
  }

  // merge movement_array with line
  if( pid == 0 ) {
    apply_conflicts(pid, movement_array, 1);
    substitute(pid , line, 0);
    apply_conflicts(pid, top_ghost_line, 0);

    //printf("PID %d AFTER APPLY 1\n", pid);
    //print_world(pid, 0);

  } else if( pid == 1 ) {
    apply_conflicts(pid, movement_array, chunk_size - 1);
    substitute(pid , line, chunk_size);
    apply_conflicts(pid, bottom_ghost_line, chunk_size);

    //printf("PID %d AFTER APPLY 1\n", pid);
    //print_world(pid, 0);
  } else {

  }
}


void send_conflicts( char* results, int pid) {
  int length = strlen(results) + 1;
  MPI_Send(&length, 1, MPI_INT, pid, CONFLICT_SIZE, MPI_COMM_WORLD);
  MPI_Send(results, length, MPI_CHAR, pid, CONFLICT_MSG, MPI_COMM_WORLD);
}

char* receive_conflicts( int from_pid ) {
  MPI_Status status[2];
  int length;
  char *receive_buffer;

  MPI_Recv(&length, 1, MPI_INT, from_pid, CONFLICT_SIZE, MPI_COMM_WORLD, &status[0]);
  receive_buffer = (char *) malloc(length);
  MPI_Recv(receive_buffer, length, MPI_CHAR, from_pid, CONFLICT_MSG, MPI_COMM_WORLD, &status[1]);
  return receive_buffer;
}

void resolve_conflicts(int pid) {
  char* line, *received, *send;

  if( pid == 0 ) {
    // joins given line with top-ghost-line
    line = calc_line( 1 );
    send = (char*) malloc( strlen(line) + strlen(top_ghost_line) + 2 );

    strcpy( send, top_ghost_line);
    strcat( send, "|");
    strcat( send, line);

    send_conflicts( send, num_processes - 1);
    received = receive_conflicts( num_processes - 1 );

    apply_received(pid, received);
  } else if( pid == 1 ) {
    line = calc_line( chunk_size -1 );
    send = (char*) malloc( strlen(line) + strlen(bottom_ghost_line) + 2 );

    strcpy( send, bottom_ghost_line);
    strcat( send, "|");
    strcat( send, line);

    send_conflicts( send, (pid + 1) % num_processes );
    received = receive_conflicts( (pid + 1) % num_processes );

    apply_received(pid, received);
  } else {
    /* TODO: resolve 4 2 critical zones */
  }
}

int main(int argc, char *argv[]) {
  int i, pid;

  if ( argc != 6 ) {
    printf("Invalid number of arguments!\n"); 
    return 1;
  }

  MPI_Init(&argc, &argv);

  MPI_Comm_rank (MPI_COMM_WORLD, &pid);
  MPI_Comm_size (MPI_COMM_WORLD, &num_processes);

  printf("Running wolves at %d of %d\n", pid, num_processes);

  wolf_breeding = atoi(argv[2]);
  squirrel_breeding = atoi(argv[3]);
  wolf_starvation = atoi(argv[4]);
  int num_generation = atoi(argv[5]);

  // only the master reads input
  char* input = ( pid == 0 ) ? send_input( fopen(argv[1], "r"), num_processes) : receive_input();

  genesis(input, pid, num_processes);
  print_world(pid, i);

  for( i = 0; i < num_generation; i++) {
    sub_generation(RED_GEN, pid);

    printf("\n\nPID %d TOP\n%s\n\n", pid, top_ghost_line);
    printf("\n\nPID %d BOT\n%s\n\n", pid, bottom_ghost_line);

    resolve_conflicts(pid);
    printf("\n\nPID %d AFTER CONFLICT\n", pid);
    //print_world(pid, i);
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("\n\nPID %d TOP\n%s\n\n", pid, top_ghost_line);
    //printf("\n\nPID %d BOT\n%s\n\n", pid, bottom_ghost_line);
    // print_world(pid, i);
    clear_ghost_line();

    duplicate();

    sub_generation(BLK_GEN, pid);

    resolve_conflicts(pid);
    MPI_Barrier(MPI_COMM_WORLD);
    print_world(pid, i);
    clear_ghost_line();

    update_generation(); 
  }

  if ( pid == 0 ) {
    char* receive_all = final_receive();
    char* receive_0 = end_result(0);

    char* result = (char*) malloc ( strlen(receive_0) + strlen(receive_all) + 1);

    strcpy( result, receive_all);
    strcat( result, receive_0);

    printf("%s", result);
  } else {
    final_send(pid);
  }

  MPI_Finalize();

  return 0;
}
