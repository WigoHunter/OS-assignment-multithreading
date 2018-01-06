#include "definitions.h"
#include "main.h"
#include <omp.h>

sem_t sem_worker;
sem_t sem_space;

sem_t sem_skeleton;
sem_t sem_engine;
sem_t sem_chassis;
sem_t sem_body;

sem_t sem_window;
sem_t sem_tire;
sem_t sem_battery;
sem_t sem_car;

int num_cars;
int num_spaces;
int num_workers;
int num_jobs;
// int round_needed;

struct Stack {
	int top;
	unsigned capacity;
	int* arr;
};

struct Stack* createStack(unsigned capacity) {
	struct Stack* stack = (struct Stack*) malloc(sizeof(struct Stack));
	stack->capacity = capacity;
	stack->top = -1;
	stack->arr = (int*) malloc(stack->capacity * sizeof(int));
	return stack;
}

int isFull(struct Stack* stack) {
	return stack->top == stack->capacity - 1;
}

int isEmpty(struct Stack* stack) {
	return stack->top == -1;
}

void push(struct Stack* stack, int num) {
	if (!isFull(stack))
		stack->arr[++stack->top] = num;
}

int pop(struct Stack* stack) {
	if(!isEmpty(stack))
		return stack->arr[stack->top--];
}

int main(int argc, char** argv)
{
	if (argc < 4) {
	printf("Usage: %s <number of cars> <number of spaces> <number of workers>\n", 
	argv[0]);
	return EXIT_SUCCESS;
	}
	num_cars     = atoi(argv[1]);
	num_spaces   = atoi(argv[2]);
	num_workers  = atoi(argv[3]);
	num_jobs = num_cars * 8;

	// Please print out your name and your university ID at the beginning of your program.
	printf("Hsu Kai-Chun, 2013545774\n");

	pthread_t threads[num_workers];
	printf("Job defined, %d workers will build %d cars with %d storage spaces\n",
			num_workers, num_cars, num_spaces);

	resource_pack *rpack = (struct resource_pack*) malloc(sizeof(struct resource_pack));

	// put semaphores into resource_pack
	initResourcePack(rpack, num_spaces, num_workers);

	// prepare work_pack
	work_pack wpack[num_jobs];

	int i, j;
	int cur_task = 0;
	int job;
	int cur_worker;
	int available_space = num_spaces;
	int available_body_to_make = num_cars;
	int available_car_to_make = num_cars;
	int space_required;
	int job_index = 0;
	int job_complete = 0;
	struct Stack* available_workers = createStack(num_workers);
	struct Stack* used_workers;

	for (i = 0; i < num_workers; i++) {
		push(available_workers, i);
	}

	// ** put in job 3 & job 7 later
	// First try to put in all other jobs that require spaces in an efficient order
	// and whenever spaces are not enough, insert job 3 or 7 in to consume car parts and release spaces
	for (i = 0; i < 6; i++) {
		if (i == 0) {
			job = 0;
			space_required = 1;
		}
		else if (i == 1) {
			job = 1;
			space_required = 1;
		}
		else if (i == 2) {
			job = 2;
			space_required = 1;
		}
		else if (i == 3) {
			job = 6;
			space_required = 1;
		}
		else if (i == 4) {
			job = 4;
			space_required = 7;
		}
		else if (i == 5) {
			job = 5;
			space_required = 4;
		}

		for (j = 0; j < num_cars; j++) {
			// avoid deadlock in scheduling
			while (available_space < space_required) {
				if(available_body_to_make > 0) {
					wpack[job_index].resource = rpack;
					wpack[job_index].jid = 3;
					wpack[job_index].times = 1;
					available_space += 2;
					available_body_to_make--;
					job_index++;
				}
				else if (available_car_to_make > 0) {
					wpack[job_index].resource = rpack;
					wpack[job_index].jid = 7;
					wpack[job_index].times = 1;
					available_space += 12;
					available_car_to_make--;
					job_index++;
				}
				else {
					printf("DEADLOCK will happen!");
				}
			}

			available_space -= space_required;
			wpack[job_index].resource = rpack;
			wpack[job_index].jid = job;
			wpack[job_index].times = job == WINDOW ? 7 : (job == TIRE ? 4 : 1);
			job_index++;
		}
	}

	// put in the remaining job 3
	for (i = 0; i < available_body_to_make; i++) {
		wpack[job_index].resource = rpack;
		wpack[job_index].jid = 3;
		wpack[job_index].times = 1;
		available_space += 2;
		job_index++;
	}

	// put in the remaining job 7
	for (i = 0; i < available_car_to_make; i++) {
		wpack[job_index].resource = rpack;
		wpack[job_index].jid = 7;
		wpack[job_index].times = 1;
		available_space += 12;
		job_index++;
	}

	// Start working and time the whole process
	double production_time = omp_get_wtime();

	// To handle cases of unused resources
	while(job_complete == 0) {
		used_workers = createStack(num_workers);

		// If there is available workers (num_workers), give it job to do
		while(!isEmpty(available_workers) && cur_task < num_jobs) {
			cur_worker = pop(available_workers);
			push(used_workers, cur_worker);
			wpack[cur_task].tid = cur_worker;
			pthread_create(&threads[cur_worker], NULL, work, &wpack[cur_task]);
			cur_task++;
		}

		// wait all that ran
		while(!isEmpty(used_workers)) {
			cur_worker = pop(used_workers);
			push(available_workers, cur_worker);
			pthread_join(threads[cur_worker], NULL);
		}
		
		// For testing: print the time expense of each round
		// printf("%f sec\n", omp_get_wtime() - production_time);
		
		if (cur_task >= num_jobs) {
			job_complete = 1;
		}
	}

	production_time = omp_get_wtime() - production_time;
	reportResults(production_time);

	destroySem();
	free(rpack);
	return EXIT_SUCCESS;
}

