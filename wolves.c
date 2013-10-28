#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define FALSE 0
#define TRUE 1

#define EMPTY 0
#define SQUIRREL 1
#define WOLF 2
#define TREE 3
#define ICE 4
#define SQUIRREL_TREE 5

typedef struct world {
  int type;
  int breeding_period;
  int starvation_period;
} world;

// GLOBAL variable!!!
world* world_array;
world** world_indexer;
world* world_array_r;
world** world_indexer_r;

int world_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;

void print_final_matrix(int world_size) {
  int i, j;

  for(i = 0; i < world_size; i++) {
    for(j = 0; j < world_size; j++) {
      switch(world_indexer_r[i][j].type) {
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
  }
}

void print_matrix(int world_size) {
  int i, j;
  for(i = 0; i < world_size; i++) {
    for(j = 0; j < world_size; j++) {
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
    printf("|\n");
  }
}

void copy_matrix(int world_size) {
  int i, j;
  world_array_r = (world*)malloc(sizeof(world)*world_size*world_size);
  world_indexer_r = (world**)malloc(sizeof(world*)*world_size);

  for (i = 0; i < world_size; ++i) {
    world_indexer_r[i] = &world_array_r[i*world_size];
  }

  for (i = 0; i < world_size; ++i) {
    for(j = 0; j < world_size; ++j) {
      world_indexer_r[i][j].type = world_indexer[i][j].type;
      world_indexer_r[i][j].breeding_period = world_indexer[i][j].breeding_period;
      world_indexer_r[i][j].starvation_period = world_indexer[i][j].starvation_period;
    }
  }

}

/** Allocates space for the world. */
void initialization(int world_size) {
  int i;

  world_array = (world*)malloc(sizeof(world)*world_size*world_size);
  world_indexer = (world**)malloc(sizeof(world*)*world_size);

  for (i = 0; i < world_size; ++i) {
    world_indexer[i] = &world_array[i*world_size];
  }
}

/** Read's file and populates the world */
void genesis(FILE *fp, int wolves_breeding_period, int squirrels_breeding_period, int wolves_starvation_period) {
  int world_x, world_y;
  int entity_number;
  char entity;

  fscanf(fp, "%d", &world_size);
  initialization(world_size);

  while (fscanf(fp, "%d %d %c",&world_x, &world_y, &entity) != EOF) { // expect 1 successful conversion
    switch(entity) {
      case 's':
        entity_number = SQUIRREL;
        world_indexer[world_x][world_y].breeding_period = squirrels_breeding_period;
        world_indexer[world_x][world_y].starvation_period = 0;
        break;
      case 'w':
        entity_number = WOLF;
        world_indexer[world_x][world_y].breeding_period = wolves_breeding_period;
        world_indexer[world_x][world_y].starvation_period = wolves_starvation_period;
        break;
      case 't':
        entity_number = TREE;
        world_indexer[world_x][world_y].breeding_period = 0;
        world_indexer[world_x][world_y].starvation_period = 0;
        break;
      case 'i':
        entity_number = ICE;
        world_indexer[world_x][world_y].breeding_period = 0;
        world_indexer[world_x][world_y].starvation_period = 0;
        break;
      default:
        entity_number = EMPTY;
        world_indexer[world_x][world_y].breeding_period = 0;
        world_indexer[world_x][world_y].starvation_period = 0;
        break;
    }
    world_indexer[world_x][world_y].type = entity_number;
  }
}

int is_free_position(int x, int y, int type){
  if( type == WOLF) {
    return  world_indexer_r[x][y].type == EMPTY || world_indexer_r[x][y].type == SQUIRREL;
  } else {
    // for types squirrel and squirrel tree
    return  world_indexer_r[x][y].type == EMPTY || world_indexer_r[x][y].type == TREE;
  }
}

int can_move_up(int x, int y, int type) {
  return x - 1 >= 0 && is_free_position(x-1, y, type);
}

int can_move_right(int x, int y, int type) {
  return y + 1 < world_size && is_free_position(x, y+1, type);
}

int can_move_down(int x, int y, int type) {
  return x + 1 < world_size && is_free_position(x+1, y, type);
}

int can_move_left(int x, int y, int type) {
  return y - 1 >= 0 && is_free_position(x, y-1, type);
}

int* find_free_positions(int x, int y, int type) {
  int *array = (int*) malloc(sizeof(int)*4);

  array[UP] = can_move_up(x,y, type);
  array[RIGHT] = can_move_right(x, y, type);
  array[DOWN] = can_move_down(x, y, type);
  array[LEFT] = can_move_left(x, y, type);

  return array;
}

int* find_squirrels(int x, int y) {
  int *array = (int*) malloc(sizeof(int)*4);

  array[UP] = x - 1 >= 0 && world_indexer_r[x-1][y].type == SQUIRREL;
  array[RIGHT] = y + 1 < world_size && world_indexer_r[x][y+1].type == SQUIRREL;
  array[DOWN] = x + 1 < world_size && world_indexer_r[x+1][y].type == SQUIRREL;
  array[LEFT] = y - 1 >= 0 && world_indexer_r[x][y-1].type == SQUIRREL;

  return array;
}

int count_free_positions(int* free_positions) {
  int i, counter = 0;

  for ( i = 0; i < 4 ; i++){
    counter += free_positions[i] ? 1 : 0; 
  }

  return counter;
}

int find_next_positon(int x, int y, int type) {
  int * free_positions = find_free_positions( x, y, type);
  int * free_squirrels = find_squirrels(x, y);
  int * specific_positions;
  int c = x * world_size + y, position, direction;

  if( type == WOLF && count_free_positions(free_squirrels) > 0){
    position = c % count_free_positions(free_squirrels);
    specific_positions = free_squirrels;
  } else {
    position = c % count_free_positions(free_positions);
    specific_positions = free_positions;
  }

  //printf("Moving x: %d y: %d\n", x, y);
  //printf("Free positions: %d %d %d %d\n", free_positions[0], free_positions[1], free_positions[2], free_positions[3]);
  //printf("Sq positions: %d %d %d %d\n", free_squirrels[0], free_squirrels[1], free_squirrels[2], free_squirrels[3]);

  //printf("Position: %d\n", position);

  for ( direction = 0; direction < 4 ; direction++){
    if( specific_positions[direction] ) {
      position--;
    }
    if ( position == -1 ) {
      return direction;
    }
  }

  //no free positions found
  return -1;
}

void update_position(int x, int y, int type, int breeding, int starvation) {
  world_indexer[x][y].type = type;
  world_indexer[x][y].breeding_period = breeding;
  world_indexer[x][y].starvation_period = starvation;
}

void move_squirrel(int from_x, int from_y, int to_x, int to_y, int breeding, int starvation) {
  int to_type = world_indexer_r[to_x][to_y].type;

  if( to_type == TREE) {
    update_position( to_x, to_y, SQUIRREL_TREE, breeding, starvation);
  } else if( to_type ==  SQUIRREL ) {
    update_position( to_x, to_y, SQUIRREL, breeding, starvation);
  } else {
    update_position( to_x, to_y, SQUIRREL, breeding, starvation);
  }

  update_position(from_x, from_y, EMPTY, breeding, starvation);
}

void go_up(int x, int y, int type, int breeding, int starvation) {
  if( type == SQUIRREL ) {
    move_squirrel(x, y, x - 1, y, breeding, starvation);
  }
}

void go_right(int x, int y, int type, int breeding, int starvation) {
  if( type == SQUIRREL ) {
    move_squirrel(x, y, x, y + 1, breeding, starvation);
  }
}

void go_down(int x, int y, int type, int breeding, int starvation) {
  if( type == SQUIRREL ) {
    move_squirrel(x, y, x + 1, y, breeding, starvation);
  }
}

void go_left(int x, int y, int type, int breeding, int starvation) {
  if( type == SQUIRREL ) {
    move_squirrel(x, y, x, y - 1, breeding, starvation);
  }
}

void move(int x, int y, int type) {
  int next_position = find_next_positon(x, y, type);
  int breeding = 0, starvation = 0;

  switch(next_position) {
    case UP:
      go_up(x, y, type, breeding, starvation);
      break;
    case RIGHT:
      go_right( x, y, type, breeding, starvation);
      break;
    case DOWN:
      go_down( x, y, type, breeding, starvation);
      break;
    case LEFT:
      go_left( x, y, type, breeding, starvation);
      break;
    default:
      return;
  }
}

void exodus(int x, int y){
  int type =  world_indexer_r[x][y].type;

  if( type == SQUIRREL || type == WOLF || type == SQUIRREL_TREE ) {
    move(x, y, type);
  }
}

void sub_generation(int black_generation){
  int i, j;

  for(i = 0; i < world_size; i++){
    if( black_generation ) {
      j = ( i % 2 == 0) ? 1 : 0;
    } else {
      j = ( i % 2 == 0) ? 0 : 1;
    }
    while( j < world_size) {
      exodus(i, j);
      j = j + 2;
    }
  }
}


int main(int argc, char *argv[]) {
  printf("Welcome to Squirrels and Wolves\n");

  if ( argc != 6 ) {
    printf("Invalid number of arguments!\n"); 
    return 1;
  }

  wolf_breeding = atoi(argv[2]);
  squirrel_breeding = atoi(argv[3]);
  wolf_starvation = atoi(argv[4]);
  int num_generation = atoi(argv[5]);

  // process input
  genesis( fopen(argv[1], "r"), wolf_breeding, squirrel_breeding, wolf_starvation);



  printf("Original:\n");
  print_matrix(world_size); fflush(stdout);
  printf("\n"); fflush(stdout);
  // process generations
  while( num_generation > 0 ) {
    printf("======== GEN\n"); fflush(stdout);
    // 1st subgeneration
    copy_matrix(world_size);
    sub_generation(FALSE);
    printf("\nRed\n");
    print_matrix(world_size); fflush(stdout);
    printf("\n"); fflush(stdout);

    // 2nd subgeneration
    copy_matrix(world_size);
    sub_generation(TRUE);
    printf("\nBlack\n");
    print_matrix(world_size); fflush(stdout);
    printf("\n"); fflush(stdout);

    num_generation--;
  }

  printf("Final:\n");
  print_final_matrix(world_size);

  return 0;
}
