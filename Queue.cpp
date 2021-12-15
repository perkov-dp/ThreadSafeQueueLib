#include "Queue.h"
#include <stdexcept>

//	оператор вывода для vector
template <class T>
ostream& operator << (ostream& os, const vector<T>& s) {
	os << "{";
	bool first = true;	//	вывод запятой только после первого эл-та
	for (const auto& x : s) {
		if (!first) {
			os << ", ";
		}
		first = false;
		os << static_cast<int>(x);
	}
	return os << "}";
}

//	оператор вывода для deque
template <class T>
ostream& operator << (ostream& os, const deque<T>& s) {
	os << "{";
	bool first = true;	//	вывод запятой только после первого эл-та
	for (const auto& x : s) {
		if (!first) {
			os << ", ";
		}
		first = false;
		os << x;
	}
	return os << "}";
}

//─────────────────────────────────────────── Деструктор ─────────────────────────────────────────
Queue::~Queue() {
	clear();
};

//──────────────────────────────────────── Метод записи в конец очереди ────────────────────────────────
void Queue::write_end(const vector<uint8_t>& data) {
	messQueue.push_back(data);			// добавление в конец
	messQueue.back().shrink_to_fit();	//	подогнать размер строки под фактический размер
	messQueue.shrink_to_fit();			//	подогнать всю очередь под фактический размер
};
//──────────────────────────────────────── Метод записи в начало очереди ────────────────────────────────
void Queue::write_begin(const vector<uint8_t>& data) {
	messQueue.push_front(data);			// добавление в начало
	messQueue.front().shrink_to_fit();	//	подогнать размер строки под фактический размер
	messQueue.shrink_to_fit();			//	подогнать всю очередь под фактический размер
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
vector<uint8_t> Queue::read(uint8_t read_index) {
    try {
    	//	безопасное обращение по индексу
        return messQueue.at(read_index);
    } catch (out_of_range const& exc) {
        cout << exc.what() << endl;
    }
    return {};
}

//──────────────────────────────────────── Изменение эл-та очереди  ─────────────────────────────
vector<uint8_t> Queue::change_el(uint8_t index, const vector<uint8_t>& val) {
    try {
    	//	безопасное обращение по индексу
        messQueue.at(index) = val;
        return messQueue.at(index);
    } catch (out_of_range const& exc) {
        cout << exc.what() << endl;
    }
    return {};
}

//──────────────────────────────────────── Определение размера очереди  ─────────────────────────────
uint8_t Queue::get_queue_size(void) {
	return messQueue.size();
}

//──────────────────────────── Удаление самого старого сообщения из очереди ──────────────────────
void Queue::erase_begin(void) {
	if (!messQueue.empty()) {
		messQueue.pop_front();
	}
};
//──────────────────────────── Удаление самого нового сообщения из очереди ──────────────────────
void Queue::erase_end(void) {
	if (!messQueue.empty()) {
		messQueue.pop_back();
	}
};


//──────────────────────────────────────────  Очистка очереди ────────────────────────────────────
void Queue::clear(void) {
	while(!messQueue.empty()) {
		messQueue.erase(messQueue.begin());
	}
};

//───────────────────────────────────── Доступ к очереди ─────────────────────────────────────
deque <vector<uint8_t>> Queue::getQueue(void) {
	return messQueue;
}


//──────────────────────────────────── Метод записи в конец очереди ────────────────────────────────────
void QueueBlock::write_end(const vector<uint8_t>& data) {
	QueueMutex mutex(this);
	Queue::write_end(data);
	CondvarSignal();
};
//──────────────────────────────────── Метод записи в начало очереди ────────────────────────────────────
void QueueBlock::write_begin(const vector<uint8_t>& data) {
	QueueMutex mutex(this);
	Queue::write_begin(data);
	CondvarSignal();
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
vector<uint8_t> QueueBlock::read(uint8_t read_index) {
	QueueMutex mutex(this);
	while(!Queue::getQueue().size()) {
		CondvarWait();
	}

	return Queue::read(read_index);
};

//──────────────────────────────────────── Изменение эл-та очереди  ─────────────────────────────
vector<uint8_t> QueueBlock::change_el(uint8_t index, const vector<uint8_t>& val) {
	QueueMutex mutex(this);
	return Queue::change_el(index, val);
}

//──────────────────────────────────────── Определение размера очереди  ─────────────────────────────
uint8_t QueueBlock::get_queue_size(void) {
	QueueMutex mutex(this);
	return Queue::get_queue_size();
}

//───────────────────────── Удаление сообщения из очереди ─────────────────────────
void QueueBlock::erase_begin(void) {
	QueueMutex mutex(this);
	Queue::erase_begin();
};
//───────────────────────── Удаление сообщения из очереди ─────────────────────────
void QueueBlock::erase_end(void) {
	QueueMutex mutex(this);
	Queue::erase_end();
};


//────────────────────────────────────────  Очистка очереди ──────────────────────────────────────
void QueueBlock::clear(void) {
	QueueMutex mutex(this);
	Queue::clear();
};


