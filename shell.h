/**
 * @file        Shell.h
 * @description Bu dosya, Linux işletim sistemi üzerinde çalışan temel bir kabuk (shell) uygulamasının 
 *              işlevlerini tanımlamaktadır. Fonksiyonlar arasında komut istemi görüntüleme, tekli komut 
 *              yürütme, giriş/çıkış yönlendirme, arka plan işlemleri ve boru (pipe) işlemleri yer almaktadır.
 * @course      İşletim Sistemleri
 * @assignment  Proje Ödevi
 * @date        26 Aralık 2024
 * @author      Awab KHALEFA - awab.daw@yahoo.com
 * @note        Bu projeyi, grup arkadaşlarımı bulamadığım için tek başıma tamamladım.
 * @github      https://github.com/Aldlioo/os_project
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Komutların maksimum uzunluğu
#define MAX_CMD_LENGTH 1024

// Maksimum argüman sayısı
#define MAX_ARGS 100

// Fonksiyon prototipleri
void sigchld_handler(int sig); // Arka plan işlemleri için sinyal işleyici
void setup_signal_handling();  // Sinyal işleme kurulumu
int parse_command(char *cmd, char **args); // Komutu argümanlara ayırır
int handle_redirection(char **args); // Giriş/çıkış yönlendirmesini işler
void increment_command(); // Sayı artırma komutunu çalıştırır
void execute_single_command(char *cmd, int background); // Tek komutu çalıştırır
void process_semicolon_commands(char *cmd); // Noktalı virgülle ayrılmış komutları işler
void execute_piped_commands(char *cmd); // Pipe ile komutları çalıştırır
void execute_command(char *cmd); // Genel komut çalıştırma fonksiyonu

#endif 

