#include <stdlib.h>
#include <stdio.h>

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
void genesis(position_info** pos_info, int wolves_breeding_period, int squirrels_breeding_period, int wolf_starvation_period) {
	int i, entity_number;

	for(i = 0; pos_info[i] != NULL; i++) {
		switch(pos_info[i]->entity) {
			case 's': entity_number = SQUIRREL;
			animal_world[pos_info[i]->pos_x][pos_info[i]->pos_y]->breeding_period = squirrels_breeding_period;
			break;
			case 'w': entity_number = WOLF;
			animal_world[pos_info[i]->pos_x][pos_info[i]->pos_y]->breeding_period = wolves_breeding_period;
			animal_world[pos_info[i]->pos_x][pos_info[i]->pos_y]->starvation_period = wolf_starvation_period;
			break;
			case 't': entity_number = TREE;
			break;
			case 'i': entity_number = ICE;
			break;
		}
		animal_world[pos_info[i]->pos_x][pos_info[i]->pos_y]->type = entity_number;
		free(pos_info[i]);
	}
	free(pos_info);

}

int main(int argc, char *argv[]){
	printf("Welcome to Squirrels and Wolves\n");

	int world_size, world_x, world_y, entity_number;
	char entity;
	FILE * fp;

	fp = fopen(argv[1], "r");

	fscanf(fp, "%d", &world_size);

	printf("%d\n", world_size);

	initialization(world_size);

	while (fscanf(fp, "%d %d %c",&world_x, &world_y, &entity) != EOF){ // expect 1 successful conversion

		switch(entity) {
			case 's': entity_number = SQUIRREL;
			animal_world[world_x][world_y]->breeding_period = atoi(argv[3]);
			break;
			case 'w': entity_number = WOLF;
			animal_world[world_x][world_y]->breeding_period = atoi(argv[2]);
			animal_world[world_x][world_y]->starvation_period = atoi(argv[3]);
			break;
			case 't': entity_number = TREE;
			break;
			case 'i': entity_number = ICE;
			break;
		}
		animal_world[world_x][world_y]->type = entity_number;

	}
	//genesis(pos_info, wolves_breeding_period, squirrels_breeding_period, wolf_starvation_period);

	print_matrix(world_size);

	return 0;
}