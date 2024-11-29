#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("filename in empty\n");
        return -1;
    }

    int prosecc_id[256];
    int curr_ind = 0;

    char * filename = argv[1];
    FILE * fp;
    char * line = NULL;
    size_t len = 0;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    while ((getline(&line, &len, fp)) != -1) {
        pid_t pid = fork();
        if(pid == 0){
            int i = 0;
            char *command[len];
            char *tok;
            tok = strtok(line, " \n");

            int time_sleep;
            sscanf(tok, "%d", &time_sleep);
            while(tok) {
                tok = strtok(NULL, " \n");
                command[i] = tok;
                i++;
            }
            command[i] = NULL;
            
            sleep(time_sleep);
            int ret = execvp(command[0], command);
            if (ret == -1) {
                perror("exec");
                return -2;
            }
        } else if (pid > 0) {
            prosecc_id[curr_ind] = pid;
            curr_ind += 1;
        } else {
            perror("fork");
            return -1;
        }
    }

    fclose(fp);

    if (line)
        free(line);

    for (int i = 0; i < curr_ind; i++) {
        int status = 0;
        waitpid(prosecc_id[i], &status, 0);
    }
    exit(EXIT_SUCCESS);
}