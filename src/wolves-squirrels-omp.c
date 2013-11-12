#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <time.h>

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
    int updated;
} world;


omp_lock_t** world_lock;
omp_lock_t* world_lock_array;


// GLOBAL variable!!!
world* world_array;
world** world_indexer;
world* world_array_r;
world** world_indexer_r;
world* old_array;


int world_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;
int current_generation;

void print_final_matrix(int world_size) {
    int i, j;

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

world* new_array() {
    return (world*)calloc( world_size*world_size , sizeof(world));   
}

world** new_indexer() {
    return (world**)calloc( world_size, sizeof(world*));
}

void new_matrix() {
    int i;
    world* n_array = new_array();

    for (i = 0; i < world_size; ++i) {
        world_indexer_r[i] = &world_array[i*world_size];
        world_indexer[i] = &n_array[i*world_size];
    }

    old_array = world_array_r;

    world_array_r = world_array;

    world_array = n_array;

    free(old_array);
}

/** Allocates space for the world. */
void initialization(int world_size) {
    int i, j;

    world_array = new_array();
    world_indexer = new_indexer();
    world_array_r = new_array();
    world_indexer_r = new_indexer();

    world_lock_array = (omp_lock_t*) malloc (sizeof(omp_lock_t) * world_size * world_size);
    world_lock = (omp_lock_t**) malloc(sizeof(omp_lock_t*) * world_size);

    #pragma omp parallel for private(j)
    for (i = 0; i < world_size; ++i) {
        world_indexer[i] = &world_array[i*world_size];
        world_indexer_r[i] = &world_array_r[i*world_size];
        world_lock[i] = &world_lock_array[i*world_size];

        for ( j = 0; j < world_size; ++j ) {
            omp_init_lock( &world_lock[i][j] );
        }
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
        world_indexer[world_x][world_y].updated = 0;
    }
    fclose(fp);
}

int is_free_position(int x, int y, int type) {
    if( type == WOLF) {
        return  world_indexer_r[x][y].type == EMPTY || world_indexer_r[x][y].type == SQUIRREL;
    } else {
        return  world_indexer_r[x][y].type == EMPTY || world_indexer_r[x][y].type == TREE;
    }
}

int can_move_up(int x, int y, int type) {
    return x - 1 >= 0 && is_free_position(x-1, y, type) && type != world_indexer_r[x-1][y].type;
}

int can_move_right(int x, int y, int type) {
    return y + 1 < world_size && is_free_position(x, y+1, type) && type != world_indexer_r[x][y+1].type;
}

int can_move_down(int x, int y, int type) {
    return x + 1 < world_size && is_free_position(x+1, y, type) && type != world_indexer_r[x+1][y].type;
}

int can_move_left(int x, int y, int type) {
    return y - 1 >= 0 && is_free_position(x, y-1, type) && type != world_indexer_r[x][y-1].type;
}

int* find_free_positions(int x, int y, int type) {
    int *array = (int*) calloc(4, sizeof(int));

    array[UP] = can_move_up(x,y, type);
    array[RIGHT] = can_move_right(x, y, type);
    array[DOWN] = can_move_down(x, y, type);
    array[LEFT] = can_move_left(x, y, type);

    return array;
}

int* find_squirrels(int x, int y) {
    int *array = (int*) calloc(4, sizeof(int));

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

int calculate_direction(int *positions, int position){
    int direction;
    for ( direction = 0; direction < 4 ; direction++){
        if( positions[direction] ) { 
            position--;
        }
        if ( position == -1 ) {
            return direction;
        }
    }
    return -1;
}
int find_next_positon(int x, int y, int type) {
    int * free_positions = find_free_positions( x, y, type);
    int * free_squirrels = find_squirrels(x, y);
    int c = x * world_size + y, position = -1;

    int free_counter = count_free_positions(free_positions);
    int squirrel_counter = count_free_positions(free_squirrels);
    int direction;

    if( type == WOLF &&  squirrel_counter > 0) {
        position = c % squirrel_counter;
        direction = calculate_direction(free_squirrels, position);
        free(free_squirrels);
        free(free_positions);
        return direction;
    } else if ( free_counter > 0 ) {
        position = c % free_counter;
        direction = calculate_direction(free_positions, position);
        free(free_positions);
        free(free_squirrels);
        return direction;
    } else {
        return -1;
    }
}

int can_update(int x, int y) {
    return !world_indexer_r[x][y].updated;
}

void resolve_conflicts(int to_x, int to_y, int to_type, int to_breeding, int to_starvation, int updated) {
    omp_set_lock( &world_lock[to_x][to_y] );

    int result_type = world_indexer[to_x][to_y].type;

    if( result_type == EMPTY ) {
        world_indexer[to_x][to_y].type = to_type;
        world_indexer[to_x][to_y].breeding_period = to_breeding;
        world_indexer[to_x][to_y].starvation_period = to_starvation;
    }
    else if( to_type == SQUIRREL && result_type == WOLF) {
        world_indexer[to_x][to_y].starvation_period = wolf_starvation;
    }
    else if( to_type == WOLF && result_type == SQUIRREL) {
        world_indexer[to_x][to_y].type = to_type;
        world_indexer[to_x][to_y].breeding_period = to_breeding;
        world_indexer[to_x][to_y].starvation_period = wolf_starvation;
    }
    else if( (to_type == SQUIRREL && result_type == SQUIRREL) || (to_type == SQUIRREL_TREE && result_type == SQUIRREL_TREE) ) {
        world_indexer[to_x][to_y].breeding_period = world_indexer[to_x][to_y].breeding_period <= to_breeding ?
        to_breeding : world_indexer[to_x][to_y].breeding_period;
    }
    else if( to_type == WOLF && result_type == WOLF) {

        if(world_indexer[to_x][to_y].starvation_period < to_starvation) {
            world_indexer[to_x][to_y].starvation_period = to_starvation;
            world_indexer[to_x][to_y].breeding_period = to_breeding;
        }
        else if(world_indexer[to_x][to_y].starvation_period == to_starvation) {
            if(world_indexer[to_x][to_y].breeding_period < to_breeding) {
                world_indexer[to_x][to_y].breeding_period = to_breeding;
            }
        }
    } else if(result_type == TREE) {
        world_indexer[to_x][to_y].type = to_type;
        world_indexer[to_x][to_y].breeding_period = to_breeding;
    }

    world_indexer[to_x][to_y].updated = 1;
    omp_unset_lock( &world_lock[to_x][to_y] );

}

void update_position(int from_x, int from_y, int from_type, 
    int to_x, int to_y, int to_type, int from_breeding, int from_starvation, int to_breeding, int to_starvation) {


    if( can_update(from_x , from_y) ) {
        resolve_conflicts(to_x, to_y, to_type, to_breeding, to_starvation, 1);
    } else {
        resolve_conflicts(to_x, to_y, to_type, from_breeding, from_starvation, 0);
    }

    world_indexer[from_x][from_y].type = from_type;
    world_indexer[from_x][from_y].breeding_period = 0;
    world_indexer[from_x][from_y].starvation_period = 0;
    world_indexer[from_x][from_y].updated = 0;
}


void move_squirrel(int from_x, int from_y, int to_x, int to_y, int breeding) {
    int to_type = world_indexer_r[to_x][to_y].type;

    int to_breeding = world_indexer_r[to_x][to_y].breeding_period;

    if( to_type == TREE) {
        update_position(from_x, from_y, EMPTY, to_x, to_y, SQUIRREL_TREE, breeding , 0, breeding - 1, 0);
    }else {
        update_position(from_x, from_y, EMPTY, to_x, to_y, SQUIRREL, breeding, 0, breeding - 1, 0);
    }
}

void move_wolves(int from_x, int from_y, int to_x, int to_y, int breeding, int starvation) {

    int to_type = world_indexer_r[to_x][to_y].type;
    int to_breeding = world_indexer_r[to_x][to_y].breeding_period;
    int to_starvation = world_indexer_r[to_x][to_y].starvation_period;

    if ( to_type == SQUIRREL ) {
        update_position( from_x, from_y, EMPTY, to_x, to_y, WOLF, breeding, starvation, breeding - 1, wolf_starvation);
    }else {
        update_position( from_x, from_y, EMPTY, to_x, to_y, WOLF, breeding, starvation, breeding - 1, starvation - 1);
    }
}

void move_squirrel_trees(int from_x, int from_y, int to_x, int to_y, int breeding) {
    int to_type = world_indexer_r[to_x][to_y].type;

    int to_breeding = world_indexer_r[to_x][to_y].breeding_period;

    if( to_type == TREE) {
        update_position( from_x, from_y, TREE, to_x, to_y, SQUIRREL_TREE, breeding, 0, breeding - 1, 0);
    }else {
        update_position( from_x, from_y, TREE, to_x, to_y, SQUIRREL, breeding, 0, breeding - 1, 0);
    }
}

void go_up(int x, int y, int type, int breeding, int starvation) {
    if( type == SQUIRREL ) { 
        move_squirrel(x, y, x - 1, y, breeding);
    } else if ( type == WOLF ) {
        move_wolves(x, y, x -1,  y, breeding, starvation);
    } else if ( type == SQUIRREL_TREE ) {
        move_squirrel_trees(x, y, x - 1, y, breeding);
    }
}

void go_right(int x, int y, int type, int breeding, int starvation) {
    if( type == SQUIRREL ) {
        move_squirrel(x, y, x, y + 1, breeding);
    } else if ( type == WOLF ) {
        move_wolves(x, y, x,  y + 1, breeding, starvation);
    } else if ( type == SQUIRREL_TREE ) {
        move_squirrel_trees(x, y, x , y + 1, breeding);
    }
}

void go_down(int x, int y, int type, int breeding, int starvation) {
    if( type == SQUIRREL ) {
        move_squirrel(x, y, x + 1, y, breeding);
    } else if ( type == WOLF ) {
        move_wolves(x, y, x + 1,  y, breeding, starvation);
    } else if ( type == SQUIRREL_TREE ) {
        move_squirrel_trees(x, y, x + 1, y, breeding);
    }
}

void go_left(int x, int y, int type, int breeding, int starvation) {
    if( type == SQUIRREL ) {
        move_squirrel(x, y, x, y - 1, breeding);
    } else if ( type == WOLF ) {
        move_wolves(x, y, x ,  y - 1, breeding, starvation);
    } else if ( type == SQUIRREL_TREE ) {
        move_squirrel_trees(x, y, x , y - 1, breeding);
    }
}

void no_move(int x, int y, int type, int breeding, int starvation) {
    world_indexer[x][y].type = type;
    if ( can_update(x, y) ) {
        world_indexer[x][y].updated = 1;
        if(world_indexer_r[x][y].starvation_period > 0) {
            world_indexer[x][y].starvation_period = starvation -1;
        }
        if(world_indexer_r[x][y].breeding_period > 0) {
            world_indexer[x][y].breeding_period = breeding - 1;
        }
    }
}

void move(int x, int y, int type) {
    int next_position = find_next_positon(x, y, type);
    int breeding = world_indexer_r[x][y].breeding_period;
    int starvation = world_indexer_r[x][y].starvation_period;

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
        no_move( x, y, type, breeding, starvation);
        return;
    }
}

void baby(int x, int y, int type){

    if( type == WOLF ) {
        world_indexer[x][y].type = type;
        world_indexer[x][y].breeding_period = wolf_breeding;
        world_indexer[x][y].starvation_period = wolf_starvation;
        world_indexer[x][y].updated = 1;
    }
    else if (type == SQUIRREL || type == SQUIRREL_TREE) {
        world_indexer[x][y].type = type;
        world_indexer[x][y].breeding_period = squirrel_breeding;
        world_indexer[x][y].starvation_period = 0;
        world_indexer[x][y].updated = 1;
    }
}

void breed(int x, int y, int type) {
    int next_position = find_next_positon(x, y, type);
    int starvation = world_indexer_r[x][y].starvation_period;
    int breeding;

    if(type == SQUIRREL || type == SQUIRREL_TREE) {
        breeding = squirrel_breeding;
    } else {
        breeding = wolf_breeding;
    }

    switch(next_position) {
        case UP:
        go_up(x, y, type, breeding, starvation);
        baby(x, y, type);
        break;
        case RIGHT:
        go_right( x, y, type, breeding, starvation);
        baby(x, y, type);
        break;
        case DOWN:
        go_down( x, y, type, breeding, starvation);
        baby(x, y, type);
        break;
        case LEFT:
        go_left( x, y, type, breeding, starvation);
        baby(x, y, type);
        break;
        default:
        no_move( x, y, type, breeding, starvation);
        return;
    }
}

void exodus(int x, int y){
    int type =  world_indexer_r[x][y].type;
    int breeding = world_indexer_r[x][y].breeding_period;
    int starvation = world_indexer_r[x][y].starvation_period;

    if( type == SQUIRREL || type == WOLF || type == SQUIRREL_TREE ) {

        if( starvation == 0 && type == WOLF  && can_update(x,y) ) {
            world_indexer[x][y].type = EMPTY;
            world_indexer[x][y].breeding_period = 0;
            world_indexer[x][y].starvation_period = 0;
            world_indexer[x][y].updated = 0;
            return;
        }
        if( breeding == 0  && can_update(x, y) ) {
            breed( x, y, type);
        }
        else {
            move(x, y, type);
        }
    } else if(type != EMPTY) {
        world_indexer[x][y].type = type;
    }
}

// Entity does not move, gets copied
void move_entity(int x, int y) {
    world_indexer[x][y].type = world_indexer_r[x][y].type;
    world_indexer[x][y].breeding_period = world_indexer_r[x][y].breeding_period;
    world_indexer[x][y].starvation_period = world_indexer_r[x][y].starvation_period;
    world_indexer[x][y].updated = world_indexer_r[x][y].updated;
}

void sub_generation(int black_generation){
    int i, j, k;

    #pragma omp parallel for private(j,k) if(world_size > 15)
    for(i = 0; i < world_size; i++) {
        if( black_generation ) {
            j = ( i % 2 == 0) ? 1 : 0;
        } else {
            j = ( i % 2 == 0) ? 0 : 1;
        }

        for( k = 0; k < world_size; k++) {
            if( k == j) {
                exodus(i, j);
                j = j + 2;
            } else {
                if(world_indexer_r[i][k].type != EMPTY && world_indexer[i][k].updated == 0) {
                    move_entity(i, k);
                }
            }
        }
    }
}

// CANCRO
void cancer() {
    int i, k;

    #pragma omp parallel for private(k)
    for(i = 0; i < world_size; i++) {
        for( k = 0; k < world_size; k++) {
            world_indexer[i][k].updated = 0;
        }
    }
}


int main(int argc, char *argv[]) {
    int i;


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

    double start, end; start = omp_get_wtime();

    // process generations
    for( i = 0; i < num_generation; i++) {
        // 1st subgeneration
        new_matrix();
        sub_generation(FALSE);

        // 2nd subgeneration
        new_matrix();
        sub_generation(TRUE);

        cancer();
    }

    printf("Final:\n"); fflush(stdout);
    print_final_matrix(world_size); fflush(stdout);

    end = omp_get_wtime();
    printf("TOTAL PARALLEL: %f\n", end - start);

    return 0;
}
