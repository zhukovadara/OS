//Алгоритм выделения памяти: наименее подходящий
//Способ хранения информации: битовая карта
#ifndef HEADER_H
#define HEADER_H

#include <string>

using namespace std;

const int MEMORY_SIZE = 1024;

void initMemory(unsigned char* memory);

int allocateMemory(unsigned char* memory, int size);

bool freeMemory(unsigned char* memory, int address, int size);

void printMemoryInfo(unsigned char* memory);

bool isBitSet(unsigned char* memory, int index);
void setBit(unsigned char* memory, int index, bool value);
int getBit(unsigned char* memory, int index);

int getNumber(const string& prompt, int minVal, int maxVal);

#endif