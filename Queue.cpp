#include "Queue.h"

//────────────────────────────────────────── Конструктор ─────────────────────────────────────────
Queue::Queue(uint8_t size)
{
	//	выделение памяти под массив размером FULL_SIZE_DATA
	messQueue.resize(size);
};

//─────────────────────────────────────────── Деструктор ─────────────────────────────────────────
Queue::~Queue() {
	messQueue.erase(messQueue.begin(), messQueue.end());
};

//──────────────────────────────────────── Метод записи в очередь ────────────────────────────────
// При успешном выполнении возвращают EOK, иначе код ошибки
void Queue::write(const vector<uint8_t>& data) {
	messQueue.push_back(data);
	messQueue.front().shrink_to_fit();
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
vector<uint8_t> Queue::read(uint8_t read_index) {
	if (read_index < messQueue.size()) {
		return messQueue[read_index];
	}
	return {127};
}

//──────────────────────────── Удаление самого старого сообщения из очереди ──────────────────────
void Queue::erase(void) {
	if (!messQueue.empty()) {
		messQueue.pop_front();
	}
};

//──────────────────────────────────────────  Очистка очереди ────────────────────────────────────
void Queue::clear(void) {
	while(!messQueue.empty()) {
		messQueue.erase(messQueue.begin());
	}
};

//──────────────────────────────────── Запись в очередь ────────────────────────────────────
void QueueBlock::write(const vector<uint8_t>& data) {
	QueueMutex mutex(this);
	Queue::write(data);
	CondvarSignal();
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
vector<uint8_t> QueueBlock::read(uint8_t read_index) {
	QueueMutex mutex(this);
	while(!messQueue.size()) {
		CondvarWait();
	}

	return Queue::read(read_index);
};

//───────────────────────── Удаление сообщения из очереди ─────────────────────────
void QueueBlock::erase(void) {
	QueueMutex mutex(this);
	Queue::erase();
};

//────────────────────────────────────────  Очистка очереди ──────────────────────────────────────
void QueueBlock::clear(void) {
	QueueMutex mutex(this);
	Queue::clear();
};


