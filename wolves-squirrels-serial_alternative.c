#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	int breeding_flag;
	int starvation_flag;
} world;

typedef struct pos_move {
	int x;
	int y;
	int on_tree;
} pos_move;

// GLOBAL variable!!!
world* animal_world_array;
world** animal_world_indexer;
world* animal_world_array_read;
world** animal_world_indexer_read;

int world_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;

void print_final_matrix(int world_size) {
	int i, j;

	for(i = 0; i < world_size; i++) {
		for(j = 0; j < world_size; j++) {
			switch(animal_world_indexer_read[i][j].type) {
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
			switch(animal_world_indexer[i][j].type) {
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
	animal_world_array_read = (world*)malloc(sizeof(world)*world_size*world_size);
	animal_world_indexer_read = (world**)malloc(sizeof(world*)*world_size);

	for (i = 0; i < world_size; ++i)
	{
		animal_world_indexer_read[i] = &animal_world_array_read[i*world_size];
	}

	for (i = 0; i < world_size; ++i)
	{
		for(j = 0; j < world_size; ++j) {
			animal_world_indexer_read[i][j].type = animal_world_indexer[i][j].type;
			animal_world_indexer_read[i][j].breeding_period = animal_world_indexer[i][j].breeding_period;
			animal_world_indexer_read[i][j].starvation_period = animal_world_indexer[i][j].starvation_period;
			animal_world_indexer_read[i][j].breeding_flag = animal_world_indexer[i][j].breeding_flag;
			animal_world_indexer_read[i][j].starvation_flag = animal_world_indexer[i][j].starvation_flag;
		}
	}

}

/* allocation */
void initialization(int world_size) {
	int i;

	animal_world_array = (world*)malloc(sizeof(world)*world_size*world_size);

	animal_world_indexer = (world**)malloc(sizeof(world*)*world_size);

	for (i = 0; i < world_size; ++i)
	{
		animal_world_indexer[i] = &animal_world_array[i*world_size];
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
			animal_world_indexer[world_x][world_y].breeding_period = squirrels_breeding_period;
			animal_world_indexer[world_x][world_y].starvation_period = 0;
			break;
			case 'w':
			entity_number = WOLF;
			animal_world_indexer[world_x][world_y].breeding_period = wolves_breeding_period;
			animal_world_indexer[world_x][world_y].starvation_period = wolves_starvation_period;
			break;
			case 't':
			entity_number = TREE;
			animal_world_indexer[world_x][world_y].breeding_period = 0;
			animal_world_indexer[world_x][world_y].starvation_period = 0;
			break;
			case 'i':
			entity_number = ICE;
			animal_world_indexer[world_x][world_y].breeding_period = 0;
			animal_world_indexer[world_x][world_y].starvation_period = 0;
			break;
			default:
			entity_number = EMPTY;
			animal_world_indexer[world_x][world_y].breeding_period = 0;
			animal_world_indexer[world_x][world_y].starvation_period = 0;
			break;
		}
		animal_world_indexer[world_x][world_y].type = entity_number;
	}
}

void update_position(int x, int y, int type, int breeding, int breeding_flag, int starvation, int starvation_flag){

	// Animais do mesmo tipo
	if(animal_world_indexer[x][y].type == type) {
		if(type == WOLF) {
			// O lobo que se move para a posicao tem maior starvation period e ganha.
			if (starvation > animal_world_indexer[x][y].starvation_period)
			{
				animal_world_indexer[x][y].starvation_period = starvation;
				animal_world_indexer[x][y].breeding_period = breeding;
			}
			// Os lobos tem o mesmo starvation period
			else if (animal_world_indexer[x][y].starvation_period == starvation) {
				animal_world_indexer[x][y].breeding_period = animal_world_indexer[x][y].breeding_period <= breeding ? animal_world_indexer[x][y].breeding_period : breeding;
			}
			// Se o lobo que ja esta na posicao tem maior starvation period, nada se altera

		// Squirrel with a breeding period wins
		} else if(type == SQUIRREL || type == SQUIRREL_TREE) {
			animal_world_indexer[x][y].breeding_period = animal_world_indexer[x][y].breeding_period <= breeding ? animal_world_indexer[x][y].breeding_period : breeding;
		}
	}

	// Suicidal squirrel jumps to the wolfs' mouth
	else if ((animal_world_indexer[x][y].type == WOLF && type == SQUIRREL)) {
		animal_world_indexer[x][y].starvation_period = wolf_starvation;
	}
	// Lucky wolf eats a squirrel
	else if (animal_world_indexer[x][y].type == SQUIRREL && type == WOLF) {
		animal_world_indexer[x][y].type = WOLF;
		animal_world_indexer[x][y].starvation_period = wolf_starvation;
	// Moves to an empty position
	} else {
		animal_world_indexer[x][y].type = type;
		animal_world_indexer[x][y].breeding_period = breeding;
		animal_world_indexer[x][y].starvation_period = starvation;
	}
	animal_world_indexer[x][y].breeding_flag = breeding_flag;
	animal_world_indexer[x][y].starvation_flag = starvation_flag;
}

void move(int num_generation, int type, int x, int y) {
	int c, sum, next;
	pos_move array[4];

	// Oh bia, aqui ficam os teus esquilinhos, eventualmente aquele que o hugo te vai dar
	if( type == SQUIRREL || type == SQUIRREL_TREE) {
		sum = 0;
		if((x - 1 >= 0) && (animal_world_indexer_read[x-1][y].type == EMPTY || animal_world_indexer_read[x-1][y].type == TREE)) {
			array[sum].x=x-1;
			array[sum].y=y;
			array[sum].on_tree = animal_world_indexer_read[x-1][y].type == TREE ? 1 : 0;
			sum++;
		}
		if((y + 1 < world_size) && (animal_world_indexer_read[x][y+1].type == EMPTY || animal_world_indexer_read[x][y+1].type == TREE)) {
			array[sum].x=x;
			array[sum].y=y+1;
			array[sum].on_tree = animal_world_indexer_read[x][y+1].type == TREE ? 1 : 0;
			sum++;
		}
		if((x + 1 < world_size) && (animal_world_indexer_read[x+1][y].type == EMPTY || animal_world_indexer_read[x+1][y].type == TREE)) {
			array[sum].x=x+1;
			array[sum].y=y;
			array[sum].on_tree = animal_world_indexer_read[x+1][y].type == TREE ? 1 : 0;
			sum++;
		}
		if((y - 1 >= 0 ) && (animal_world_indexer_read[x][y-1].type == EMPTY || animal_world_indexer_read[x][y-1].type == TREE)) {
			array[sum].x=x;
			array[sum].y=y-1;
			array[sum].on_tree = animal_world_indexer_read[x][y-1].type == TREE ? 1 : 0;
			sum++;
		}

		if( sum == 0 ){
			if(animal_world_indexer_read[x][y].breeding_flag < num_generation) {
				if(animal_world_indexer_read[x][y].breeding_period > 0) {
					animal_world_indexer[x][y].breeding_period--;
				}
				animal_world_indexer[x][y].breeding_flag = num_generation;
			}
		} else {
			c = x * world_size + y;
			next = c % sum;
			if(animal_world_indexer_read[x][y].breeding_period == 0 ) {
				if(type == SQUIRREL_TREE){
					update_position(x, y, SQUIRREL_TREE, squirrel_breeding, num_generation, 0, 0);
				} else {
					update_position(x, y, SQUIRREL, squirrel_breeding, num_generation, 0, 0);
				}
				if(array[next].on_tree) {
					update_position(array[next].x, array[next].y, SQUIRREL_TREE, squirrel_breeding, num_generation, 0, 0);
				}
				else {
					update_position(array[next].x, array[next].y, SQUIRREL, squirrel_breeding, num_generation, 0, 0);
				}
			}
			else {
				if(array[next].on_tree) {
					update_position(array[next].x, array[next].y, SQUIRREL_TREE, animal_world_indexer_read[x][y].breeding_period - 1, num_generation, 0, 0);
				}
				else {
					update_position(array[next].x, array[next].y, SQUIRREL, animal_world_indexer_read[x][y].breeding_period - 1, num_generation, 0, 0);
				}
				if(type == SQUIRREL_TREE) {
					update_position(x,y, TREE, 0, 0, 0, 0);
				}
				else {
					update_position(x,y, EMPTY, 0, 0, 0, 0);
				}
			}
		}
	} else if (type == WOLF) {

		if(animal_world_indexer_read[x][y].starvation_period == 0) {
			update_position(x,y, EMPTY, 0, 0, 0, 0);
			return;
		}

		sum = 0;
		//Find squirls
		if((x - 1 >= 0) && (animal_world_indexer_read[x-1][y].type == SQUIRREL)) {
			array[sum].x=x-1;
			array[sum].y=y;
			sum++;
		}
		if((y + 1 < world_size) && (animal_world_indexer_read[x][y+1].type == SQUIRREL)) {
			array[sum].x=x;
			array[sum].y=y+1;
			sum++;
		}
		if((x + 1 < world_size) && (animal_world_indexer_read[x+1][y].type == SQUIRREL)) {
			array[sum].x=x+1;
			array[sum].y=y;
			sum++;
		}
		if((y - 1 >= 0 ) && (animal_world_indexer_read[x][y-1].type == SQUIRREL)) {
			array[sum].x=x;
			array[sum].y=y-1;
			sum++;
		}

		if(sum == 1) {
			if(animal_world_indexer_read[x][y].breeding_period == 0 ) {
				update_position(x,y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
				update_position(array[0].x, array[0].y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
			}
			else {
				update_position(array[0].x, array[0].y, WOLF, animal_world_indexer_read[x][y].breeding_period - 1, num_generation, animal_world_indexer_read[x][y].starvation_period - 1, num_generation);
				update_position(x,y, EMPTY, 0, 0, 0, 0);
			}

		} else if(sum > 1) {
			c = x * world_size + y;
			next = c % sum;

			if(animal_world_indexer_read[x][y].breeding_period == 0 ) {
				update_position(x,y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
				update_position(array[next].x, array[next].y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
			}
			else {
				update_position(array[next].x, array[next].y, WOLF, animal_world_indexer_read[x][y].breeding_period - 1, num_generation, animal_world_indexer_read[x][y].starvation_period - 1, num_generation);
				update_position(x,y, EMPTY, 0, 0, 0, 0);
			}
		} else {
			sum = 0;
			if((x - 1 >= 0) && (animal_world_indexer_read[x-1][y].type == EMPTY)) {
				array[sum].x=x-1;
				array[sum].y=y;
				sum++;
			}
			if((y + 1 < world_size) && (animal_world_indexer_read[x][y+1].type == EMPTY)) {
				array[sum].x=x;
				array[sum].y=y+1;
				sum++;
			}
			if((x + 1 < world_size) && (animal_world_indexer_read[x+1][y].type == EMPTY)) {
				array[sum].x=x+1;
				array[sum].y=y;
				sum++;
			}
			if((y - 1 >= 0 ) && (animal_world_indexer_read[x][y-1].type == EMPTY)) {
				array[sum].x=x;
				array[sum].y=y-1;
				sum++;
			}

			if( sum == 0) {
				if(animal_world_indexer_read[x][y].breeding_flag < num_generation) {
					if(animal_world_indexer_read[x][y].breeding_period > 0) {
						animal_world_indexer[x][y].breeding_period--;
					}
					if(animal_world_indexer_read[x][y].starvation_period > 0) {
						animal_world_indexer[x][y].starvation_period--;
					}
					animal_world_indexer[x][y].breeding_flag = num_generation;
					animal_world_indexer[x][y].starvation_flag = num_generation;
				}
			} else {
				c = x * world_size + y;
				next = c % sum;
				printf("c %d, sum %d\n", c, sum);
				printf("x %d, y %d\n", x, y);
				printf("next %d, array[next].x %d, array[next].y %d\n", next, array[next].x, array[next].y);

				if(animal_world_indexer_read[x][y].breeding_period == 0 ) {
					update_position(x,y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
					update_position(array[next].x, array[next].y, WOLF, wolf_breeding, num_generation, wolf_starvation, num_generation);
				}
				else {
					update_position(array[next].x, array[next].y, WOLF, animal_world_indexer_read[x][y].breeding_period - 1, num_generation, animal_world_indexer_read[x][y].starvation_period - 1, num_generation);
					update_position(x,y, EMPTY, 0, 0, 0, 0);
				}
			}
		}
	}
}

void exodus(int num_generation, int x, int y){
	switch(animal_world_indexer[x][y].type) {
		case EMPTY:
		break;
		case TREE:
		break;
		case ICE:
		break;
		case SQUIRREL:
		move(num_generation, SQUIRREL, x, y);
		break;
		case WOLF:
		move(num_generation, WOLF, x, y);
		break;
		case SQUIRREL_TREE:
		move(num_generation, SQUIRREL_TREE, x, y);
		break;
	}
}

void red_revelation(int num_generation){
	int i, j;

	for(i = 0; i < world_size; i++){
		if( i % 2 == 0) {
			j = 0;
		} else {
			j = 1;
		}

		for( ; j < world_size; j = j + 2){
			exodus(num_generation, i, j);
		}
	}
}

void black_lamentation(int num_generation){
	int i, j;

	for(i = 0; i < world_size; i++){
		if( i % 2 == 0) {
			j = 1;
		} else {
			j = 0;
		}

		for( ; j < world_size; j = j + 2){
			exodus(num_generation, i, j);
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
	genesis( fopen(argv[1], "r"), wolf_breeding, squirrel_breeding, wolf_starvation);
	print_matrix(world_size);

	while( num_generation > 0 ) {
		copy_matrix(world_size);
		red_revelation(num_generation);
		printf(">>>>RED<<<<\n");
		print_matrix(world_size);
		copy_matrix(world_size);
		black_lamentation(num_generation);
		printf(">>>>BLACK<<<<\n");
		print_matrix(world_size);
		num_generation--;
	}
	printf("Final:\n");
	//print_matrix(world_size);
	print_final_matrix(world_size);

	return 0;
}