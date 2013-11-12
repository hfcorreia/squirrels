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
    int is_breading;
    cell* cell;
} position;

typedef struct move {
    position* actual_position;
    position* next_position;
    struct move* next;
} move;


typedef struct queue {
    struct move* head;
    struct move* tail;
} queue;

cell* world_array;
cell** world_indexer;

int world_size;
int wolf_breeding;
int squirrel_breeding;
int wolf_starvation;

/* Will always return the pointer to queue */
queue* add_move(queue* s, position* actual_position, position* next_position) {
    move* p = (move*) calloc( 1 , sizeof(*p) );

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
move* remove_move( queue* s )
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
    s->head = p;
    if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */

    return h;
}

queue* create_queue(void) {
    queue* p = (queue*) calloc( 1 , sizeof(queue));

    if( NULL == p ) {
        fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
    }

    p->head = p->tail = NULL;

    return p;
}

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
}

/** Allocates space for the world. */
void initialization() {
    int i;

    world_array = (cell*) calloc( world_size*world_size , sizeof(cell));   
    world_indexer = (cell**) calloc( world_size, sizeof(cell*));

    for (i = 0; i < world_size; ++i) {
        world_indexer[i] = &world_array[i*world_size];
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
                break;
            case 'w':
                type_num = WOLF;
                world_indexer[x][y].breeding = wolf_breeding;
                world_indexer[x][y].starvation =  wolf_starvation;
                break;
            case 't':
                type_num = TREE;
                world_indexer[x][y].breeding = 0;
                world_indexer[x][y].starvation = 0;
                break;
            case 'i':
                type_num = ICE;
                world_indexer[x][y].breeding = 0;
                world_indexer[x][y].starvation = 0;
                break;
            default:
                type_num = EMPTY;
                world_indexer[x][y].breeding = 0;
                world_indexer[x][y].starvation = 0;
                break;
        }
        world_indexer[x][y].type = type_num;
    }
}

