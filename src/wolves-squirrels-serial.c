#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
void output(void) {
    int i, j;

    printf("Final:\n"); 
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

void print_world(void) {
    int i, j;
    for(i = -1; i < world_size; i++) {
        for(j = 0; j < world_size; j++) {
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
    for(i = -1; i < world_size; i++) {
        for(j = 0; j < world_size; j++) {
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
void initialization() {
    int i;

    world_array = (cell*) calloc( world_size*world_size , sizeof(cell));   
    world_indexer = (cell**) calloc( world_size, sizeof(cell*));
    world_array_read = (cell*) calloc( world_size*world_size , sizeof(cell));   
    world_indexer_read = (cell**) calloc( world_size, sizeof(cell*));

    for (i = 0; i < world_size; ++i) {
        world_indexer[i] = &world_array[i*world_size];
        world_indexer_read[i] = &world_array_read[i*world_size];
    }
}

/** Read's file and populates the world */
void genesis(FILE *fp) {
    int x, y, type_num;
    char type;

    fscanf(fp, "%d", &world_size);

    initialization(world_size);

    while (fscanf(fp, "%d %d %c", &x, &y, &type) != EOF) {
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
            default:
                type_num = EMPTY;
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

int main(int argc, char *argv[]) {
    int i;
    clock_t start_t;

    if ( argc != 6 ) {
        printf("Invalid number of arguments!\n"); 
        return 1;
    }

    wolf_breeding = atoi(argv[2]);
    squirrel_breeding = atoi(argv[3]);
    wolf_starvation = atoi(argv[4]);
    int num_generation = atoi(argv[5]);

    // process input
    genesis( fopen(argv[1], "r") );

    start_t = clock();

    //printf("ORIGINAL WORLD\n");
    //print_world();
    // process generations
    for( i = 0; i < num_generation; i++) {
     //   printf("========== GEN %d ===== \n", i);
        sub_generation(RED_GEN);

      //  printf("AFTER RED WORLD\n");
//        print_world();

        duplicate();
        sub_generation(BLK_GEN);

       // printf("\nAFTER BLACK WORLD\n");
//        print_world();

        update_generation();
    }

    printf("TOTAL SERIAL: %f\n", (double) (clock() - start_t) / CLOCKS_PER_SEC );
    
    // process output
    output();
    return 0;
}
