#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


bool file_exists (char *filename) {
    struct stat buffer;   
    return (stat(filename, &buffer) == 0);
}

bool is_need_backup(char source[], char drain[]){
    char abs_path_drain[1024];
    realpath(drain, abs_path_drain);
    if (!file_exists(abs_path_drain)){
        return true;
    }
    char abs_path_source[1024];
    realpath(source, abs_path_source);
    struct stat src;
    struct stat drn;
    stat(abs_path_drain, &drn);
    stat(abs_path_source, &src);
    if (drn.st_mtime < src.st_mtime){
        return true;
    }
    return false;
}

void make_directory(char source_path[], char relative_path[]){
    char abs_path[1024];
    realpath(relative_path,abs_path);

    struct stat st;
    stat(source_path, &st);
    mkdir(abs_path, st.st_mode);
}

void copy_to_drain(char source_path[], char drain_path[]){
    pid_t pid = fork();
    if (pid == 0){
        char * command[] = {"cp", source_path, drain_path, NULL};
        execvp(command[0], command);
    } else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
    }else {
        perror("fork error");
    }
}

void to_gz(char drain_path[]){
    pid_t pid = fork();
    if (pid == 0){
        char * command[] = {"gzip", drain_path, "--force" ,NULL};
        execvp(command[0], command);
    } else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
    }else {
        perror("fork error");
    }
}

void backup_dir(char source_name[],char drain_name[]){
    DIR* source = opendir(source_name);
    struct dirent *entry;
    while((entry=readdir(source)) != NULL){
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")){
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", drain_name, entry->d_name);

        char source_path[1024];
        char drain_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_name, entry->d_name);
        snprintf(drain_path, sizeof(drain_path), "%s/%s", drain_name, entry->d_name);

        struct stat source_stat;
        stat(source_path, &source_stat);
        
        if (S_ISDIR(source_stat.st_mode)){
            if (!strcmp(source_path, drain_name)){
                continue;
            }
            if (!file_exists(path)){
                make_directory(source_path, drain_path);
            }
            backup_dir(source_path, drain_path);
        }
        else if (S_ISREG(source_stat.st_mode)){
            char drain_path_with_gz[1024];
            snprintf(drain_path_with_gz, sizeof(drain_path_with_gz), "%s.%s", drain_path, "gz");
            if (is_need_backup(source_path, drain_path_with_gz)){
                copy_to_drain(source_path, drain_name);
                to_gz(drain_path);
            }
        }
    }
}

int main(int argc, char *argv[])
{   
    if (argc != 3){
        printf("wrong arguments\n");
        return 0;
    }
    if (!file_exists(argv[2]) || !file_exists(argv[1])){
        printf("such folder does not exist\n");
        return 0;
    }
    if (!strcmp(argv[1], argv[2])){
        printf("directories cannot match\n");
        return 0;
    }
    backup_dir(argv[1], argv[2]);
}