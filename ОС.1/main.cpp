#include "Header.h"
#include <iostream>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    unsigned char memory[MEMORY_SIZE / 8];

    initMemory(memory);

    int choice;

    do {
        cout << "\n МЕНЮ \n";
        cout << "1. Выделить память\n";
        cout << "2. Освободить память\n";
        cout << "3. Показать состояние памяти\n";
        cout << "0. Выход\n";

        choice = getNumber("Ваш выбор: ", 0, 3);

        switch (choice) {
        case 1: {
            int size = getNumber("Введите размер выделяемой памяти (1-1024): ", 1, MEMORY_SIZE);
            int address = allocateMemory(memory, size);

            if (address == -1) {
                cout << "\n[!] ОШИБКА: Недостаточно памяти для выделения "
                    << size << " байт.\n";
            }
            else {
                cout << "\n[+] Успешно выделено " << size
                    << " байт по адресу: " << address << "\n";
            }
            break;
        }

        case 2: {
            int address = getNumber("Введите начальный адрес (0-1023): ", 0, MEMORY_SIZE - 1);
            int size = getNumber("Введите размер освобождаемого участка: ", 1, MEMORY_SIZE);

            if (freeMemory(memory, address, size)) {
                cout << "\nУчасток по адресу " << address
                    << " размером " << size << " байт освобождён.\n";
            }
            else {
                cout << "\nОШИБКА: Не удалось освободить участок.\n";
                cout << "    Проверьте адрес и размер (возможно, часть участка свободна).\n";
            }
            break;
        }

        case 3: {
            printMemoryInfo(memory);
            break;
        }

        case 0: {
            cout << "\nВыход из программы.\n";
            break;
        }
        }
    } while (choice != 0);

    return 0;
}