int is_free_position(int x, int y, int type) {
    if( type == WOLF) {
        return ( world_indexer[x][y].type == EMPTY || world_indexer[x][y].type == SQUIRREL) && world_indexer[x][y].type != type;
    } else {
        return ( world_indexer[x][y].type == EMPTY || world_indexer[x][y].type == TREE) && world_indexer[x][y].type != type;
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

int* find_free_positions(position* actual_position) {
    int *array = (int*) calloc(4, sizeof(int));
    int x = actual_position->x, y = actual_position->y, type = actual_position->cell->type;

    array[UP] = can_move_up(x,y, type);
    array[RIGHT] = can_move_right(x, y, type);
    array[DOWN] = can_move_down(x, y, type);
    array[LEFT] = can_move_left(x, y, type);

    return array;
}

int* find_squirrels(position *actual_position) {
    int *array = (int*) calloc(4, sizeof(int));
    int x = actual_position->x, y = actual_position->y;

    array[UP] = x - 1 >= 0 && world_indexer[x-1][y].type == SQUIRREL;
    array[RIGHT] = y + 1 < world_size && world_indexer[x][y+1].type == SQUIRREL;
    array[DOWN] = x + 1 < world_size && world_indexer[x+1][y].type == SQUIRREL;
    array[LEFT] = y - 1 >= 0 && world_indexer[x][y-1].type == SQUIRREL;

    return array;
}

int count_free_positions(int* free_positions) {
    int i, counter = 0;

    for ( i = 0; i < 4 ; i++){
        counter += free_positions[i] ? 1 : 0; 
    }

    return counter;
}

int calculate_direction(int *possible_positions, int next_position){
    int direction;

    for ( direction = 0; direction < 4 ; direction++){
        if( possible_positions[direction] ) { 
            next_position--;
        }
        if ( next_position == -1 ) {
            return direction;
        }
    }

    return -1;
}

int find_next_positon(position *actual_position) {
    int* free_positions = find_free_positions(actual_position);
    int* free_squirrels = find_squirrels(actual_position);
    int c = actual_position->x * world_size + actual_position->y;

    int free_counter = count_free_positions(free_positions);
    int squirrel_counter = count_free_positions(free_squirrels);
    int direction = -1;

    if( actual_position->cell->type == WOLF &&  squirrel_counter > 0) {
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

position* go_up(position *actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x - 1;
    next_position->y = actual_position->y;

    next_cell->type = world_indexer[next_position->x][next_position->y].type;
    next_cell->starvation = world_indexer[next_position->x][next_position->y].starvation;
    next_cell->breeding = world_indexer[next_position->x][next_position->y].breeding;

    next_position->cell = next_cell;

    return next_position;
}

position* go_right(position *actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x;
    next_position->y = actual_position->y + 1;

    next_cell->type = world_indexer[next_position->x][next_position->y].type;
    next_cell->starvation = world_indexer[next_position->x][next_position->y].starvation;
    next_cell->breeding = world_indexer[next_position->x][next_position->y].breeding;

    next_position->cell = next_cell;

    return next_position;
}

position* go_down(position *actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x + 1;
    next_position->y = actual_position->y;

    next_cell->type = world_indexer[next_position->x][next_position->y].type;
    next_cell->starvation = world_indexer[next_position->x][next_position->y].starvation;
    next_cell->breeding = world_indexer[next_position->x][next_position->y].breeding;

    next_position->cell = next_cell;

    return next_position;
}

position* go_left(position *actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x;
    next_position->y = actual_position->y - 1;

    next_cell->type = world_indexer[next_position->x][next_position->y].type;
    next_cell->starvation = world_indexer[next_position->x][next_position->y].starvation;
    next_cell->breeding = world_indexer[next_position->x][next_position->y].breeding;

    next_position->cell = next_cell;

    return next_position;
}

// note must verify if is going to the same cell
position* no_move(position *actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x;
    next_position->y = actual_position->y;

    next_cell->type = world_indexer[next_position->x][next_position->y].type;
    next_cell->starvation = world_indexer[next_position->x][next_position->y].starvation;
    next_cell->breeding = world_indexer[next_position->x][next_position->y].breeding;

    next_position->cell = next_cell;

    return next_position;
}

position* die(position* actual_position) {
    position* next_position = (position*) calloc( 1, sizeof(position) );
    cell* next_cell = (cell*) calloc( 1, sizeof(cell));

    next_position->x = actual_position->x;
    next_position->y = actual_position->y;
    
    next_cell->type = EMPTY;
    next_cell->starvation = 0;
    next_cell->breeding = 0;

    next_position->cell = next_cell;

    return next_position;
}

position* go(position *actual_position) {
    int next_position = find_next_positon(actual_position);

    switch(next_position) {
        case UP:
            return go_up( actual_position );
            break;
        case RIGHT:
            return go_right( actual_position );
            break;
        case DOWN:
            return go_down( actual_position );
            break;
        case LEFT:
            return go_left( actual_position );
            break;
        default:
            return no_move( actual_position );
    }

}

queue* breed(position *actual_position, queue* movements) {
    int next_direction = find_next_positon(actual_position);
    position* next_position;

    switch(next_direction) {
        case UP:
            next_position = go_up( actual_position );
            break;
        case RIGHT:
            next_position = go_right( actual_position );
            break;
        case DOWN:
            next_position = go_down( actual_position );
            break;
        case LEFT:
            next_position = go_left( actual_position );
            break;
        default:
            //can not move 
            actual_position->is_breading = 1;
            return add_move(movements, actual_position, actual_position);
    }

    // move the original squirrel to next place
    actual_position->is_breading = 1;
    return add_move(movements, actual_position, next_position);
}

queue* move_squirrel(position *actual_position, queue* movements) {
    if( actual_position->cell->breeding == 0 ) {
        movements = breed(actual_position, movements);
    } else {
        position* next_position = go(actual_position);
        movements = add_move( movements, actual_position, next_position);
    }
    return movements;
}

queue* move_squirrel_tree(position *actual_position, queue* movements) {
    if( actual_position->cell->breeding == 0 ) {
        movements = breed(actual_position, movements);
    } else {
        position* next_position = go(actual_position);
        movements = add_move( movements, actual_position, next_position);
    }
    return movements;
}

queue* move_wolf(position *actual_position, queue* movements) {
    if( actual_position->cell->starvation == 0 ) {
        movements = add_move(movements, actual_position, die(actual_position));
    } else if( actual_position->cell->breeding == 0 ) {
        movements = breed(actual_position, movements);
    } else {
        position* next_position = go(actual_position);
        movements = add_move( movements, actual_position, next_position);
    }
    return movements;
}

queue* exodus(int x, int y, queue* movements){
    position* actual_position = (position*) calloc( 1, sizeof(position) );
    actual_position->x = x;
    actual_position->y = y;
    actual_position->cell = &world_indexer[x][y];

    int type = actual_position->cell->type;
    move* movement;
    switch(type) {
        case SQUIRREL:
            movements = move_squirrel(actual_position, movements);
            break;
        case SQUIRREL_TREE:
            movements = move_squirrel_tree(actual_position, movements);
            break;
        case WOLF:
            movements = move_wolf(actual_position, movements);
            break;
    }
    
    return movements;
}

queue* sub_generation(int is_black_gen ){
    int i, j;
    queue* movements = create_queue();

    for(i = 0; i < world_size; i++) {
        if( is_black_gen ) {
            j = ( i % 2 == 0) ? 1 : 0;
        } else {
            j = ( i % 2 == 0) ? 0 : 1;
        }

        for( j = 0; j < world_size; j = j + 2) {
            exodus(i, j, movements);
        }
    }
}

int is_no_movement(position* actual_position, position* next_position) {
    return actual_position->x == next_position->x && actual_position->y == next_position->y;
}

position* resolve_conflict( position* next_position ) {

    return next_position;
}

void breeding_move(position* actual_position, position* next_position) {
    if ( is_no_movement(actual_position, next_position) ) {
        if ( actual_position->cell->type == WOLF ) {
            world_indexer[actual_position->x][actual_position->y].breeding = wolf_breeding;
            // starvation?
        } else {
            world_indexer[actual_position->x][actual_position->y].breeding = squirrel_breeding;
        }
    } else {
        if ( actual_position->cell->type == WOLF ) {
            world_indexer[actual_position->x][actual_position->y].breeding = wolf_breeding;
            // starvation?
            next_position->cell->breeding = wolf_breeding;
        } else {
            world_indexer[actual_position->x][actual_position->y].breeding = squirrel_breeding;
            next_position->cell->breeding = squirrel_breeding;
        }
        resolve_conflict( next_position );
    }
}

void normal_move(position* actual_position, position* next_position) {
    if( actual_type == WOLF && next_type == EMPTY ) {

    }
    if( actual_type == WOLF && next_type == WOLF  ) {} // conflict
    if( actual_type == WOLF && next_type == SQUIRREL ) {}

    if( actual_type == SQUIRREL && next_type == EMPTY ) {}
    if( actual_type == SQUIRREL && next_type == WOLF  ) {} // conflict
    if( actual_type == SQUIRREL && next_type == SQUIRREL ) {} // conflict
    if( actual_type == SQUIRREL && next_type == SQUIRREL_TREE ) {} // conflict
    if( actual_type == SQUIRREL && next_type == TREE ) {}


    if( actual_type == SQUIRREL_TREE && next_type == EMPTY ) {}
    if( actual_type == SQUIRREL_TREE && next_type == WOLF  ) {} // conflict
    if( actual_type == SQUIRREL_TREE && next_type == SQUIRREL ) {} // conflict
    if( actual_type == SQUIRREL_TREE && next_type == SQUIRREL_TREE ) {} //conflict
    if( actual_type == SQUIRREL_TREE && next_type == TREE ) {}
}

void move_element(position* actual_position, position* next_position) {
    int actual_type = actual_position->cell->type;
    int next_type = new_move->cell->type;

    if( actual_type->is_breading ) {
        breeding_move(actual_position, next_position);
    } else if ( ! is_no_movement(actual_position, next_position) ) {
        normal_move(actual_position, next_position);
    } else {
        world_indexer[x][y].type = next_position->cell->type;
        world_indexer[x][y].starvation = next_position->cell->starvation;
        world_indexer[x][y].breeding = next_position->cell->breeding;
    }
}

void process_moves(queue* movements) {
    while( movements->head != NULL ) {
        move* new_move = remove_move(movements);
        move_element( new_move->actual_position, new_move->next_position);
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
            } else if( type == SQUIRREL || type = SQUIRREL_TREE ) {
                if( world_indexer[i][j].breeding > 0 )  world_indexer[i][j].breeding--;
            }
        }
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

    // process generations
    for( i = 0; i < num_generation; i++) {
        queue* red_movements = sub_generation(RED_GEN);
        process_moves(red_movements);
        queue* black_movements = sub_generation(BLK_GEN);

        update_generation();
    }

    printf("TOTAL SERIAL: %f\n", (double) (clock() - start_t) / CLOCKS_PER_SEC );
    
    // process output
    output();
    return 0;
}

