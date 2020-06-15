#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//number of philosophers 
#define N		5 
//relative left and right of philosopher 
#define LEFT		( i + N - 1 ) % N
#define RIGHT		( i + 1 ) % N

//states of the philosophers
#define THINKING	0 
#define HUNGRY		1 
#define EATING		2

#define EATING_TIME	1
#define THINKING_TIME	0

//define alias for pthread_mutex_unlock and pthread_mutex_lock
#define	up		pthread_mutex_unlock
#define	down		pthread_mutex_lock


pthread_mutex_t m; 		// initialized to 1 - declare a mutex
pthread_mutex_t s[N];		// initialized to 0's
int state[N];			// initiated to THINKING's
pthread_t philos[N];


// functions
void grab_forks(int i);
void put_away_forks(int i);
void test(int i);
void* philosopher_cycle(void* num);

//------------------------------------------------------------------------------------------------------


int main(int argc, char** argv)
{
    	int i;
	pthread_mutex_init(&m, NULL); //initialises the mutex m to 0

	int* num = (int*) malloc(N*sizeof(int)); //to allocate a block of memory dynamically

	for(i = 0; i < N; i++){

		state[i]=THINKING;
		pthread_mutex_init(&s[i], NULL); //initialises the mutex array
		down(&s[i]); //initialises array s to 0

		num[i] = i;
		//creation of threads - on success, pthread_create() returns 0
		if(pthread_create(&philos[i], NULL, philosopher_cycle, (void*)(num+i)))
        	{
            		printf("failed to create thread");
            		return 1;
        	}
	}

	for(i = 0; i < N; i++){
		//waits for the thread specified by thread to terminate
		pthread_join(philos[i],NULL);
	}
	//destroy the mutex(lock) object referenced by mutex; the mutex object becomes, in effect, uninitialized
	pthread_mutex_destroy(&m);
	for(i = 0; i < N; i++){
		pthread_mutex_destroy(&s[i]);
	}
	//therminate calling threads
	pthread_exit(NULL);
	return 0;
}

//------------------------------------------------------------------------------------------------------
//philosophers grab forks function
void grab_forks( int i ) {
	//lock the mutex the philosopher is going to eat and the others have to wait because they have been locked
	down(&m);
		printf("Philosopher %d is hungry.\n", i);
		// state that hungry
		state[i]=HUNGRY;
		// eat if neighbours are not eating
		test(i);
	up(&m); //unlock mutex 
	down(&s[i]); //lock fork for philosopher to eat
}

//philosophers put forks on the table
void put_away_forks( int i ) {
	down(&m); //lock the mutex
		// state that thinking
		state[i]=THINKING; 
		printf("Philosopher %d has stopped eating.\n", i); 
		test(LEFT); //first left neighbor is poked
		test(RIGHT); //then right neighbor is poked
	up(&m); //unlock the mutex
}

//test function checks the neighbours are eating or not
void test(int i) {
	//check if neighbors are eating
	if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
	{
		//neighbors are not eating, starts eating - change state to eat
		state[i] = EATING;
		up(&s[i]); //unlock mutex
	}
}

//-------------------------------------------------------------------------------------------------------
//philosopher cycle function
void* philosopher_cycle(void* num)
{
    int* i = (int*)num;

    printf("Philosopher %d came to the table.\n", *i);

    while(1)
    {
	printf("Philosopher %d is thinking.\n", *i);
        sleep(THINKING_TIME); //sleep during thinking time
        grab_forks(*i);
        printf("Philosopher %d has started eating.\n", *i);
        sleep(EATING_TIME); //sleep during eating time
        put_away_forks(*i);
    }
    return NULL;
}


