#include <signal.h>
#include <string.h>
#include <errno.h>

#include "signal.h"

#ifndef __set_errno
#define __set_errno(e) errno = EINVAL
#endif  /* __set_errno */

#define CHECK_ARGS(signum, handler) do {                      \
    if (handler == SIG_ERR || signum < 1 || NSIG <= signum) { \
        __set_errno(EINVAL);                                  \
        return SIG_ERR;                                       \
    }                                                         \
} while(0)

/* sig_procmask ショートカット関数 */
/* sigemptyset -> sigaddset(,signum) -> sigprocmask(how,,oldset) する */
static inline int __sig_how_procmask(int how, int signum, sigset_t *oldset) {
    sigset_t newset;
    if (sigemptyset(&newset) < 0) {
        return -1;
    }
    if (sigaddset(&newset, signum) < 0) {
        return -1;
    }
    if (sigprocmask(how, &newset, oldset) < 0) {
        return -1;
    }
    return 0;
}

sighandler_t SYSV_signal(int signum, sighandler_t handler) {
    struct sigaction newact, oldact;

    CHECK_ARGS(signum, handler);

    memset(&newact, 0, sizeof(newact));
    memset(&oldact, 0, sizeof(newact));

    newact.sa_handler = handler;
    /* SYSV signal の動作を設定する */
    /* a) ハンドラ起動時に処理方法がリセットされる */
    /* b) ハンドラ実行中に原因となったシグナルがブロックされない */
    /* c) ハンドラによって中断されたシステムコールが再開されない */
    newact.sa_flags  |= SA_RESETHAND;
    newact.sa_flags  |= SA_NODEFER;
    newact.sa_flags  &= ~SA_RESTART;
    if (sigaction(signum, &newact, &oldact) < 0) {
        return SIG_ERR;
    }
    return oldact.sa_handler;
}

sighandler_t SYSV_sigset(int signum, sighandler_t handler) {
    struct sigaction newact, oldact;
    sigset_t oldset;

    CHECK_ARGS(signum, handler);

    memset(&newact, 0, sizeof(newact));
    memset(&oldact, 0, sizeof(newact));

    if (handler == SIG_HOLD) {
        /* SIG_HOLD の場合 */
        /* シグナルマスクに signum を追加 */
        if (__sig_how_procmask(SIG_BLOCK, signum, &oldset) < 0) {
            return SIG_ERR;
        }
        /* signum の動作は変更しない (newact: NULL) */
        if (sigaction(signum, NULL, &oldact) < 0) {
            return SIG_ERR;
        }
        /* 呼び出し前に signum がブロックされていた場合は SIG_HOLD を返す */
        /* ブロックされていない場合は変更前の handler を返す */
	    return sigismember(&oldset, signum) ? SIG_HOLD : oldact.sa_handler;
    } else {
        /* SIG_HOLD 以外の場合 */
        /* signum の動作を handler (SIG_DFL, SIG_IGN 含む) にセット */
	    newact.sa_handler = handler;
        /* SYSV signal と違い、シグナル実行中は signum がブロックされる */
        if (sigaddset(&newact.sa_mask, signum) < 0) {
            return SIG_ERR;
        }
	    if (sigaction(signum, &newact, &oldact) < 0) {
	        return SIG_ERR;
	    }
        /* シグナルマスクから signum を取り除く */
	    if (__sig_how_procmask(SIG_UNBLOCK, signum, &oldset) < 0) {
	        return SIG_ERR;
	    }
        /* 呼び出し前に signum がブロックされていた場合は SIG_HOLD を返す */
        /* ブロックされていない場合は変更前の handler を返す */
	    return sigismember(&oldset, signum) ? SIG_HOLD : oldact.sa_handler;
    }
}

int SYSV_sighold(int signum) {
    /* 呼び出し元プロセスのシグナルマスクに signum を追加する */
    /* 成功した場合 0, 失敗した場合 1 を返す */
    return __sig_how_procmask(SIG_BLOCK, signum, NULL);
}
int SYSV_sigrelse(int signum) {
    /* 呼び出し元プロセスのシグナルマスクから signum を削除する */
    /* 成功した場合 0, 失敗した場合 1 を返す */
    return __sig_how_procmask(SIG_UNBLOCK, signum, NULL);
}
int SYSV_sigignore(int signum) {
    /* signum の動作を SIG_IGN に設定する */
    /* 成功した場合 0, 失敗した場合 1 を返す */
    if (SYSV_signal(signum, SIG_IGN) == SIG_ERR) {
        return -1;
    }
    return 0;
}

int SYSV_sigpause(int signum) {
    sigset_t nowset;
    /* signum を呼び出しプロセスのシグナルマスクから削除 */
    if (SYSV_sigrelse(signum) < 0) {
        return -1;
    }
    /* シグナルが受信されるまで、呼び出しプロセスを保留 */
    if (sigprocmask(SIG_SETMASK, NULL, &nowset) < 0) {
        return -1;
    }
    return sigsuspend(&nowset);
}
