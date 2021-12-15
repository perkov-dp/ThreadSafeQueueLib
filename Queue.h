//    ОПИСАНИЕ КЛАССОВ ОЧЕРЕДЕЙ

#pragma once

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <cstdint>	//	uint8_t
#include <vector>
#include <deque>
#include <iostream>
using namespace std;

//	оператор вывода для vector
template <class T>
ostream& operator << (ostream& os, const vector<T>& s);

//	оператор вывода для deque
template <class T>
ostream& operator << (ostream& os, const deque<T>& s);

using u_int = unsigned int;

class MutexAndCondvar {
private:
	pthread_mutex_t Mutex;
	pthread_cond_t  CondVar;

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
    int MutexTrulock(void) {
    	return pthread_mutex_trylock(&Mutex);
    };
    int CondvarBroadcast(void) {
    	return pthread_cond_broadcast(&CondVar);
    };
public:
    virtual ~MutexAndCondvar(){ destroy_class(); };
    int MutexLock(void) {
    	return pthread_mutex_lock(&Mutex);
    };
    int MutexUnlock(void) {
    	return pthread_mutex_unlock(&Mutex);
    };
    int CondvarWait(void) {
    	return pthread_cond_wait(&CondVar, &Mutex);
    };
    int CondvarSignal(void) {
    	return pthread_cond_signal(&CondVar);
    };
};

 //───────────────────────────────────────────────────────────────────────────────────────────────
class Queue {
public:
    //────────────────────────────────────────── Деструктор ──────────────────────────────────────
    virtual ~Queue();
    //───────────────────────────────────── Запись в очередь ───────────────────────────────
    void write_end(const vector<uint8_t>& data);
    void write_begin(const vector<uint8_t>& data);
    //──────────────────────────────────── Чтение из очереди ──────────────────────────────
    vector<uint8_t> read(uint8_t read_index);
    //──────────────────────────────────────── Изменение эл-та очереди  ─────────────────────────────
    vector<uint8_t> change_el(uint8_t index, const vector<uint8_t>& val);
    //──────────────────────────────────────── Определение размера очереди  ─────────────────────────────
    uint8_t get_queue_size(void);
    //────────────────────── Удаление сообщения из очереди ────────────────────────
    void erase_begin(void);
    void erase_end(void);
    //─────────────────────────────────────  Очистка очереди ─────────────────────────────────────
    void clear(void);

    //───────────────────────────────────── Доступ к очереди ─────────────────────────────────────
    deque <vector<uint8_t>> getQueue(void);
private:
    deque <vector<uint8_t>> messQueue;
};

// очередь с блокировкой
class QueueBlock : public Queue, protected MutexAndCondvar {
	friend class QueueMutex;
public:
    //──────────────────────────────────────── Деструктор ────────────────────────────────────────
    virtual ~QueueBlock(){};
    //───────────────────────────────────── Запись в очередь ───────────────────────────────
    void write_end(const vector<uint8_t>& data);
    void write_begin(const vector<uint8_t>& data);
    //──────────────────────────────────── Чтение из очереди ──────────────────────────────
    vector<uint8_t> read(uint8_t read_index);
    //──────────────────────────────────────── Изменение эл-та очереди  ─────────────────────────────
    vector<uint8_t> change_el(uint8_t index, const vector<uint8_t>& val);
    //──────────────────────────────────────── Определение размера очереди  ─────────────────────────────
    uint8_t get_queue_size(void);
    //────────────────────── Удаление сообщения из очереди ────────────────────────
    void erase_begin(void);
    void erase_end(void);
    //─────────────────────────────────────  Очистка очереди ─────────────────────────────────────
    void clear(void);
};

// Блокирование мутекса при создание локального объекта QueueMutex в функции
// и разблокирование при выходе из функции (автоматически вызывается деструктор)
class QueueMutex {
public:
	QueueMutex(QueueBlock *q): queue(q) {queue->MutexLock();}
	~QueueMutex() {queue->MutexUnlock();}
private:
	QueueBlock *queue;
};
