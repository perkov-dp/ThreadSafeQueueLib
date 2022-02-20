#pragma once

#include <vector>
#include <deque>
#include <iostream>
#include <stdexcept>

#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

//───────────────────────────────────────────────────────────────────────────────────────────────
class Queue {
public:
   //───────────────────────────────────── Запись в очередь ───────────────────────────────
   void write_end(const vector<uint8_t>& data);
   void write_begin(const vector<uint8_t>& data);

   //──────────────────────────────────── Чтение из очереди ──────────────────────────────
   vector<uint8_t> read(uint8_t read_index);

   //──────────────────────────────────────── Изменение эл-та очереди  ─────────────────────────────
   vector<uint8_t> change_el(uint8_t index, const vector<uint8_t>& val);

   //──────────────────────────────────────── Определение размера очереди  ─────────────────────────────
   uint8_t get_queue_size();

   //────────────────────── Удаление сообщения из очереди ────────────────────────
   void erase_begin();
   void erase_end();

   //─────────────────────────────────────  Очистка очереди ─────────────────────────────────────
   void clear();

   //───────────────────────────────────── Доступ к очереди ─────────────────────────────────────
   deque <vector<uint8_t>> getQueue();
private:
   deque <vector<uint8_t>> messQueue;
   mutex m;
   condition_variable cv;
};
