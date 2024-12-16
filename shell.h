/*
Hüseyin Göbekli (G211210041) 2B 
Okan Başol (G211210083) 2B 
Şimal Ece Kazdal (G221210068) 2B 
Muhammed İrfan Bakar (G221210596) 2B 
Betül Kurt (G221210054) 2C  
*/

#ifndef SHELL_H
#define SHELL_H

// Gerekli Kütüphaneler
#include <stdio.h>   // Giriş/Çıkış işlemleri
#include <stdlib.h>  // Yardımcı fonksiyonlar
#include <unistd.h>  // POSIX sistem çağrıları
#include <string.h>  // String işlemleri
#include <fcntl.h>   // Dosya işlemleri
#include <sys/types.h> // Veri türleri
#include <sys/wait.h>  // Süreç kontrolü
#include <signal.h>  // Sinyal işleme

// Makrolar
#define MAX_ARGS 64              // Maksimum argüman sayısı
#define MAX_COMMAND_LEN 1024     // Maksimum komut uzunluğu
#define MAX_HISTORY 100          // Maksimum geçmiş komut sayısı
#define MAX_JOBS 100             // Maksimum arka plan iş sayısı
#define MAX_PIPES 10             // Maksimum boru hattı sayısı

// Veri Yapıları
typedef struct {
    pid_t pid;                  // Süreç kimliği
    char command[MAX_COMMAND_LEN]; // Komut
    int active;                 // İş durumu
} Job;

// Global Değişkenler
extern char history[MAX_HISTORY][MAX_COMMAND_LEN]; // Komut geçmişi
extern int history_count;                          // Geçmiş sayacı

extern Job jobs[MAX_JOBS];      // Arka plan işleri
extern int job_count;           // İş sayısı

// Fonksiyon Prototipleri
void handle_signal(int signal);                   // Sinyal işleme
void add_to_history(const char *command);         // Geçmişe komut ekleme
void print_history();                             // Komut geçmişini yazdırma
void print_jobs();                                // Arka plan işleri yazdırma
void wait_for_background_jobs();                  // Arka plan işleri bekleme
void execute_piped_commands(char *commands[], int num_pipes); // Pipe ile komut çalıştırma
void execute_multiple_commands(char *command);    // Birden fazla komut çalıştırma
void execute_command(char *command);              // Tek komut çalıştırma

#endif // SHELL_H
