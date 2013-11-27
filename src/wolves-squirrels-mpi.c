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

typedef struct world_cell {
    int type;
    int breeding;
    int starvation;
} cell;

typedef struct position {
    int x;
    int y;
    cell* cell;
    int is_breeding;
} position;

cell* world_array;
cell** world_indexer;

cell* world_array_read;
cell** world_indexer_read;

int world_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;

/** prints the final output */
void output(int pid) {
    int i, j;

    printf("Final at %d:\n", pid); 
    for(i = 0; i < world_size; i++) {
        for(j = 0; j < world_size; j++) {
            switch(world_indexer[i][j].type) {
                case SQUIRREL:
                    printf("%d %d %c\n", i, j, 's');
                    break;
                case WOLF:
                    printf("%d %d %c\n", i, j, 'w');
                    break;
                case TREE:
                    printf("%d %d %c\n", i, j, 't');
                    break;
                case ICE:
                    printf("%d %d %c\n", i, j, 'i');
                    break;
                case SQUIRREL_TREE:
                    printf("%d %d %c\n", i, j, '$');
                    break;
            }
        }
    };
    
    fflush(stdout);
}

void print_world(int pid, int world_size, int chunk_size, int end_size) {
    int i, j;

    printf("WORLD %d, chunk_size %d: PID %d\n", world_size, chunk_size, pid);
    if(pid == 0) chunk_size = end_size;
    for(i = -1; i < chunk_size ; i++) {
        for(j = 0; j < world_size ; j++) {
            if( i == -1) {
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
    printf("READ\n");
    for(i = -1; i < chunk_size ; i++) {
        for(j = 0; j < world_size ; j++) {
            if( i == -1) {
                printf("|%d", j % 10);
            } else {
                printf("%c", '|');
                switch(world_indexer_read[i][j].type) {
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
}

/** Read's file and populates the world */
void genesis(char *input, int pid, int num_processes) { 
    char* save;
    char* token = strtok_r(input, "\n", &save);
    int chunk;

    world_size = atoi(token);
    int chunk_size = world_size / num_processes;
    int end_size; 
    
    if( pid == 0 )  {
      chunk = num_processes;
      end_size = world_size - (chunk - 1) * chunk_size;
      initialization(world_size, end_size);
    }
    else {
      chunk = pid;
      initialization(world_size, chunk_size);
    }


    while ( (token = strtok_r(NULL, "\n", &save)) != NULL ) {
      int x, y, type_num;
      char type;

      sscanf( token, "%d %d %c", &x, &y, &type);
    //  printf("antes x=%d,y=%d,type= %c\n", x, y, type);
      x =  x - ((chunk-1)*chunk_size);
     // printf("depois x=%d,y=%d,type= %c\n", x, y, type);
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

    print_world(pid, world_size, chunk_size, end_size );
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
    array[DOWN] = x + 1 < world_size && is_free_position(x+1, y, type);
    array[LEFT] = y - 1 >= 0 && is_free_position(x, y-1, type);

    return array;
}

int* find_squirrels(position *actual) {
    int *array = (int*) calloc(4, sizeof(int));
    int x = actual->x, y = actual->y;

    array[UP] = x - 1 >= 0 && world_indexer_read[x-1][y].type == SQUIRREL;
    array[RIGHT] = y + 1 < world_size && world_indexer_read[x][y+1].type == SQUIRREL;
    array[DOWN] = x + 1 < world_size && world_indexer_read[x+1][y].type == SQUIRREL;
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

int find_next_positon(position *actual) {
    int* free_positions = find_free_positions(actual);
    int* free_squirrels = find_squirrels(actual);
    int c = actual->x * world_size + actual->y;

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
}

void breed(position *actual) {
    int x = actual->x, y = actual->y;
    if( actual->is_breeding ) {
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
}

void move_element(position* actual, position* next) {
    if( actual->is_breeding ) {
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

void go(position *actual) {
    int next = find_next_positon(actual);

    switch(next) {
        case UP:
            go_up( actual );
            break;
        case RIGHT:
            go_right( actual );
            break;
        case DOWN:
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

void exodus(int x, int y) {
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
            actual->is_breeding = 1;
        }

        go(actual);
    }

    free(actual);
}

void sub_generation(int is_black_gen ){
    int i, j;

    for(i = 0; i < world_size; i++) {
      int k;
        if( is_black_gen ) {
            k = ( i % 2 == 0) ? 1 : 0;
        } else {
            k = ( i % 2 == 0) ? 0 : 1;
        }

        for( j = k ; j < world_size; j = j + 2) {
            exodus(i, j);
        }
    }
}

void update_generation() {
    int i, j;
    
    for( i = 0 ; i < world_size ; i++ ) {
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
    memcpy(world_array_read, world_array, world_size*world_size*sizeof(cell));
    for (i = 0; i < world_size; ++i) {
        world_indexer_read[i] = &world_array_read[i*world_size];
    }
}

void duplicate() {
    memcpy(world_array_read, world_array, world_size*world_size*sizeof(cell));
    int i;

    for (i = 0; i < world_size; ++i) {
        world_indexer_read[i] = &world_array_read[i*world_size];
    }
}

#define BUFFER 1000

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
    int world_size = atoi(buffer);
    int chunk_size = world_size / num_processes;
    int offset = chunk_size, pid;

    for( pid = 1 ; pid < num_processes + 1; pid++){
      while( fgets(buffer, BUFFER, fp) != NULL) {
        char *save;
        char *token = strtok_r(buffer, " ", &save);
        int x = atoi(token);
        
        if( x < offset ) {
          char *result = (char*) malloc( strlen(sendBuffer) + strlen(token) + strlen(save) + 2);

          strcpy(result, sendBuffer);
          strcat(result, token);
          strcat(result, " ");
          strcat(result, save);

          char* tmp = sendBuffer;
          sendBuffer = result;
          free(tmp);
        } else {
          if(pid != num_processes) {
            int length = strlen(sendBuffer) + 1;
            MPI_Send(&length, 1, MPI_INT, pid, CHUNK_SIZE, MPI_COMM_WORLD);
            MPI_Send(sendBuffer, length, MPI_CHAR, pid, CHUNK_MSG, MPI_COMM_WORLD);
            offset += chunk_size;
            free(sendBuffer);

            sendBuffer = (char*) malloc(strlen(world_size_c)+strlen(token)+strlen(save)+2);
            strcpy(sendBuffer, world_size_c);
            strcat(sendBuffer, token);
            strcat(sendBuffer, " ");
            strcat(sendBuffer, save);
          }
          if( pid == num_processes - 1 ) offset = world_size;
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

int main(int argc, char *argv[]) {
    int i, pid, num_processes;

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


    if( 1 == 0 ) {
      for( i = 0; i < num_generation; i++) {
        sub_generation(RED_GEN);

        duplicate();
        sub_generation(BLK_GEN);

        update_generation();
      }
    }

    MPI_Finalize();

    //output(pid); fflush(stdout);
    return 0;
}
