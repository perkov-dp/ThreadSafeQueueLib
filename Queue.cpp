#include "Queue.h"

//────────────────────────────────────────── Конструктор ─────────────────────────────────────────
Queue::Queue(u_int size, REGIM_WRITE_QUEUE regim) :
		FULL_SIZE_DATA(size), regWrite(regim), currentSize(0),
		pointWrite(0), pointRead(0), freeSizeEnd(0), dataQueue(NULL)
{
	dataQueue = new u_char[FULL_SIZE_DATA];
};

//─────────────────────────────────────────── Деструктор ─────────────────────────────────────────
Queue::~Queue() {
	delete [] dataQueue;
};

//──────────────────────────────────────── Метод записи в очередь ────────────────────────────────
// При успешном выполнении возвращают EOK, иначе код ошибки
int Queue::write(const void* firstData, u_int firstDataSize,
		const void* secondData, u_int secondDataSize
){
	try {
		int ret = EOK;
		u_int fullSize = firstDataSize + secondDataSize + sizeof(u_int);
		if((firstDataSize && !firstData)
			|| (secondDataSize && !secondData)
			|| (!firstDataSize && !secondDataSize)
			|| (fullSize > FULL_SIZE_DATA)
		){
			throw EINVAL; // Некорректное значение параметров
		}
		int newFreeSizeEnd = 0;

		if(!freeSizeEnd && pointRead <= pointWrite && (pointWrite + fullSize) > FULL_SIZE_DATA)
		{
			newFreeSizeEnd = FULL_SIZE_DATA - pointWrite;
		}

		while(fullSize > ( FULL_SIZE_DATA - (currentSize + freeSizeEnd + newFreeSizeEnd)))
		{
			if(regWrite == NO_WRITE) {
				throw ENOMEM;
			}
			ret = ENOMEM;
			erase();
			if(!freeSizeEnd && pointRead <= pointWrite && (pointWrite + fullSize) > FULL_SIZE_DATA)
			{
				newFreeSizeEnd = FULL_SIZE_DATA - pointWrite;
			}
		}

		if(newFreeSizeEnd)
		{
			freeSizeEnd = newFreeSizeEnd;
			if(pointWrite == pointRead) {
				pointRead = freeSizeEnd = 0;
			}
			pointWrite = 0;
		}
		currentSize += fullSize;
		fullSize -= sizeof(u_int);
		memcpy(dataQueue + pointWrite, &fullSize, sizeof(u_int));
		pointWrite += sizeof(u_int);
		if(firstDataSize) {
			memcpy(dataQueue + pointWrite, firstData, firstDataSize);
			pointWrite += firstDataSize;
		}

		if(secondDataSize) {
			memcpy(dataQueue + pointWrite, secondData, secondDataSize);
			pointWrite += secondDataSize;
		}
		if(pointWrite >= FULL_SIZE_DATA) {
			pointWrite = 0;
		}
		return ret;
	}
	catch(int err) {
		return err;
	}
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
void* Queue::read(u_int& size, REGIM_READ_QUEUE regim)
{
	void* ptr = NULL;
	if(currentSize) {
		memcpy(&size, dataQueue + pointRead, sizeof(u_int));
    	ptr = dataQueue + pointRead + sizeof(u_int);
    	if(regim == READ_CLEAR) {
    		erase();
    	}
	}
	return ptr;
};

//──── Чтение из очереди - возвращает размер сообщения и копирует сообщение в буфер ──
int Queue::read(void* buf, u_int size, REGIM_READ_QUEUE regim) {
	int sizeRead = -1;
    if(size && buf) {
    	if(!currentSize) {
    		sizeRead = 0;
    	}
		else
		{
			u_int uSize;
			void* ptr = read(uSize, regim);
			if(uSize <= size)
			{
				sizeRead = uSize;
				memcpy(buf, ptr, sizeRead);
			}
			else {
				sizeRead = -1;
			}
		}
	}
    return sizeRead;
};

//──────────────────────────── Удаление самого старого сообщения из очереди ──────────────────────
int Queue::erase(void)
{
	if(currentSize) {
		u_int size;
		memcpy(&size, dataQueue + pointRead, sizeof(u_int));
		size += sizeof(u_int);
		pointRead += size;
		currentSize -= size;
		if((pointRead + freeSizeEnd) >= FULL_SIZE_DATA) {
			pointRead = freeSizeEnd = 0;
		}
		return 0;
	}
	return -1;
};

//──────────────────────────────────────────  Очистка очереди ────────────────────────────────────
void Queue::clear(void) {
	currentSize = pointRead = pointWrite = freeSizeEnd = 0;
};

//──────────────────────────────────── Запись в очередь ────────────────────────────────────
int QueueBlock::write(const void* firstData, u_int firstDataSize,
		const void* secondData, u_int secondDataSize
){
	QueueMutex mutex(this);
	int ret = Queue::write(firstData, firstDataSize, secondData, secondDataSize);
	CondvarSignal();
	return ret;
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
void* QueueBlock::read(u_int& size, REGIM_READ_QUEUE regim) {
	QueueMutex mutex(this);
	while(!currentSize) {
		CondvarWait();
	}

	return Queue::read(size, regim);
};

//──────────────────────────────────────── Чтение из очереди  ─────────────────────────────
int QueueBlock::read(void* buf, u_int size, REGIM_READ_QUEUE regim) {
	QueueMutex mutex(this);
	while(!currentSize) {
		CondvarWait();
	}

	return Queue::read(buf, size, regim);
};

//───────────────────────── Удаление сообщения из очереди ─────────────────────────
int QueueBlock::erase(void) {
	QueueMutex mutex(this);
	return Queue::erase();
};

//────────────────────────────────────────  Очистка очереди ──────────────────────────────────────
void QueueBlock::clear(void) {
	QueueMutex mutex(this);
	Queue::clear();
};


