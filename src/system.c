#include "include/system.h"

#include <sys/types.h>
#include <unistd.h>

bool
shell(char *cmd) {
    pid_t pid = fork();
    if(pid == 0) {
        // create a new session so we are not waiting for children
        setsid();

        // close its io streams so they are not cluttering our logs
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // exec it with a shell so you can have your piping, extrapolation etc
        execlp("sh", "sh", "-c", cmd, (char *)NULL);

        // child should not execute the parent code if the exec fails
        _exit(127);
    }

    return pid > 0;
}
