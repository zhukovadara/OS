#include "Header.h"
#include <iostream>
#include <climits>

using namespace std;

int getBit(unsigned char* memory, int index) {
    int byteIndex = index / 8;
    int bitIndex = index % 8;
    return (memory[byteIndex] >> bitIndex) & 1;
}

void setBit(unsigned char* memory, int index, bool value) {
    int byteIndex = index / 8;
    int bitIndex = index % 8;
    if (value) {
        memory[byteIndex] |= (1 << bitIndex);
    }
    else {
        memory[byteIndex] &= ~(1 << bitIndex);
    }
}

bool isBitSet(unsigned char* memory, int index) {
    return getBit(memory, index) == 1;
}

void initMemory(unsigned char* memory) {
    int bytesNeeded = MEMORY_SIZE / 8;
    for (int i = 0; i < bytesNeeded; i++) {
        memory[i] = 0;
    }
}

int allocateMemory(unsigned char* memory, int size) {
    if (size <= 0 || size > MEMORY_SIZE) {
        return -1;
    }

    int bestStart = -1;
    int bestSize = INT_MAX;

    int currentStart = -1; 
    int currentSize = 0;

    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (!isBitSet(memory, i)) {
            if (currentStart == -1) {
                currentStart = i;
            }
            currentSize++;
        }
        else {
            if (currentStart != -1 && currentSize >= size) {
                if (currentSize < bestSize) {
                    bestSize = currentSize;
                    bestStart = currentStart;
                }
            }
            currentStart = -1;
            currentSize = 0;
        }
    }

    if (currentStart != -1 && currentSize >= size) {
        if (currentSize < bestSize) {
            bestSize = currentSize;
            bestStart = currentStart;
        }
    }

    if (bestStart != -1) {
        for (int i = bestStart; i < bestStart + size; i++) {
            setBit(memory, i, true);
        }
        return bestStart;
    }

    return -1;
}

bool freeMemory(unsigned char* memory, int address, int size) {
    if (address < 0 || size <= 0 || address + size > MEMORY_SIZE) {
        return false;
    }

    for (int i = address; i < address + size; i++) {
        if (!isBitSet(memory, i)) {
            return false;
        }
    }

    for (int i = address; i < address + size; i++) {
        setBit(memory, i, false);
    }

    return true;
}

void printMemoryInfo(unsigned char* memory) {
    cout << "\n Информация о памяти \n";
    cout << "Общий размер памяти: " << MEMORY_SIZE << " байт\n\n";

    int freeBlocks = 0;
    int busyBlocks = 0;
    int freeBytes = 0; 
    int busyBytes = 0;

    bool inBlock = false;
    bool isFree = false;
    int blockStart = 0;
    int blockSize = 0;

    for (int i = 0; i < MEMORY_SIZE; i++) {
        bool current = !isBitSet(memory, i);

        if (!inBlock) {
            inBlock = true;
            isFree = current;
            blockStart = i;
            blockSize = 1;
        }
        else if (current == isFree) {
            blockSize++;
        }
        else {
            if (isFree) {
                freeBlocks++;
                freeBytes += blockSize;
            }
            else {
                busyBlocks++;
                busyBytes += blockSize;
            }

            isFree = current;
            blockStart = i;
            blockSize = 1;
        }
    }

    if (inBlock) {
        if (isFree) {
            freeBlocks++;
            freeBytes += blockSize;
        }
        else {
            busyBlocks++;
            busyBytes += blockSize;
        }
    }

    cout << "Свободно байт: " << freeBytes << "\n";
    cout << "Занято байт: " << busyBytes << "\n";
    cout << "Свободных участков: " << freeBlocks << "\n";
    cout << "Занятых участков: " << busyBlocks << "\n";

    cout << "\n Список всех участков \n";

    inBlock = false;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        bool current = !isBitSet(memory, i);

        if (!inBlock) {
            inBlock = true;
            isFree = current;
            blockStart = i;
            blockSize = 1;
        }
        else if (current == isFree) {
            blockSize++;
        }
        else {
            cout << (isFree ? "[СВОБОДЕН] " : "[ЗАНЯТ]    ")
                << "адрес: " << blockStart
                << ", размер: " << blockSize << " байт\n";

            isFree = current;
            blockStart = i;
            blockSize = 1;
        }
    }

    if (inBlock) {
        cout << (isFree ? "[СВОБОДЕН] " : "[ЗАНЯТ]    ")
            << "адрес: " << blockStart
            << ", размер: " << blockSize << " байт\n";
    }
}

int getNumber(const string& prompt, int minVal, int maxVal) {
    int num;
    bool ok;

    do {
        cout << prompt;
        cin >> num;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Ошибка! Введите целое число.\n";
            ok = false;
        }
        else if (num < minVal || num > maxVal) {
            cout << "Ошибка! Значение должно быть от " << minVal << " до " << maxVal << ".\n";
            ok = false;
        }
        else {
            ok = true;
        }
    } while (!ok);

    cin.ignore(10000, '\n');
    return num;
}