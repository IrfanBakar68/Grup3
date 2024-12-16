#ifndef SHELL_H
#define SHELL_H

// Gerekli Kütüphaneler
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Makrolar
#define MAX_ARGS 64
#define MAX_COMMAND_LEN 1024
#define MAX_HISTORY 100
#define MAX_JOBS 100
#define MAX_PIPES 10

// Veri Yapıları
typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LEN];
    int active;
} Job;

// Global Değişkenler
extern char history[MAX_HISTORY][MAX_COMMAND_LEN];
extern int history_count;

extern Job jobs[MAX_JOBS];
extern int job_count;

// Fonksiyon Prototipleri
void handle_signal(int signal);
void add_to_history(const char *command);
void print_history();
void print_jobs();
void wait_for_background_jobs();
void execute_piped_commands(char *commands[], int num_pipes);
void execute_multiple_commands(char *command);
void execute_command(char *command);

#endif // SHELL_H
