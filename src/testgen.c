#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char * argv[]){

    int world_size = atoi( argv[1] );
    int array[] = { 0, 1, 2, 3, 4, 0, 0, 0, 0};

    printf("%d\n", world_size);

    int i, j;
    srand((unsigned) time(NULL));

    for( i = 0 ; i < world_size; i++){
        for( j = 0; j < world_size; j++) {
            char c;
            int pos = array[ rand() % 9];

            if( pos == 0 ){
                continue;
            }

            if ( pos == 1 ) {
                c = 'w';
            } else if (pos == 2) {
                c = 's';
            } else if ( pos == 3) {
                c = 't';
            } else if ( pos == 4) {
                c = 'i';
            }

            printf("%d %d %c\n", i, j, c);
        }
    }
}


