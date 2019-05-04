#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fcntl.h> 
#include<unistd.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<sys/wait.h>


#define M 10
#define N 10
#define NAMED_PIPE "fifo_pipe"
#define RESULT_FILE_NAME "result.bin"
#define DEBUG 0
 
typedef struct point {
	int x;
	int y;
} point;

typedef struct ship {
	point start;
	point end;
	int value;
} ship;

void print_board(int b[N][M]);
point new_point(int x, int y);
int place_ship(ship s, int (*board)[N][M]);
int remove_ship(ship s[], int num_of_ships, point attack, int (*board)[N][M], int result_fd);
ship generate_random_ship();
 
int  main() {
	pid_t pid = 0;
	int named_pipe_fd = 0;
	int result_file_fd = 0;
	
	(void) mkfifo(NAMED_PIPE, 0666);
	
	pid = fork();
	
	if(pid == 0) {
		int received[N][M] = {{0}};
		int ships_arr_size = 0;
		point attack;
		named_pipe_fd = open(NAMED_PIPE, O_RDONLY);
		
		if (0 <= named_pipe_fd) {
			
			if(read(named_pipe_fd, received, N*M*sizeof(int)) >= 0) {
				print_board(received);
				
				if(read(named_pipe_fd, &ships_arr_size, sizeof(int)) < 0) {
					printf("Child: Error reading from fifo");
					close(named_pipe_fd); 
					return -1;
				}
				
				ship  ships[ships_arr_size];
				
				if(read(named_pipe_fd, ships, ships_arr_size*sizeof(ship)) < 0) {
					printf("Child: Error reading from fifo");
					close(named_pipe_fd); 
					return -1;
				}
				
				result_file_fd = open(RESULT_FILE_NAME, O_CREAT|O_WRONLY| O_TRUNC, 0777);
				if (result_file_fd < 0) {
					printf("Child: Error creating results file\n");
				}
				
				
				attack.x = 0;
				attack.y = 0;

				int i = ships_arr_size;
				while(i > 0) {
					printf("Select point to attack (format:'M, N'):\n");
					scanf("%d, %d", &attack.x, &attack.y);
					
					if(remove_ship(ships, ships_arr_size, attack, &received, result_file_fd) < 0) {
						print_board(received);
						continue;
					}
				
					print_board(received);
					i--;
				}
				
				close(result_file_fd);
				printf("Finished!\n");
				
			} else {
				printf("Child: Error reading from fifo");
				return -1;
			}
			
			
			
		} else {
			printf("Child: Cannot open the pipe!\n");
		}
		
		close(named_pipe_fd); 	
		
	} else if(pid > 0) {
		
		int board[N][M] = {{0}};
		int number_of_ships = 0, i =0;

		printf("Please enter the number of ships:\n");
		scanf("%d", &number_of_ships);
		
		ship ships[number_of_ships];
		
		while(i < number_of_ships) {
			ship s = generate_random_ship();
			s.value = i + 1;
			
			if(place_ship(s, &board) == -1) {
				continue;
			}
			//printf("start: %d, %d\nend: %d, %d\n", s.start.x, s.start.y, s.end.x, s.end.y);
			
			ships[i] = s;
			i++;
		}
		
		named_pipe_fd = open(NAMED_PIPE, O_WRONLY);
		int bytes_written = write(named_pipe_fd, board, N*M*sizeof(int));
		if(bytes_written < 0) {
			printf("Parent: Error writing to fifo");
			return -1;
		}
		
		bytes_written = write(named_pipe_fd, &number_of_ships, sizeof(int));
		if(bytes_written < 0) {
			printf("Parent: Error writing to fifo");
			return -1;
		}
		
		bytes_written = write(named_pipe_fd, ships, number_of_ships*sizeof(ship));
		if(bytes_written < 0) {
			printf("Parent: Error writing to fifo");
			return -1;
		}
		
		close(named_pipe_fd);
		wait(NULL);
		
	} else {
		printf("Error in creating process");
	}

	return 0;
}

