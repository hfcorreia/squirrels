#include <stdlib.h>
#include <stdio.h>

#define MAX 10
#define SQUIRREL 1
#define WOLF 2
#define TREE 3
#define ICE 4

struct world {
	int type; //char
	int breeding_period;
	int starvation_period;
} world[MAX][MAX];

void print_matrix() {
	int i, j;
	for(i = 0; i < MAX; i++) {
		for(j = 0; j < MAX; j++) {
			printf("%d", world[i][j].type);
		}
		printf("\n");
	}
}

void genesis(int world_x, int world_y, char entity) {
	int entity_number;

	switch(entity) {
		case 's': entity_number = SQUIRREL;
		break;
		case 'w': entity_number = WOLF;
		break;
		case 't': entity_number = TREE;
		break;
		case 'i': entity_number = ICE;
		break;
	}
	world[world_x][world_y].type = entity_number;

}

int main(int argc, char *argv[]){
	printf("Welcome to Squirrels and Wolves\n");

	int world_size, wolves_breeding_period, squirrels_breeding_period, 
	wolf_starvation_period, number_generations;
	FILE * fp;

	fp = fopen(argv[1], "r");

	fscanf(fp, "%d", &world_size);

	printf("%d\n", world_size);

	int world_x, world_y;
	char entity;

	while (fscanf(fp, "%d %d %c",&world_x,&world_y,&entity) != EOF){ // expect 1 successful conversion
		printf("world-x%d world-y%d entity-%c\n", world_x, world_y, entity);

		genesis(world_x, world_y, entity);
	}
	
	scanf("%d", &wolves_breeding_period);
	scanf("%d", &squirrels_breeding_period);
	scanf("%d", &wolf_starvation_period);
	scanf("%d", &number_generations);

	print_matrix();

	return 0;
}