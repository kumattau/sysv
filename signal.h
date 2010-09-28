#ifndef SIGNAL_H_
#define SIGNAL_H_

typedef void (*sighandler_t)(int);

sighandler_t SYSV_signal(int signum, sighandler_t handler);
sighandler_t SYSV_sigset(int signum, sighandler_t handler);

int SYSV_sighold(int signum);
int SYSV_sigrelse(int signum);
int SYSV_sigignore(int signum);
int SYSV_sigpause(int signum);

#ifdef SYSVSIGNAL
#define signal    SYSV_signal
#define sigset    SYSV_sigset
#define sighold   SYSV_sighold
#define sigrelse  SYSV_sigrelse
#define sigignore SYSV_sigignore
#define sigpause  SYSV_sigpause
#endif  /* SYSVSIGNAL */

#endif  /* SIGNAL_H_ */