void print_board(int b[N][M]) {
	int i, j, num_of_digits;

	printf("\nN |M ");
	for(j = 0; j < M; j++) {
		printf("%d ", j);
	}

	printf("\n  |\n");	
	
	for(i = 0; i < N; i++) {
		printf("%d |  ", i);
		for(j = 0; j < M; j++) {
			if(DEBUG) {
				printf("%d ", b[i][j]);
			} else {
				if(b[i][j] < 0) {
					printf("x ");
				} else {
					printf("* ");
				}
			}
			
			num_of_digits = j/10;
			while(num_of_digits != 0) {
				printf(" ");
				num_of_digits /= 10;
			}
			
		}
		printf("\n");
	}
}

point new_point(int x, int y) {
	point p;
	p.x = x;
	p.y = y;
	
	return p;
}


int place_ship(ship s, int (*board)[N][M]) { 
	int i;
	
	if(s.start.x >= s.end.x) {
		//printf("Cannot place ship. Start point must be < end point!\n");
		return -1;
	}
	
	if((s.end.x - s.start.x) <  2) {
		//printf("Cannot place ship. The ship must be at least 2 blocks!\n");
		return -1;
	}
	
	if(s.start.y != s.end.y) {
		//printf("Cannot place ship. The points must be on the same level!\n");
		return -1;
	}
	
	if(s.start.x >= M || s.start.y >= N || s.end.x >= M || s.end.y >= N) {
		//printf("Cannot place ship. x <= %d and y <= %d!\n", M, N);
		return -1;
	}
	
	//check if there is another ship
	//replace 's.start.x - 1' with 's.start.x' and 's.end.x + 1' with 's.end.x' for opportunity to place ships one next to another
	for(i = s.start.x - 1; i <= s.end.x + 1; i++) {
		if(i < 0) {
			continue;
		}

		if((*board)[s.start.y][i] != 0) {
			//printf("Cannot place ship!\n");
			return -1;
		}
	}
	
	//placing ship
	for(i = s.start.x; i <= s.end.x; i++) {
		(*board)[s.start.y][i] = s.value;
	}
	
	return 0;
}

int remove_ship(ship s[], int num_of_ships, point attack, int (*board)[N][M], int result_fd) {
	int i = 0, j = 0;
	int value_to_remove = 0;
	int ship_to_remove;
	char result_msg[150];
	
	while(i < num_of_ships) {
		if((*board)[attack.y][attack.x] == s[i].value) {
			value_to_remove = s[i].value;
			ship_to_remove = i;
			break;
		}
		
		i++;
	}
	
	if(value_to_remove == 0) {
		printf("Miss! There is no ship.\n");
		
		sprintf(result_msg, "Attack point: %d, %d\nMiss! There is no ship.\n", attack.x, attack.y);
		int bytes_written = write(result_fd, result_msg, 100*sizeof(char));
		
		if(bytes_written < 0) {
			printf("Error writing result to file");
		}
		
		return -1;
	}
	
	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			if((*board)[i][j] == value_to_remove) {
				(*board)[i][j] = -1;
			}
		}
	}
	
	printf("Ship removed:\nstart: %d, %d\nend: %d, %d\n", s[ship_to_remove].start.x, s[ship_to_remove].start.y, s[ship_to_remove].end.x, s[ship_to_remove].end.y);
	
	sprintf(result_msg, "Attack point: %d, %d\nShip removed:\nstart: %d, %d\nend: %d, %d\n", attack.x, attack.y, s[ship_to_remove].start.x, s[ship_to_remove].start.y, s[ship_to_remove].end.x, s[ship_to_remove].end.y);
	int bytes_written = write(result_fd, result_msg, 100*sizeof(char));
	
	if(bytes_written < 0) {
		printf("Error writing result to file");
	}
	
	return 0;
}

ship generate_random_ship() {
	struct timeval time; 
	gettimeofday(&time,NULL);
	srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
		
	int ship_size =  (rand() % 3) + 2; //(rand() % (upper - lower)) + lower //upper = 5, lower =  2
	int p_x = rand() % (M+1);
	int p_y = rand() % (N+1);
		
	point start = new_point(p_x, p_y);
	point end = new_point(p_x + ship_size, p_y);
	
	ship random_ship;
	random_ship.start = start;
	random_ship.end = end;
	
	return random_ship;
}
