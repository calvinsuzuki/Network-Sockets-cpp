#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int x = 0;

void* func(void *arg) {
    int *a = (int *)arg;

    // pthread_mutex_lock(&clients_mutex);
    sleep(1);
    x += a[0];
    printf("Thread: %i\n", x);
    // pthread_mutex_unlock(&clients_mutex);
}

int main() {

    int *num = (int*)malloc( sizeof(int) );
    num[0] = 1;
    pthread_t tr1;
    pthread_t tr2;
    pthread_create(&tr1, NULL, &func, (void*)num);
    pthread_create(&tr2, NULL, &func, (void*)num);

    sleep(3);
    printf("Main: %i\n", x);
    return 0;
}