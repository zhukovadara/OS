#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <locale>

using namespace std;

const int PAGE_SIZE = 32;
const int MEM_SIZE = 1024;
const int NUM_PAGES_RAM = MEM_SIZE / PAGE_SIZE;
const int NUM_PAGES_DISK = MEM_SIZE / PAGE_SIZE;

unsigned char RAM[MEM_SIZE];
unsigned char DISK[MEM_SIZE];

inline int get_vpage_id(int page_idx) {
    return RAM[page_idx * PAGE_SIZE + 0];
}

inline void set_vpage_id(int page_idx, int id) {
    RAM[page_idx * PAGE_SIZE + 0] = static_cast<unsigned char>(id);
}

inline bool is_valid(int page_idx) {
    return (RAM[page_idx * PAGE_SIZE + 1] & 0x04) != 0;
}

inline void set_valid(int page_idx, bool valid) {
    if (valid) RAM[page_idx * PAGE_SIZE + 1] |= 0x04;
    else RAM[page_idx * PAGE_SIZE + 1] &= ~0x04;
}

inline bool get_bit_R(int page_idx) {
    return (RAM[page_idx * PAGE_SIZE + 1] & 0x01) != 0;
}

inline void set_bit_R(int page_idx, bool r) {
    if (r) RAM[page_idx * PAGE_SIZE + 1] |= 0x01;
    else RAM[page_idx * PAGE_SIZE + 1] &= ~0x01;
}

inline bool get_bit_M(int page_idx) {
    return (RAM[page_idx * PAGE_SIZE + 1] & 0x02) != 0;
}

inline void set_bit_M(int page_idx, bool m) {
    if (m) RAM[page_idx * PAGE_SIZE + 1] |= 0x02;
    else RAM[page_idx * PAGE_SIZE + 1] &= ~0x02;
}

void init_memory() {
    memset(RAM, 0, MEM_SIZE);
    memset(DISK, 0, MEM_SIZE);
    for (int i = 0; i < NUM_PAGES_DISK; ++i) {
        for (int j = 0; j < PAGE_SIZE; ++j) {
            DISK[i * PAGE_SIZE + j] = static_cast<unsigned char>((i * 10 + j) % 256);
        }
    }
}

void reset_reference_bits() {
    for (int i = 0; i < NUM_PAGES_RAM; ++i) {
        if (is_valid(i)) {
            set_bit_R(i, false);
        }
    }
}

int find_page_in_ram(int vpage_id) {
    for (int i = 0; i < NUM_PAGES_RAM; ++i) {
        if (is_valid(i) && get_vpage_id(i) == vpage_id) {
            return i;
        }
    }
    return -1;
}

int select_victim_nru() {
    for (int cls = 0; cls < 4; ++cls) {
        int indices[NUM_PAGES_RAM];
        int count = 0;
        for (int i = 0; i < NUM_PAGES_RAM; ++i) {
            if (!is_valid(i)) continue;
            bool r = get_bit_R(i);
            bool m = get_bit_M(i);
            int current_class = (r ? 2 : 0) + (m ? 1 : 0);
            if (current_class == cls) {
                indices[count++] = i;
            }
        }
        if (count > 0) {
            int rand_idx = rand() % count;
            return indices[rand_idx];
        }
    }
    return -1;
}

void swap_page(int vpage_id_to_load) {
    int free_slot = -1;
    for (int i = 0; i < NUM_PAGES_RAM; ++i) {
        if (!is_valid(i)) {
            free_slot = i;
            break;
        }
    }
    int victim_slot = -1;
    if (free_slot == -1) {
        cout << "RAM полна. Запуск алгоритма NRU для выбора жертвы..." << endl;
        victim_slot = select_victim_nru();
        if (victim_slot == -1) {
            cerr << "Критическая ошибка: не удалось найти страницу для выгрузки." << endl;
            return;
        }
        int victim_vpage = get_vpage_id(victim_slot);
        cout << "Выгружаем виртуальную страницу " << (int)victim_vpage
            << " из слота RAM[" << victim_slot << "] на диск." << endl;
        if (victim_vpage < NUM_PAGES_DISK) {
            memcpy(DISK + victim_vpage * PAGE_SIZE,
                RAM + victim_slot * PAGE_SIZE + 4,
                PAGE_SIZE - 4);
            set_bit_M(victim_slot, false);
        }
        set_valid(victim_slot, false);
        free_slot = victim_slot;
    }
    cout << "Загружаем виртуальную страницу " << vpage_id_to_load
        << " с диска в слот RAM[" << free_slot << "]." << endl;
    if (vpage_id_to_load < NUM_PAGES_DISK) {
        memcpy(RAM + free_slot * PAGE_SIZE + 4,
            DISK + vpage_id_to_load * PAGE_SIZE,
            PAGE_SIZE - 4);
    }
    else {
        memset(RAM + free_slot * PAGE_SIZE + 4, 0, PAGE_SIZE - 4);
    }
    set_vpage_id(free_slot, vpage_id_to_load);
    set_valid(free_slot, true);
    set_bit_R(free_slot, true);
    set_bit_M(free_slot, false);
}

