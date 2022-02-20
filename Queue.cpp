#include "Queue.h"

//──────────────────────────────────────── Метод записи в конец очереди ────────────────────────────────
void Queue::write_end(const vector<uint8_t>& data) {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	messQueue.push_back(data);			// добавление в конец
	messQueue.back().shrink_to_fit();	//	подогнать размер строки под фактический размер
	messQueue.shrink_to_fit();			//	подогнать всю очередь под фактический размер

	//	оповещаем о добавлении нового элемента -> очередь непустая
	cv.notify_all();
};
//──────────────────────────────────────── Метод записи в начало очереди ────────────────────────────────
void Queue::write_begin(const vector<uint8_t>& data) {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	messQueue.push_front(data);			// добавление в начало
	messQueue.front().shrink_to_fit();	//	подогнать размер строки под фактический размер
	messQueue.shrink_to_fit();			//	подогнать всю очередь под фактический размер

	//	оповещаем о добавлении нового элемента -> очередь непустая
	cv.notify_all();
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
vector<uint8_t> Queue::read(uint8_t read_index) {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	//	пока очередь пустая -> ждем добавления элемента
	while(!getQueue().size()) {
		cv.wait(lg);
	}

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
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	//	пока очередь пустая -> ждем добавления элемента
	while(!getQueue().size()) {
		cv.wait(lg);
	}

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
uint8_t Queue::get_queue_size() {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	return messQueue.size();
}

//──────────────────────────── Удаление самого старого сообщения из очереди ──────────────────────
void Queue::erase_begin() {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	if (!messQueue.empty()) {
		messQueue.pop_front();
	}
};
//──────────────────────────── Удаление самого нового сообщения из очереди ──────────────────────
void Queue::erase_end() {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	if (!messQueue.empty()) {
		messQueue.pop_back();
	}
};


//──────────────────────────────────────────  Очистка очереди ────────────────────────────────────
void Queue::clear(void) {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	while(!messQueue.empty()) {
		messQueue.erase(messQueue.begin());
	}
};

//───────────────────────────────────── Доступ к очереди ─────────────────────────────────────
deque<vector<uint8_t>> Queue::getQueue(void) {
	// Отныне текущий поток единственный, который имеет доступ к rawQueue
	std::unique_lock<std::mutex> lg(m);

	return messQueue;
}

