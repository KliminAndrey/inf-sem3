/* Создать программу, запускаемую на N терминалах, для обмена сообщениями между терминалами через файл.
 Каждый экземпляр программы, запущенный на каждом терминале содержит два процесса:
 один записывает файл, второй читает. Так на двух терминалах работает четыре процесса,
 обеспечивая двусторонний обмен. 
 Идентификатор пользователя (терминала), можно задавать параметром при запуске приложения.
 Предусмотреть хранение информации для отсутствующего получателя.
 Начать разработку со схемы данных. Предусмотреть поля для идентификаторов, отправителя и получателя,
а так же данных.
Разработать протокол обмена и структуру файла.
Продумать протокол очистки файла.
Исследовать вопрос с уникальностью идентификаторов.
*/
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME "mailbox.dat"
#define TEMPFILE "mailbox.tmp"
#define MSG_LEN  60

struct Message {
    int msg_id;           // ID сообщения
    int sender_id;        // ID отправителя
    int receiver_id;      // ID получателя
    int status;           // 0 - не прочитано, 1 - прочитано
    time_t timestamp;     // Время отправки
    char text[MSG_LEN];
};

void writer_process(int my_id) {
    int receiver;
    char text[MSG_LEN];
    static int msg_counter = 1;
    printf("[Writer %d] Введите ID получателя: \n", my_id);
    
    while (1) {
        printf("ID получателя: ");
        if (scanf("%d", &receiver) != 1) break;
        if (receiver == 0) exit(0); 
        
        getchar(); 
        
        printf("Текст: ");
        fgets(text, MSG_LEN, stdin);
        text[strcspn(text, "\n")] = '\0';

        int fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd < 0) {
            printf("Can't open file for writing\n");
            exit(-1);
        }

        struct Message msg;
        msg.msg_id = (int)(time(NULL) ^ getpid() ^ msg_counter++); // уникальный id
        msg.sender_id = my_id;
        msg.receiver_id = receiver;
        msg.status = 0;
        msg.timestamp = time(NULL);
        strncpy(msg.text, text, MSG_LEN - 1);
        msg.text[MSG_LEN - 1] = '\0';

        write(fd, &msg, sizeof(struct Message));
        
        close(fd);
    }
}

void reader_process(int my_id) {
    while (1) {
        int fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
            sleep(1);
            continue;
        }

        struct Message msg;
        
        while (read(fd, &msg, sizeof(struct Message)) == sizeof(struct Message)) {
            if (msg.receiver_id == my_id && msg.status == 0) {
                char time_buf[30];
                strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&msg.timestamp));
                
                printf("\n\033[32m[Входящее #%d от %d в %s]: %s\033[0m\n", 
                       msg.msg_id, msg.sender_id, time_buf, msg.text);
                
                msg.status = 1; 
                lseek(fd, -sizeof(struct Message), SEEK_CUR);
                write(fd, &msg, sizeof(struct Message));
                lseek(fd, 0, SEEK_CUR);
            }
        }

        int fd_temp = open(TEMPFILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        lseek(fd, 0, SEEK_SET); 

        while (read(fd, &msg, sizeof(struct Message)) == sizeof(struct Message)) {
            if (msg.status == 0) {
                write(fd_temp, &msg, sizeof(struct Message));
            }
        }

        close(fd_temp);
        close(fd); 
        
        rename(TEMPFILE, FILENAME);
 
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <terminal_id>\n", argv[0]);
        exit(-1);
    }

    int my_id = atoi(argv[1]);
    if (my_id <= 0) {
        printf("ID число > 0!\n");
        exit(-1);
    }

    printf("Терминал %d (PID: %d) запускается...\n", my_id, (int)getpid());

    pid_t pid_writer = fork();
    if (pid_writer < 0) {
        printf("Can't fork writer\n");
        exit(-1);
    } else if (pid_writer == 0) {
        writer_process(my_id);
    }

    pid_t pid_reader = fork();
    if (pid_reader < 0) {
        printf("Can't fork reader\n");
        exit(-1);
    } else if (pid_reader == 0) {
        reader_process(my_id);
    }

    while (1) {
        sleep(10);
    }

    return 0;
}