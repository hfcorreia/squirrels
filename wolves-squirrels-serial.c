#include <stdlib.h>
#include <stdio.h>

#define EMPTY 0
#define SQUIRREL 1
#define WOLF 2
#define TREE 3
#define ICE 4

typedef struct world {
  int type; //char
  int breeding_period;
  int starvation_period;
} world;

typedef struct position_info {
  int pos_x;
  int pos_y;
  char entity;
} position_info;

// GLOBAL variable!!!
world*** animal_world;
int world_size;

/* prints the matrix state */
void print_matrix(int world_size) {
  int i, j;
  for(i = 0; i < world_size; i++) {
    for(j = 0; j < world_size; j++) {
      printf("%d", animal_world[i][j]->type);
      free(animal_world[i][j]);
    }
    printf("\n");
    free(animal_world[i]);
  }
  free(animal_world);
}

/* allocation */
void initialization(int world_size) {
  int i, j;
  animal_world = (world***)malloc(sizeof(world**)*world_size);

  for (i = 0; i < world_size; i++)
  {
    animal_world[i] = (world**)malloc(sizeof(world*)*world_size);
    for (j = 0; j < world_size; j++)
    {
      animal_world[i][j] = (world*)malloc(sizeof(world));
    }
  }
}

/* fills the matrix with entities */
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
        animal_world[world_x][world_y]->breeding_period = squirrels_breeding_period;
        animal_world[world_x][world_y]->starvation_period = 0;
        break;
      case 'w': 
        entity_number = WOLF;
        animal_world[world_x][world_y]->breeding_period = wolves_breeding_period;
        animal_world[world_x][world_y]->starvation_period = wolves_starvation_period;
        break;
      case 't': 
        entity_number = TREE;
        animal_world[world_x][world_y]->breeding_period = 0;
        animal_world[world_x][world_y]->starvation_period = 0;
        break;
      case 'i': 
        entity_number = ICE;
        animal_world[world_x][world_y]->breeding_period = 0;
        animal_world[world_x][world_y]->starvation_period = 0;
        break;
      default:
        entity_number = EMPTY;
        animal_world[world_x][world_y]->breeding_period = 0;
        animal_world[world_x][world_y]->starvation_period = 0;
        break;
    }
    animal_world[world_x][world_y]->type = entity_number;
  }
}

int main(int argc, char *argv[]) {
  printf("Welcome to Squirrels and Wolves\n");

  if ( argc != 6 ) {
    printf("Invalid number of arguments!\n"); 
    return 1;
  }

  int wolf_breeding = atoi(argv[2]);
  int squirrel_breeding = atoi(argv[3]);
  int wolf_starvation = atoi(argv[4]);
  int num_generation = atoi(argv[5]);

  genesis( fopen(argv[1], "r"), wolf_breeding, squirrel_breeding, wolf_starvation);

  while( num_generation > 0 ){
  }
  
  return 0;
}
