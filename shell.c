#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define HISTORY_SIZE 100 // Komut geçmişi için maksimum boyut

// Prototipler
void handle_background_process(int sig);
void add_to_history(const char *command);
void print_history();
int execute_history_command(const char *command);
int execute_builtin(char *args[]);
int execute_pipeline(char *commands[], int num_commands);
void execute_sequential_commands(char *input);

// Komut geçmişini tutan yapı
char *history[HISTORY_SIZE];
int history_count = 0;

// Komut geçmişine ekleme
void add_to_history(const char *command) {
    if (history_count < HISTORY_SIZE) {
        history[history_count++] = strdup(command);
    } else {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history[HISTORY_SIZE - 1] = strdup(command);
    }
}

// Komut geçmişini yazdırma
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s", i + 1, history[i]);
    }
}

// Geçmiş komutunu çalıştırma
int execute_history_command(const char *command) {
    if (command[0] == '!') {
        int num = atoi(command + 1);
        if (num > 0 && num <= history_count) {
            printf("Tekrar çalıştırılıyor: %s", history[num - 1]);
            return system(history[num - 1]);
        } else {
            fprintf(stderr, "Hata: Geçersiz geçmiş komut numarası\n");
            return -1;
        }
    }
    return 0; // Geçmiş komutu değil
}

// Arka plan süreçlerini yönetme
void handle_background_process(int sig) {
    (void)sig; // Kullanılmayan parametreyi işaretle, uyarıyı bastır
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        printf("[Arka planda bir süreç tamamlandı]\n");
    }
}


// Yerleşik komutları çalıştırma
int execute_builtin(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: hedef belirtilmedi\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd başarısız");
            }
        }
        return 1; // Yerleşik komut işlendi
    } else if (strcmp(args[0], "exit") == 0) {
        printf("Shell Sonlandırılıyor...\n");
        exit(0);
    } else if (strcmp(args[0], "history") == 0) {
        print_history();
        return 1;
    }
    return 0; // Yerleşik komut değil
}

// Boru komutlarını çalıştırma
int execute_pipeline(char *commands[], int num_commands) {
    int i;
    int pipe_fd[2];
    int prev_fd = -1;

    for (i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            if (pipe(pipe_fd) == -1) {
                perror("Pipe oluşturulamadı");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if (i < num_commands - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
            char *args[] = {"/bin/sh", "-c", commands[i], NULL};
            execvp(args[0], args);
            perror("execvp başarısız");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            if (prev_fd != -1) {
                close(prev_fd);
            }
            if (i < num_commands - 1) {
                close(pipe_fd[1]);
                prev_fd = pipe_fd[0];
            }
            waitpid(pid, NULL, 0);
        } else {
            perror("fork başarısız");
            return -1;
        }
    }

    return 0;
}

// Sıralı komutları çalıştırma
void execute_sequential_commands(char *input) {
    char *commands[64];
    int num_commands = 0;

    char *token = strtok(input, ";\n");
    while (token != NULL && num_commands < 64) {
        commands[num_commands++] = token;
        token = strtok(NULL, ";\n");
    }

    for (int i = 0; i < num_commands; i++) {
        char *current_command = commands[i];
        char *args[64];
        char *input_file = NULL;
        char *output_file = NULL;
        int background = 0;
        int arg_count = 0;

        char *sub_token = strtok(current_command, " \t\n");
        while (sub_token != NULL) {
            if (strcmp(sub_token, "<") == 0) {
                input_file = strtok(NULL, " \t\n");
            } else if (strcmp(sub_token, ">") == 0) {
                output_file = strtok(NULL, " \t\n");
            } else if (strcmp(sub_token, "&") == 0) {
                background = 1;
            } else {
                args[arg_count++] = sub_token;
            }
            sub_token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL;

        if (args[0] != NULL && execute_builtin(args)) {
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (input_file) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("Giriş dosyası açılamadı");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (output_file) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Çıkış dosyası açılamadı");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("execvp başarısız");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            if (!background) {
                waitpid(pid, NULL, 0);
            } else {
                printf("[Arka planda çalışan PID: %d]\n", pid);
            }
        } else {
            perror("fork başarısız");
        }
    }
}

int main() {
    printf("Merhaba, Shell Uygulamasına Hoş Geldiniz!\n");

    signal(SIGCHLD, handle_background_process);

    while (1) {
        printf("> ");
        fflush(stdout);

        char command[256];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        add_to_history(command);

        if (strchr(command, ';')) {
            execute_sequential_commands(command);
        } else if (execute_history_command(command) == 0) {
            char *commands[64];
            int num_commands = 0;

            char *token = strtok(command, "|\n");
            while (token != NULL && num_commands < 64) {
                commands[num_commands++] = token;
                token = strtok(NULL, "|\n");
            }

            if (num_commands > 1) {
                if (execute_pipeline(commands, num_commands) == -1) {
                    fprintf(stderr, "Bir boru komutunda hata oluştu\n");
                }
            } else {
                char *args[64];
                char *input_file = NULL;
                char *output_file = NULL;
                int background = 0;
                int arg_count = 0;

                token = strtok(command, " \t\n");
                while (token != NULL) {
                    if (strcmp(token, "<") == 0) {
                        input_file = strtok(NULL, " \t\n");
                    } else if (strcmp(token, ">") == 0) {
                        output_file = strtok(NULL, " \t\n");
                    } else if (strcmp(token, "&") == 0) {
                        background = 1;
                    } else {
                        args[arg_count++] = token;
                    }
                    token = strtok(NULL, " \t\n");
                }
                args[arg_count] = NULL;

                if (args[0] != NULL && execute_builtin(args)) {
                    continue;
                }

                pid_t pid = fork();
                if (pid == 0) {
                    if (input_file) {
                        int fd = open(input_file, O_RDONLY);
                        if (fd < 0) {
                            perror("Giriş dosyası açılamadı");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }

                    if (output_file) {
                        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) {
                            perror("Çıkış dosyası açılamadı");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }

                    execvp(args[0], args);
                    perror("execvp başarısız");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    if (!background) {
                        waitpid(pid, NULL, 0);
                    } else {
                        printf("[Arka planda çalışan PID: %d]\n", pid);
                    }
                } else {
                    perror("fork başarısız");
                }
            }
        }
    }

    return 0;
}
