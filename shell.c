#include "shell.h"

// Arka planda çalışan işlemler için sinyal işleyici
void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    // Tamamlanan işlemleri bekle ve durumu bildir
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("[pid %d] retval: %d\n", pid, WEXITSTATUS(status));
            fflush(stdout);
        } else if (WIFSIGNALED(status)) {
            printf("[pid %d] terminated by signal: %d\n", pid, WTERMSIG(status));
            fflush(stdout);
        }
    }
}

// SIGCHLD sinyali için işlem kurar
void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // Sinyal işleyiciyi belirle
    sigemptyset(&sa.sa_mask);        // Maske ayarla
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    // Sinyal işlemi ayarla
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

// Komutu argümanlara ayırır
int parse_command(char *cmd, char **args) {
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Argüman sonunu belirt
    return i;
}

// Giriş ve çıkış yönlendirmelerini işler
int handle_redirection(char **args) {
    int i = 0;
    while (args[i]) {
        if (strcmp(args[i], "<") == 0) { // Giriş yönlendirme
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Hata: Giriş dosyası belirtilmemiş\n");
                return -1;
            }
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("open");
                return -1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], ">") == 0) { // Çıkış yönlendirme
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Hata: Çıkış dosyası belirtilmemiş\n");
                return -1;
            }
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            args[i + 1] = NULL;
        }
        i++;
    }
    return 0;
}

// Sayı artırma komutunu çalıştırır
void increment_command() {
    int num;
    while (scanf("%d", &num) == 1) {
        printf("%d\n", num + 1); // Sayıyı bir artır ve yazdır
        fflush(stdout);
    }

    if (feof(stdin)) {
        clearerr(stdin); // EOF durumunu temizle
    } else {
        fprintf(stderr, "Hata: Giriş okunamadı\n");
    }

    // Standart giriş EOF sonrası yeniden ayarlanabilir
    freopen("/dev/tty", "r", stdin); // Konsoldan standart girişi geri yükle
}


// Tek bir komut çalıştırır
void execute_single_command(char *cmd, int background) {
    char *args[MAX_ARGS];
    int arg_count = parse_command(cmd, args);

    if (arg_count == 0) {
        return; // Boş komut
    }

    // Arka plan çalıştırma kontrolü
    if (background && strcmp(args[arg_count - 1], "&") == 0) {
        args[arg_count - 1] = NULL; // '&' işaretini kaldır
    }

    if (strcmp(args[0], "quit") == 0) { // Programı sonlandır
        while (waitpid(-1, NULL, 0) > 0);
        exit(0);
    }

    if (strcmp(args[0], "increment") == 0) { // 'increment' komutunu çalıştır
        if (handle_redirection(args) < 0) {
            fprintf(stderr, "Hata: Yönlendirme başarısız\n");
            return;
        }
        increment_command();
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (handle_redirection(args) < 0) {
            exit(1);
        }
        execvp(args[0], args); // Komutu çalıştır
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        if (background) {
            printf("[pid %d] arka planda çalışıyor\n", pid);
            fflush(stdout);
        } else {
            waitpid(pid, NULL, 0); // Ön planda bekle
            printf("> ");
            fflush(stdout);
        }
    } else {
        perror("fork");
    }
}

// Noktalı virgülle ayrılmış komutları işler
void process_semicolon_commands(char *cmd) {
    char *commands[MAX_ARGS];
    int num_commands = 0;

    char *token = strtok(cmd, ";");
    while (token) {
        commands[num_commands++] = token;
        token = strtok(NULL, ";");
    }

    for (int i = 0; i < num_commands; i++) {
        char *current_cmd = commands[i];
        int background = 0;

        // Komutun '&' ile bitip bitmediğini kontrol et
        size_t len = strlen(current_cmd);
        while (len > 0 && current_cmd[len - 1] == ' ') {
            current_cmd[--len] = '\0'; // Son boşlukları kaldır
        }
        if (len > 0 && current_cmd[len - 1] == '&') {
            background = 1;
            current_cmd[--len] = '\0';
        }

        execute_single_command(current_cmd, background);
    }
}

// Pipe ile komutları çalıştırır
void execute_piped_commands(char *cmd) {
    char *commands[MAX_ARGS];
    int num_commands = 0;

    char *token = strtok(cmd, "|");
    while (token) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    int input_fd = 0; // Standart giriş ile başla

    for (int i = 0; i < num_commands; i++) {
        int pipe_fds[2];
        pipe(pipe_fds);

        pid_t pid = fork();
        if (pid == 0) {
            dup2(input_fd, STDIN_FILENO);
            if (i < num_commands - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
            }
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            execute_single_command(commands[i], 0);
            exit(0);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
            close(pipe_fds[1]);
            input_fd = pipe_fds[0];
        } else {
            perror("fork");
        }
    }
    close(input_fd);
}

// Komutu çalıştırma fonksiyonu
void execute_command(char *cmd) {
    if (strchr(cmd, '|')) {
        execute_piped_commands(cmd);
    } else if (strchr(cmd, ';')) {
        process_semicolon_commands(cmd);
    } else {
        int background = 0;
        size_t len = strlen(cmd);
        while (len > 0 && cmd[len - 1] == ' ') {
            cmd[--len] = '\0';
        }
        if (len > 0 && cmd[len - 1] == '&') {
            background = 1;
            cmd[--len] = '\0';
        }
        execute_single_command(cmd, background);
    }
}

// Ana fonksiyon
int main() {
    char command[MAX_CMD_LENGTH];

    setup_signal_handling(); // Sinyal işleme kurulumu

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break; // EOF veya hata
        }
        command[strcspn(command, "\n")] = '\0'; // Satır sonunu kaldır

        execute_command(command); // Komutu çalıştır
    }

    return 0;
}

