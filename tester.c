#include <stdio.h>
#include <sys/time.h>

int main(void)
{
    struct timeval start, finish;
    double time;

    gettimeofday(&start, NULL);

    for(int i=0; i<1000000;i++){
        i++;
    }

    gettimeofday(&finish, NULL);

    time = (finish.tv_sec - start.tv_sec) ;
    time += (finish.tv_usec - start.tv_usec) / 1000000.0;
    printf("TecnicoFS completed in %.4lf seconds.\n", time);
}