void reportResults(double production_time) {
	int *sem_value = malloc(sizeof(int));
	printf("=====Final report=====\n");

	sem_getvalue(&sem_skeleton, sem_value);
	printf("Unused Skeleton: %d\n",   *sem_value);
	sem_getvalue(&sem_engine,   sem_value);
	printf("Unused Engine: %d\n",     *sem_value);
	sem_getvalue(&sem_chassis,  sem_value);
	printf("Unused Chassis: %d\n",    *sem_value);
	sem_getvalue(&sem_body,     sem_value);
	printf("Unused Body: %d\n",       *sem_value);
	sem_getvalue(&sem_window,   sem_value);
	printf("Unused Window: %d\n",     *sem_value);
	sem_getvalue(&sem_tire,     sem_value);
	printf("Unused Tire: %d\n",       *sem_value);
	sem_getvalue(&sem_battery,  sem_value);
	printf("Unused Battery: %d\n",    *sem_value);

	sem_getvalue(&sem_space, sem_value);
	if (*sem_value < num_spaces) {
		printf("There are waste car parts!\n");
	}
	sem_getvalue(&sem_car, sem_value);
	printf("Production of %d %s done, production time: %f sec, space usage: %d\n", 
			*sem_value,
			*sem_value > 1 ? "cars" : "car",	       
			production_time, num_spaces);
	printf("==========\n");
	free(sem_value);
}

void initResourcePack(struct resource_pack *pack,
		int space_limit, int num_workers) {
	initSem();
	pack->space_limit  = space_limit;
	pack->num_workers  = num_workers;
	pack->sem_space    = &sem_space   ;
	pack->sem_worker   = &sem_worker  ;

	pack->sem_skeleton = &sem_skeleton;
	pack->sem_engine   = &sem_engine  ;
	pack->sem_chassis  = &sem_chassis ;
	pack->sem_body     = &sem_body    ;

	pack->sem_window   = &sem_window  ;
	pack->sem_tire     = &sem_tire    ;
	pack->sem_battery  = &sem_battery ;
	pack->sem_car      = &sem_car     ;
}

int destroySem(){
#if DEBUG
	printf("Destroying semaphores...\n");
#endif
	sem_destroy(&sem_worker);
	sem_destroy(&sem_space);

	sem_destroy(&sem_skeleton);
	sem_destroy(&sem_engine);
	sem_destroy(&sem_chassis);
	sem_destroy(&sem_body);

	sem_destroy(&sem_window);
	sem_destroy(&sem_tire);
	sem_destroy(&sem_battery);
	sem_destroy(&sem_car);
#if DEBUG
	printf("Semaphores destroyed\n");
#endif
	return 0;
}

int initSem(){
#if DEBUG
	printf("Initiating semaphores...\n");
#endif
	sem_init(&sem_worker,   0, num_workers);
	sem_init(&sem_space,    0, num_spaces);

	sem_init(&sem_skeleton, 0, 0);
	sem_init(&sem_engine,   0, 0);
	sem_init(&sem_chassis,  0, 0);
	sem_init(&sem_body,     0, 0);

	sem_init(&sem_window,   0, 0);
	sem_init(&sem_tire,     0, 0);
	sem_init(&sem_battery,  0, 0);
	sem_init(&sem_car,      0, 0);
#if DEBUG
	printf("Init semaphores done!\n");
#endif
	return 0;
}

