#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
        if(argc != 2) return -1;
        char *envp[] = {"", "", argv[1], NULL};
        char *const args[] = {NULL};

        execve("/vortex/vortex4", args, envp);
}
