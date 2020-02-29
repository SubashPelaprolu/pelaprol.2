#include <stdio.h>
#include <stdlib.h>	
#include <semaphore.h>
#include "user.h"
#include "oss.h"

static OSS *ossaddr = NULL;

static int check_number(const int number, const unsigned int start, const unsigned int end){
	int i;
	for(i=2; i < number; i++){
			if((number % i) == 0){
				return 0;	
			}

			
			sem_wait(&ossaddr->mutex);
			if((ossaddr->clock.tv_nsec < start) || (ossaddr->clock.tv_nsec >= end)){
				sem_post(&ossaddr->mutex);
				return -1;	
			}
			sem_post(&ossaddr->mutex);
	}
	return 1; /* its a prime number */
}

static void save_results(struct user * u, const int result){

	if(sem_wait(&ossaddr->mutex) == -1){
		perror("sem_wait");
	}

	switch(result){
		default:
		case -1:	u->checked = 0;	break;

		case 0:		u->prime   = 0;
							u->checked = 1;	break;

		case 1:		u->checked = 1;
							u->prime   = 1;		break;
	}

	if(sem_post(&ossaddr->mutex) == -1){
		perror("sem_wait");
	}
}

int main(const int argc, char * argv[]) {

	const int user_id = atoi(argv[1]);

	ossaddr = oss_init(0);
	if(ossaddr == NULL){
		return 1;
	}

	const unsigned int one_ms = 1000000;
	if(sem_wait(&ossaddr->mutex) == -1){
		perror("sem_wait");
		return 2;
	}
	const unsigned int start = ossaddr->clock.tv_nsec;

	if(sem_post(&ossaddr->mutex) == -1){
		perror("sem_wait");
		return 2;
	}
	const unsigned int end = start + one_ms;


	const int prime_number = check_number(ossaddr->users[user_id].number, start, end);
	save_results(&ossaddr->users[user_id], prime_number);

	oss_deinit();
	return 0;
}
