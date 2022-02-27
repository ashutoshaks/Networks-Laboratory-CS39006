#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex;
int x = 0;

void handler(int signum) {
    if (signum == SIGUSR1) {
        pthread_exit(NULL);
    }
}

void *runner(void *arg) {
    signal(SIGUSR1, handler);
    pthread_mutex_lock(&mutex);
    x++;
    pthread_mutex_unlock(&mutex);

    while (1)
        ;
}

int main() {
    pthread_t tid;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&tid, NULL, runner, NULL);

    // pthread_mutex_lock(&mutex);
    // x++;
    // pthread_mutex_unlock(&mutex);
    sleep(2);
    printf("%d\n", x);
    pthread_mutex_lock(&mutex);
    pthread_kill(tid, SIGUSR1);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    printf("%d\n", x);
    return 0;
}