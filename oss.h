typedef struct {
	pid_t oss_pid;									
	sem_t mutex;										
	struct timespec clock;					
	struct user users[USER_LIMIT];
} OSS;

OSS* oss_init(const int is_oss);
void oss_deinit();
