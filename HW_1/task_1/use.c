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
        } else {
            perror("fork");
            return -1;
        }
    }

    fclose(fp);
    
    while( waitpid(-1, NULL, 0) > 0);
        
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}