unsigned char read_byte(int vaddr) {
    int vpage_id = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;
    reset_reference_bits();
    int slot = find_page_in_ram(vpage_id);
    if (slot == -1) {
        cout << "Page Fault: Страница " << vpage_id << " не в RAM. Выполняем подкачку." << endl;
        swap_page(vpage_id);
        slot = find_page_in_ram(vpage_id);
    }
    set_bit_R(slot, true);
    unsigned char val = RAM[slot * PAGE_SIZE + 4 + offset];
    cout << "Чтение адреса " << vaddr << " (стр. " << vpage_id
        << ", смещ. " << offset << ") -> значение: " << (int)val << endl;
    return val;
}

void write_byte(int vaddr, unsigned char value) {
    int vpage_id = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;
    reset_reference_bits();
    int slot = find_page_in_ram(vpage_id);
    if (slot == -1) {
        cout << "Page Fault: Страница " << vpage_id << " не в RAM. Выполняем подкачку." << endl;
        swap_page(vpage_id);
        slot = find_page_in_ram(vpage_id);
    }
    set_bit_R(slot, true);
    set_bit_M(slot, true);
    RAM[slot * PAGE_SIZE + 4 + offset] = value;
    cout << "Запись адреса " << vaddr << " (стр. " << vpage_id
        << ", смещ. " << offset << ") <- значение: " << (int)value << endl;
}

void show_map() {
    cout << "\n--- Карта распределения страниц ---" << endl;
    cout << "Всего виртуальных страниц (эмуляция): " << NUM_PAGES_DISK << endl;
    cout << "Слоты RAM: " << NUM_PAGES_RAM << endl;
    cout << "Формат: [VPageID] | [Slot RAM] | [R] [M] [Valid] | [Location]" << endl;
    cout << "-------------------------------------" << endl;
    for (int vp = 0; vp < NUM_PAGES_DISK; ++vp) {
        int slot = find_page_in_ram(vp);
        bool in_ram = (slot != -1);
        char loc_char;
        int slot_num = -1;
        bool r_bit = false, m_bit = false, valid = false;
        if (in_ram) {
            slot_num = slot;
            r_bit = get_bit_R(slot);
            m_bit = get_bit_M(slot);
            valid = true;
            loc_char = 'R';
        }
        else {
            loc_char = 'D';
        }
        cout << "VPage[" << vp << "] | Slot[" << slot_num << "] | R=" << r_bit
            << " M=" << m_bit << " V=" << valid << " | Location: " << loc_char << endl;
    }
    cout << "-------------------------------------\n" << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(static_cast<unsigned>(time(nullptr)));
    init_memory();
    cout << "Система виртуальной памяти инициализирована." << endl;
    cout << "Размер страницы: " << PAGE_SIZE << " байт." << endl;
    cout << "Размер RAM и DISK: " << MEM_SIZE << " байт." << endl;
    int choice;
    do {
        cout << "\nМеню:" << endl;
        cout << "1. Чтение ячейки памяти (Read)" << endl;
        cout << "2. Запись в ячейку памяти (Write)" << endl;
        cout << "3. Отображение карты распределения (Map)" << endl;
        cout << "0. Выход" << endl;
        cout << "Выберите действие: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        switch (choice) {
        case 1: {
            int addr;
            cout << "Введите виртуальный адрес для чтения (0-" << (NUM_PAGES_DISK * PAGE_SIZE - 1) << "): ";
            if (cin >> addr) {
                if (addr < 0 || addr >= NUM_PAGES_DISK * PAGE_SIZE) {
                    cout << "Ошибка: адрес вне диапазона." << endl;
                }
                else {
                    read_byte(addr);
                }
            }
            break;
        }
        case 2: {
            int addr;
            unsigned int val;
            cout << "Введите виртуальный адрес для записи (0-" << (NUM_PAGES_DISK * PAGE_SIZE - 1) << "): ";
            if (cin >> addr) {
                if (addr < 0 || addr >= NUM_PAGES_DISK * PAGE_SIZE) {
                    cout << "Ошибка: адрес вне диапазона." << endl;
                }
                else {
                    cout << "Введите значение для записи (0-255): ";
                    if (cin >> val) {
                        write_byte(addr, static_cast<unsigned char>(val));
                    }
                }
            }
            break;
        }
        case 3:
            show_map();
            break;
        case 0:
            cout << "Завершение работы." << endl;
            break;
        default:
            cout << "Неверный выбор." << endl;
        }
    } while (choice != 0);
    return 0;
}
