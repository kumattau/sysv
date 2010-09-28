#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "signal.h"

void alrm_signal_handler(int signum) {
    SYSV_sighold(signum);
    printf("catch SIGUSR1 %d for signal\n", signum);
    SYSV_sigrelse(signum);
    SYSV_signal(SIGUSR1, alrm_signal_handler);
}

void alrm_sigset_handler(int signum) {
    printf("catch SIGUSR2 %d for sigset\n", signum);
}

int test_signal(void) {

    printf("[TEST SIGNAL]\n");

    int timer_count = 3, i;

    timer_t           tid1, tid2;
    struct itimerspec its1, its2;
    struct sigevent   sev1, sev2;

    SYSV_signal(SIGUSR1, alrm_signal_handler);
    SYSV_sigset(SIGUSR2, alrm_sigset_handler);

    sev1.sigev_notify = SIGEV_SIGNAL;
    sev1.sigev_signo  = SIGUSR1;
    sev2.sigev_notify = SIGEV_SIGNAL;
    sev2.sigev_signo  = SIGUSR2;

    timer_create(CLOCK_REALTIME, &sev1, &tid1);
    timer_create(CLOCK_REALTIME, &sev2, &tid2);

    its1.it_value.tv_sec     = 0;
    its1.it_value.tv_nsec    = 1000000;
    its1.it_interval.tv_sec  = 0;
    its1.it_interval.tv_nsec = 1000000;
    its2.it_value.tv_sec     = 0;
    its2.it_value.tv_nsec    = 1000000;
    its2.it_interval.tv_sec  = 0;
    its2.it_interval.tv_nsec = 1000000;

    timer_settime(tid1, 0, &its1, NULL);
    timer_settime(tid2, 0, &its2, NULL);

    for(i = 0; i < timer_count; i++) {
        printf("count = %d\n", i);
        SYSV_sigpause(SIGUSR1);
        SYSV_sigpause(SIGUSR2);
    }

    return 1;
}

int main(void) {

    printf("%s\n", test_signal() ? "OK" : "NG");

    return 0;
}

