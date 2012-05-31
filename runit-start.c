#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAXINTLEN 10

/*
(c) Alexander Rumyantsev, Wikimart, 2012
This code is published under the terms of General Public License
*/

int main(int argc, char ** argv) {

    char *svlogd_args[] = {"svlogd", (char *)NULL, (char *)NULL};
    char *runsvdir_args[] = {"runsvdir", "-P", (char *)NULL, (char *)NULL};
    char *pids = malloc(MAXINTLEN + 1);
    struct stat buf;

    if (argc < 3) {
        printf("Usage: runit-start [service dir] [log dir]\n");
        return 1;
    }

    if (stat(argv[1], &buf) < 0) {
        perror("stat");
        return 1;
    }

    if (! S_ISDIR(buf.st_mode)) {
        printf("arg 1 is not a directory\n");
        return 1;
    }

    if (stat(argv[2], &buf) < 0) {
        perror("stat");
        return 1;
    }

    if (! S_ISDIR(buf.st_mode)) {
        printf("arg 2 is not a directory\n");
        return 1;
    }

    runsvdir_args[2] = argv[1];
    svlogd_args[1] = argv[2];

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    int devnull = open("/dev/null",O_RDWR);
    if (devnull < 0) {
        perror("open /dev/null");
        return 1;
    }

    chdir("/");

    signal(SIGCHLD, SIG_IGN);

    pid_t svlogd = fork();

    if (svlogd < 0)
        return 1;

    if (svlogd == 0) {
    /* we are child, run logger */
        pid_t svlogd_s = setsid();
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);

        execvp("svlogd", svlogd_args) < 0;
        return 1;
    }

    pid_t runsvdir = fork();

    if (runsvdir < 0)
        return 1;

    if (runsvdir == 0) {
    /* we are child, run runsvdir */
        pid_t svlogd_s = setsid();
        close(pipefd[0]);
        dup2(devnull, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        execvp("runsvdir", runsvdir_args) < 0;
        return 1;
    }

    int pidf;
    int pidl;
    pidf = open("/var/run/svlogd.pid", O_WRONLY | O_CREAT | O_TRUNC);
    pidl = snprintf(pids, MAXINTLEN, "%d", svlogd);
    write(pidf, pids, pidl);
    close(pidf);
    pidf = open("/var/run/runsvdir.pid", O_WRONLY | O_CREAT | O_TRUNC);
    pidl = snprintf(pids, MAXINTLEN, "%d", runsvdir);
    write(pidf, pids, pidl);
    close(pidf);

    return 0;
}
