#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include "user.h"
#include "oss.h"

static OSS *ossaddr = NULL;
static char * logfile = NULL;

unsigned int max_procs = 2;
unsigned int num_procs = 0;
unsigned int ext_procs = 0;

unsigned int max_users = 4;
unsigned int num_users = 0;

static int seq_base = 101;
static int seq_step = 4;

static void wait_users(struct user * users, const unsigned int size, const int nowait){
	int i, status;
	const int flags = (nowait) ? WNOHANG : 0;
	for(i=0; i < size; i++){
		if(users[i].pid > 0){
			if(waitpid(users[i].pid, &status, flags) > 0){
				users[i].pid = 0;
				num_procs--;
				ext_procs++;
				printf("Child %u was terminated at time %lu.%lu\n", i, ossaddr->clock.tv_sec, ossaddr->clock.tv_nsec);
			}
		}
	}
}

static void stop_users(struct user * users, const unsigned int size){
	int i;

	for(i=0; i < size; i++){
		if(users[i].pid > 0){
			if(kill(users[i].pid, SIGTERM)){
				users[i].pid = 0;
				num_procs--;
				ext_procs++;
				printf("Child %u was terminated at time %lu.%lu\n", i, ossaddr->clock.tv_sec, ossaddr->clock.tv_nsec);
			}
		}
	}
}

static void signal_handler(const int sig){

	switch(sig){
		case SIGCHLD:
			wait_users(ossaddr->users, max_users, 1);
			break;
		case SIGINT: case SIGTERM: case SIGALRM:
			stop_users(ossaddr->users, max_users);
			ext_procs = USER_LIMIT;
		default:
			printf("Signal %d at time %lu.%lu\n", sig, ossaddr->clock.tv_sec, ossaddr->clock.tv_nsec);
			break;
	}
}

static int user_fork(const unsigned int user_id){

	char arg[100];
	snprintf(arg, sizeof(arg), "%u", user_id);

	const pid_t pid = fork();
	switch(pid){
		case -1:
			perror("fork");
			break;

		case 0:
			ossaddr->users[user_id].pid = getpid();
			execl("./user", "./user", arg, NULL);
			perror("execl");
			exit(EXIT_FAILURE);

		default:
			printf("Started child %d with PID=%d for number %d at time %lu.%lu\n",
				user_id, pid, ossaddr->users[num_users].number, ossaddr->clock.tv_sec, ossaddr->clock.tv_nsec);
			break;
	}

	return pid;
}

static void advance_time(){
	const unsigned int ns_in_sec = 1000000000;
	const unsigned int ns_step = 10000;

	ossaddr->clock.tv_nsec += ns_step;
	if(ossaddr->clock.tv_nsec > ns_in_sec){
    ossaddr->clock.tv_sec += 1;
    ossaddr->clock.tv_nsec %= ns_in_sec;
  }
}

static void search_primes(){

	while(ext_procs < max_users){

		if(	(num_procs < max_procs) &&
				(num_users < max_users)){

			const pid_t pid = user_fork(num_users);
			if(pid > 0){
				num_users++;
				num_procs++;
			}else if(pid == -1){
				break;
			}
		}

		sem_wait(&ossaddr->mutex);
		advance_time();
		sem_post(&ossaddr->mutex);
		usleep(100);
	}
}

static void print_primes(struct user * users, const int size){
	int i;

	for(i=0; i < size; i++){
		if(users[i].prime){
			printf("%d is PRIME number\n", users[i].number);
		}else if(users[i].checked){
			printf("%d is NOT a PRIME number\n", -1*users[i].number);
		}else{
			printf("%d was NOT checked\n", users[i].number);
		}
	}
}

static int parse_arguments(const int argc, char * const * argv){

	int rv=0,opt;
	while((opt = getopt(argc, argv, "ho:n:s:b:i:")) != -1){
		switch(opt){
			case 'h':
				printf("Usage: ./oss [-h] -n max_users -s max_procs -b first number -i increment -o logfile\n");
				printf("\t-%c\t  \t\t%s\n", 'h', "Help");
				printf("\t-%c\t%d\t\t%s\n", 'n', max_users, "Maximum users");
				printf("\t-%c\t%d\t\t%s\n", 's', max_procs, "Maximum processes");
				printf("\t-%c\t%d\t\t%s\n", 'b', seq_base, "First sequence number");
				printf("\t-%c\t%d\t\t%s\n", 'i', seq_step, "Sequence increment");
				printf("\t-%c\t%s\t%s\n", 'o', "output.txt", "Log filename");
				exit(0);

			case 'o': logfile = optarg;
				break;

			case 'n':	max_users = atoi(optarg);
				break;
			case 's': max_procs	= atoi(optarg);
        break;

			case 'b': seq_base = atoi(optarg);
				break;
			case 'i': seq_step = atoi(optarg);
	      break;

			default:
				fprintf(stderr, "Error: Invalid option '%c'\n", opt);
				rv = -1;
		}
	}
	return rv;
}

static int check_arguments(){

	if(logfile){
		stdout = freopen(logfile, "w", stdout);
	}else{
		stdout = freopen("output.txt", "w", stdout);
	}

  if(stdout == NULL){
		perror("freopen");
		return -1;
	}

	if((max_users > USER_LIMIT) || (max_procs > USER_LIMIT)){
		fprintf(stderr, "Error: User limit for -n/-s is %d", USER_LIMIT);
		return -1;
	}

	return 0;
}

static void init_users(struct user * users, const int size){
	int i;
	for(i=0; i < size; i++){
		users[i].number = seq_base + (i*seq_step);
		users[i].checked = 0;
		users[i].prime = 0;
	}
}

int main(const int argc, char * const argv[]) {

	signal(SIGINT,	signal_handler);
  signal(SIGALRM, signal_handler);
	signal(SIGCHLD, signal_handler);

	alarm(2);

	ossaddr = oss_init(1);
	if(ossaddr == NULL){
		return EXIT_FAILURE;
	}

	if(	(parse_arguments(argc, argv) == -1) ||
			(check_arguments() == -1)){
		return EXIT_FAILURE;
	}

	init_users(ossaddr->users, max_users);
	search_primes();
	print_primes(ossaddr->users, max_users);

	printf("Finished at %lu.%lu\n", ossaddr->clock.tv_sec, ossaddr->clock.tv_nsec);
	fflush(stdout);

	wait_users(ossaddr->users, max_users, 0);
	oss_deinit();

	return EXIT_SUCCESS;
}
