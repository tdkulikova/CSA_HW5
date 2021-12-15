#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <chrono>

// количество комнат в отеле
const int rooms_amount = 30;

// отель
int hotel[rooms_amount];

// индекс для поселения в отель
int in_room_number = 0;

// индекс для выселения из отеля
int out_room_number = 0;

// переменная, по которой будет осуществляться выход из цикла
int iterations = 0;

// семафор, отображающий насколько  отель пуст (кол-во свободных комнат)
sem_t empty;

// семафор, отображающий насколько полон отель (кол-во занятых комнат)
sem_t full;

// мьютекс для операции поселения
pthread_mutex_t mutex_in;

//мутекс для операции выселения
pthread_mutex_t mutex_out;

// инициализатор генератора случайных чисел
unsigned int seed = 101;

//стартовая функция потоков – посетителей
void *visitor(void *visitor_param) {
    int visitor_number = *((int *) visitor_param);
    while (iterations < 10000) {
        // защита операции поселения
        pthread_mutex_lock(&mutex_in);
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time);
        // получение текущего времени
        time_info = localtime(&raw_time);
        ++iterations;
        // количество свободных комнат уменьшается на единицу
        sem_wait(&empty);
        // поселение посетителя в отель
        hotel[in_room_number] = visitor_number;
        int in_number = in_room_number + 1;
        in_room_number = (in_room_number + 1) % rooms_amount;
        // количество занятых комнат увеличилось на единицу
        sem_post(&full);
        printf("Visitor number %d goes to room number [%d] at %s", visitor_number,
               in_number, asctime(time_info));
        pthread_mutex_unlock(&mutex_in);
        sleep(6);
    }
    return nullptr;
}

// стартовая функция потоков – комнат (выселение посетителя из комнаты)
void *room(void *param) {
    int visitor_number;
    while (iterations < 10000) {
        sleep(6);
        // защита операции выселения
        pthread_mutex_lock(&mutex_out);
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time);
        time_info = localtime(&raw_time);
        // количество занятых ячеек уменьшается на единицу
        sem_wait(&full);
        visitor_number = hotel[out_room_number];
        hotel[out_room_number] = 0;
        int out_number = out_room_number + 1;
        out_room_number = (out_room_number + 1) % rooms_amount;
        // количество свободных ячеек увеличивается на единицу
        sem_post(&empty);
        printf("Room number %d: visitor number [%d] goes out at %s", out_number, visitor_number, asctime(time_info));
        pthread_mutex_unlock(&mutex_out);
    }
    return nullptr;
}

int main(int argc, char *argv[]) {
    srand(seed);
    if (argc != 2) {
        printf("Incorrect command argument\n");
        exit(0);
    }
    // инициализация мьютексов и семафоров
    pthread_mutex_init(&mutex_in, nullptr);
    pthread_mutex_init(&mutex_out, nullptr);
    //количество свободных комнат равно количеству комнат
    sem_init(&empty, 0, rooms_amount);
    // количество занятых комнат равно 0
    sem_init(&full, 0, 0);
    int64_t visitors_amount;
    visitors_amount = strtol(argv[1], nullptr, 0);
    if (visitors_amount <= 0 || visitors_amount > 100) {
        printf("Incorrect command argument\n");
        exit(0);
    }
    pthread_t visitor_threads[visitors_amount];
    int visitors[visitors_amount];
    // запуск потоков-посетителей
    for (int i = 0; i < visitors_amount; ++i) {
        visitors[i] = i + 1;
        pthread_create(&visitor_threads[i], nullptr, visitor, (void *) (visitors + i));
    }
    // запуск потоков-комнат
    pthread_t room_threads[30];
    int rooms[30];
    for (int i = 0; i < 30; ++i) {
        rooms[i] = i + 1;
        pthread_create(&room_threads[i], nullptr, room, (void *) (rooms + i));
    }
    // главный поток - тоже комната
    int mNum = 0;
    room((void *) &mNum);
    return 0;
}

