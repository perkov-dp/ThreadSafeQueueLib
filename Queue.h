//    ОПИСАНИЕ КЛАССОВ ОЧЕРЕДЕЙ

#pragma once

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

class MutexAndCondvar {
protected:
	pthread_mutex_t Mutex;
	pthread_cond_t  CondVar;
public:
    MutexAndCondvar(){ init_class(); };
    virtual ~MutexAndCondvar(){ destroy_class(); };
    void init_class() {
    	pthread_mutexattr_t mutex_attr;
    	pthread_condattr_t  cond_attr;
    	pthread_mutexattr_init(&mutex_attr);
    	pthread_condattr_init (&cond_attr );
    	pthread_mutex_init(&Mutex,   &mutex_attr);
    	pthread_cond_init (&CondVar, &cond_attr );
    	pthread_mutexattr_destroy(&mutex_attr);
    	pthread_condattr_destroy(&cond_attr);
    };
    void destroy_class(void) {
    	pthread_mutex_destroy(&Mutex);
    	pthread_cond_destroy (&CondVar);
    };
    int MutexLock(void)        { return pthread_mutex_lock(&Mutex);          };
    int MutexTrulock(void)     { return pthread_mutex_trylock(&Mutex);       };
    int MutexUnlock(void)      { return pthread_mutex_unlock(&Mutex);        };
    int CondvarWait(void)      { return pthread_cond_wait(&CondVar, &Mutex); };
    int CondvarSignal(void)    { return pthread_cond_signal(&CondVar);       };
    int CondvarBroadcast(void) { return pthread_cond_broadcast(&CondVar);    };
};

// Режим записи в очередь при отсуствии свободного места
enum REGIM_WRITE_QUEUE {
	NO_WRITE,  // Отказ при попытке записи (default)
	RE_WRITE   // Запись нового сообщения со стиранием необходимого кол-ва самых старых
};

 // Режим чтения из очереди
enum REGIM_READ_QUEUE {
	READ_CLEAR, // Чтение сообщения с удалением
	READ_ONLY   // Чтение сообщения без удаления
};

 //───────────────────────────────────────────────────────────────────────────────────────────────
class Queue {
protected:
    const u_int FULL_SIZE_DATA;       // Полный размер данных в очереди
    const REGIM_WRITE_QUEUE regWrite; // Режим записи в очередь при отсуствии свободного места
    u_int currentSize;                // Текущий размер данных в очереди
    u_int pointWrite;                 // Точка записи в очередь
    u_int pointRead;                  // Точка чтения из очереди
    u_int freeSizeEnd;                // Размер блока данных, свободного в конце очереди
    u_char* dataQueue;                // Указатель на данные очереди
public:
    //────────────────────────────────────────── Конструктор ─────────────────────────────────────
    Queue(u_int size, REGIM_WRITE_QUEUE regim = NO_WRITE);
    //────────────────────────────────────────── Деструктор ──────────────────────────────────────
    virtual ~Queue();
    //───────────────────────────────────── Запись в очередь ───────────────────────────────
    int write(const void* firstData, u_int firstDataSize,
    		const void* secondData = NULL, u_int secondDataSize = 0);
    //──────────────────────────────────── Чтение из очереди ──────────────────────────────
    void* read(u_int& size, REGIM_READ_QUEUE regim = READ_CLEAR);
    int read(void* buf, u_int size, REGIM_READ_QUEUE regim = READ_CLEAR);
    //────────────────────── Удаление сообщения из очереди ────────────────────────
    int erase(void);
    //─────────────────────────────────────  Очистка очереди ─────────────────────────────────────
    void clear(void);
    u_int getFullSize(void)  {return FULL_SIZE_DATA; };
    u_int getCurrentSize(void) {return currentSize; };
};

// очередь с блокировкой
class QueueBlock : public Queue, protected MutexAndCondvar {
	friend class QueueMutex;
public:
    //────────────────────────────────────────── Конструктор ─────────────────────────────────────
    QueueBlock(u_int size, REGIM_WRITE_QUEUE regim = NO_WRITE): Queue(size, regim) {};
    //──────────────────────────────────────── Деструктор ────────────────────────────────────────
    virtual ~QueueBlock(){};
    //───────────────────────────────────── Запись в очередь ───────────────────────────────
    int write(const void* firstData, u_int firstDataSize,
    		const void* secondData = NULL, u_int secondDataSize = 0);
    //──────────────────────────────────── Чтение из очереди ──────────────────────────────
    int read(void* buf, u_int size, REGIM_READ_QUEUE regim = READ_CLEAR);
    void *read(u_int& size, REGIM_READ_QUEUE regim = READ_CLEAR);
    //────────────────────── Удаление сообщения из очереди ────────────────────────
    int erase(void);
    //─────────────────────────────────────  Очистка очереди ─────────────────────────────────────
    void clear(void);
};

// Блокирование мутекса при создание локального объекта QueueMutex в функции
// и разблокирование при выходе из функции (автоматически вызывается деструктор)
class QueueMutex {
public:
	QueueMutex(QueueBlock *q): queue(q){ queue->MutexLock(); }
	~QueueMutex(){queue->MutexUnlock(); }
private:
	QueueBlock *queue;
};
