#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	int entities = atoi(argv[1]);
	int world_size = atoi(argv[2]);
	int i = 0;
	printf("%d\n", world_size);

	srand(time(NULL));

	while(i < entities) {
		int animal = rand()%6;
		if(animal != 0) {
			printf("%d %d ", rand()%world_size, rand()%world_size);

			switch(animal) {
				case 1:
				printf("%c\n", 's');
				break;
				case 2:
				printf("%c\n", 'w');
				break;
				case 3:
				printf("%c\n", 't');
				break;
				case 4:
				printf("%c\n", 'i');
				break;
				case 5:
				printf("%c\n", '$');
				break;
				default:
				break;
			}
			i++;
		}
	}

	return 0;
}