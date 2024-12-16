/*
Hüseyin Göbekli (G211210041) 2B 
Okan Başol (G211210083) 2B 
Şimal Ece Kazdal (G221210068) 2B 
Muhammed İrfan Bakar (G221210596) 2B 
Betül Kurt (G221210054) 2C  
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "shell.h"


#define MAX_ARGS 64
#define MAX_COMMAND_LEN 1024
#define MAX_HISTORY 100
#define MAX_JOBS 100
#define MAX_PIPES 10

// Fonksiyon Prototipleri
void execute_command(char *command); // Tek bir komut çalıştırma fonksiyonu prototipi
void execute_multiple_commands(char *command); // Çoklu komutları işleme fonksiyonu prototipi
void execute_piped_commands(char *commands[], int num_pipes); // Pipe desteği için prototip

// Komut geçmişi
typedef struct {
    char command[MAX_COMMAND_LEN];
} History;

History history[MAX_HISTORY];
int history_count = 0;

// Arka plan çalışan süreçlerin bilgisi
typedef struct {
    pid_t pid;
    char command[MAX_COMMAND_LEN];
    int active;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;

// Sinyal işleyici fonksiyonu
void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nCtrl+C algılandı. Shell çalışmaya devam ediyor.\n> ");
        fflush(stdout);
    } else if (signal == SIGTSTP) {
        printf("\nCtrl+Z devre dışı bırakıldı.\n> ");
        fflush(stdout);
    }
}

// Komut geçmişine ekleme
void add_to_history(const char *command) {
    if (history_count < MAX_HISTORY) {
        strncpy(history[history_count++].command, command, MAX_COMMAND_LEN);
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strncpy(history[i - 1].command, history[i].command, MAX_COMMAND_LEN);
        }
        strncpy(history[MAX_HISTORY - 1].command, command, MAX_COMMAND_LEN);
    }
}

// Komut geçmişini yazdırma
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i].command);
    }
}

// Arka plan süreçlerini listeleme
void print_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active) {
            printf("[%d] PID: %d, Komut: %s\n", i + 1, jobs[i].pid, jobs[i].command);
        }
    }
}

// Arka plan işlemleri bekleme
void wait_for_background_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active) {
            int status;
            printf("Arka plan süreci bekleniyor. PID: %d\n", jobs[i].pid);
            waitpid(jobs[i].pid, &status, 0);
            printf("Süreç tamamlandı. PID: %d, Durum: %d\n", jobs[i].pid, WEXITSTATUS(status));
            jobs[i].active = 0;
        }
    }
}

// Boru ile komut çalıştırma
void execute_piped_commands(char *commands[], int num_pipes) {
    int pipe_fds[2 * num_pipes];
    pid_t pid;

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fds + i * 2) < 0) {
            perror("Pipe oluşturulamadı");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i <= num_pipes; i++) {
        pid = fork();
        if (pid == 0) {
            // Pipe girişini ayarla
            if (i != 0) {
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            // Pipe çıkışını ayarla
            if (i != num_pipes) {
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fds[j]);
            }

            char *args[MAX_ARGS];
            int arg_idx = 0;
            char *token = strtok(commands[i], " ");
            while (token != NULL) {
                args[arg_idx++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_idx] = NULL;

            execvp(args[0], args);
            perror("Komut çalıştırılamadı");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Fork hatası");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipe_fds[i]);
    }

    for (int i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}

// Çoklu komutları işleme
void execute_multiple_commands(char *command) {
    char *sub_command = strtok(command, ";");
    while (sub_command != NULL) {
        // Alt komutu çalıştır
        execute_command(sub_command);
        sub_command = strtok(NULL, ";");
    }
}

// Komut çalıştırma (Giriş/Çıkış Yönlendirme ve Boru Desteği)
void execute_command(char *command) {
    char *args[MAX_ARGS];
    char *commands[MAX_PIPES];
    int i = 0, num_pipes = 0;
    int background = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;

    // Çoklu komut desteği
    if (strchr(command, ';') != NULL) {
        execute_multiple_commands(command);
        return;
    }

    // Komutları pipe'lara göre böl
    char *token = strtok(command, "|");
    while (token != NULL) {
        commands[num_pipes++] = token;
        token = strtok(NULL, "|");
    }

    if (num_pipes > 1) {
        execute_piped_commands(commands, num_pipes - 1);
        return;
    }

    // Tekli komut ayrıştırma
    token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            background = 1;
        } else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            output_file = token;
            append = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            output_file = token;
            append = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL) {
        return; // Boş komut
    }

    // Built-in komutlar
    if (strcmp(args[0], "history") == 0) {
        print_history();
        return;
    }

    if (strcmp(args[0], "jobs") == 0) {
        print_jobs();
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Giriş yönlendirme
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Giriş dosyası açılamadı");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Çıkış yönlendirme
        if (output_file != NULL) {
            int fd;
            if (append) {
                fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd < 0) {
                perror("Çıkış dosyası açılamadı");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror("Komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        if (background) {
            jobs[job_count].pid = pid;
            strncpy(jobs[job_count].command, command, MAX_COMMAND_LEN);
            jobs[job_count].active = 1;
            job_count++;
            printf("Süreç arka planda çalışıyor. PID: %d\n", pid);
        } else {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("Fork hatası");
    }
}

int main() {
    char command[MAX_COMMAND_LEN];

    // Sinyalleri işlemek için sinyal işleyicilerini ayarla
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(command, sizeof(command), stdin)) {
            perror("Komut okunamadı");
            continue;
        }

        command[strcspn(command, "\n")] = 0; // Yeni satır karakterini kaldır

        if (strcmp(command, "quit") == 0) {
            printf("Shell sonlandırılıyor, arka plan işlemleri bekleniyor...\n");
            wait_for_background_jobs();
            printf("Shell kapatıldı.\n");
            break;
        }

        add_to_history(command); // Komutu geçmişe ekle
        execute_command(command); // Komutu çalıştır
    }

    return 0;
}
