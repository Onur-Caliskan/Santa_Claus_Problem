#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

//initilazing global variables
#define NUM_REINDEER 9
#define MAX_ELVES_WAITING 3

//creating mutex and semaphores
sem_t santaSem, reindeerSem, elfSem;
sem_t reindeerMutex, elfMutex, elfCounterMutex;
int numElvesWaiting = 0;
int numElvesGettingHelp = 0;
int reindeerArrived = 0;


// a function for santa claus to preapers his sleigh
void prepareSleigh() {
    printf("Santa Claus is preparing the sleigh.\n");
    fflush(stdout);
    sleep(2); // Simulate sleigh preparation time
}

// a function for reindeer that semaphore calls 
void getHitched(int reindeerId) {
    printf("Reindeer %d is getting hitched to the sleigh.\n", reindeerId);
    fflush(stdout);
}

//a function for santa that if semaphore calls function triggers...
void helpElves() {
    printf("Santa Claus is helping the elves.\n");
    fflush(stdout);
    sleep(1); // Simulate helping time
}


//elf semaphore triggers that function to take help from santa
void getHelp(int elfId) {
    printf("Elf %d is getting help from Santa Claus.\n", elfId);
    fflush(stdout);
}

//function for santa claus to wait lock and semaphore to help elves or preaperesleigh 
void *santaClaus(void *arg) {
    while (1) {
        sem_wait(&santaSem);
		
		//semaphore connected to reindeer mutex if it calls semaphore triggers
        sem_wait(&reindeerMutex);
        if (reindeerArrived == NUM_REINDEER) {
            prepareSleigh();
            //for every reindeer semaphore works
            for (int i = 0; i < NUM_REINDEER; i++) {
                sem_post(&reindeerSem);
            }
            //after job is finished function sets arrived reindeers to zero
            reindeerArrived = 0;
        }
        sem_post(&reindeerMutex);
		
		// it is a code block to determine is enough elves come to take help from santa
        sem_wait(&elfMutex);
        if (numElvesWaiting >= MAX_ELVES_WAITING) {
            helpElves();
            for (int i = 0; i < MAX_ELVES_WAITING; i++) {
                sem_post(&elfSem);
            }
            numElvesGettingHelp += MAX_ELVES_WAITING;
            numElvesWaiting -= MAX_ELVES_WAITING;
        }
        sem_post(&elfMutex);
    }

    return NULL;
}

//reindeer function to trigger santa's functions
void *reindeer(void *arg) {
    int id = *((int *)arg);
    sleep(rand() % 5 + 1);

    while (1) {
        sem_wait(&reindeerMutex);
        printf("Reindeer %d has returned from vacation.\n", id);
        fflush(stdout);

        reindeerArrived++;
        //if all reindeers come back from vacation they trigger santasemaphore for preaparing sleigh
        if (reindeerArrived == NUM_REINDEER) {
            sem_post(&santaSem);
        }
        sem_post(&reindeerMutex);

        sem_wait(&reindeerSem);
        getHitched(id);
        sem_post(&reindeerSem);

        // Reindeer go on vacation
        sleep(rand() % 5 + 1);
    }

    return NULL;
}


//elf fucntion to trigger santa to take help or trigger gethelp function for elves
void *elf(void *arg) {
    int id = *((int *)arg);
    sleep(rand() % 5 + 1);

    while (1) {
        sem_wait(&elfMutex);
        printf("Elf %d needs help from Santa Claus.\n", id);
        fflush(stdout);
		
		//increments number of elves arrived to santa to invoke semaphore
        numElvesWaiting++;
        if (numElvesWaiting == MAX_ELVES_WAITING) {
            sem_post(&santaSem);
        }
        sem_post(&elfMutex);

        sem_wait(&elfSem);
        getHelp(id);

        sem_wait(&elfCounterMutex);
        numElvesGettingHelp++;
        if (numElvesGettingHelp == MAX_ELVES_WAITING) {
        	//if all elves got help it sets to zero 
            numElvesGettingHelp = 0;
            sem_post(&elfCounterMutex);
        } else {
            sem_post(&elfSem);
            sem_post(&elfCounterMutex);
            sem_wait(&elfSem);
        }

        // Elves return to work
        sleep(rand() % 5 + 1);
    }

    return NULL;
}

int main() {
	
	//initilazing threads
    pthread_t santaThread;
    pthread_t reindeerThreads[NUM_REINDEER];
    pthread_t elfThreads[MAX_ELVES_WAITING];
    int reindeerIds[NUM_REINDEER], elfIds[MAX_ELVES_WAITING];
    int i;

    // Initialize semaphores
    sem_init(&santaSem, 0, 0);
    sem_init(&reindeerSem, 0, 0);
    sem_init(&elfSem, 0, 0);
    sem_init(&reindeerMutex, 0, 1);
    sem_init(&elfMutex, 0, 1);
    sem_init(&elfCounterMutex, 0, 1);

    // Create Santa Claus thread
    pthread_create(&santaThread, NULL, santaClaus, NULL);

    // Create reindeer threads
    for (i = 0; i < NUM_REINDEER; i++) {
        reindeerIds[i] = i + 1;
        pthread_create(&reindeerThreads[i], NULL, reindeer, &reindeerIds[i]);
    }

    // Create elf threads
    for (i = 0; i < MAX_ELVES_WAITING; i++) {
        elfIds[i] = i + 1;
        pthread_create(&elfThreads[i], NULL, elf, &elfIds[i]);
    }

    // Wait for all threads to finish
    pthread_join(santaThread, NULL);
    for (i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeerThreads[i], NULL);
    }
    for (i = 0; i < MAX_ELVES_WAITING; i++) {
        pthread_join(elfThreads[i], NULL);
    }

    // Destroy semaphores
    sem_destroy(&santaSem);
    sem_destroy(&reindeerSem);
    sem_destroy(&elfSem);
    sem_destroy(&reindeerMutex);
    sem_destroy(&elfMutex);
    sem_destroy(&elfCounterMutex);

    return 0;
}
