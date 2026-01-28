#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *a){
    fprintf(stderr,"Usage: %s <pid>\n",a);
    exit(1);
}

static int isnum(const char*s){
    for(;*s;s++) if(!isdigit(*s)) return 0;
    return 1;
}

int main(int c,char**v){
    if(c!=2||!isnum(v[1])) usage(v[0]);

    int pid = atoi(v[1]);
    char path[256];
    FILE *f;

    sprintf(path, "/proc/%d/stat", pid);
    f = fopen(path, "r");
    if (!f){
        perror("stat");
        return 1;
    }

    char state;
    int ppid;
    long utime, stime;

    fscanf(f, "%*d (%*[^)]) %c %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld",
           &state, &ppid, &utime, &stime);
    fclose(f);

    sprintf(path, "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f){
        perror("status");
        return 1;
    }

    char line[256];
    long vmrss = 0;
    while (fgets(line, sizeof(line), f)){
        if (strncmp(line, "VmRSS:", 6) == 0){
            sscanf(line, "VmRSS: %ld kB", &vmrss);
            break;
        }
    }
    fclose(f);

    sprintf(path, "/proc/%d/cmdline", pid);
    f = fopen(path, "r");
    if(!f){
        perror("cmdline");
        return 1;
    }

    char cmd[1024];
    int n = fread(cmd, 1, sizeof(cmd)-1, f);
    fclose(f);
    cmd[n] = '\0';

    for(int i = 0; i < n; i++)
        if (cmd[i] == '\0') cmd[i] = ' ';

    printf("State: %c\n", state);
    printf("PPID: %d\n", ppid);
    printf("Cmd: %s\n", cmd);
    printf("CPU time (utime+stime): %ld\n", utime + stime);
    printf("VmRSS: %ld kB\n", vmrss);

    return 0;
}
