# Команды сборки и запуска

## Структура модулей

```
src/
├── Control.h / Control.c   ← SSH управление
├── DataBase.h / DataBase.c ← Работа с БД (SQLite)
├── Scan.h / Scan.c         ← ARP-сканер сети
└── Main.c                  ← Точка входа, объединяет все модули
```

---

## Сборка проекта

Сборка всех модулей (однострочная):
```bash
gcc -g -Wall -Wextra src/Main.c src/Control.c src/DataBase.c src/Scan.c -o build/debug/control $(pkg-config --cflags --libs libssh) -lsqlite3
```

Сборка всех модулей (многострочная):
```bash
gcc -g -Wall -Wextra src/Main.c src/Control.c src/DataBase.c src/Scan.c \
    -o build/debug/control \
    $(pkg-config --cflags --libs libssh) -lsqlite3
```

---

## Запуск

Быстрый (без пинга):
```bash
sudo ./build/debug/control
```

С предварительным пингом всей подсети:
```bash
sudo ./build/debug/control --scan
```

С параметрами:
```bash
sudo ./build/debug/control [db_path] [key_path] [user] [subnet] [--scan]
```

---

## Параметры по умолчанию

| Параметр | Значение |
|----------|----------|
| db_path | `data/database.sqlite` |
| key_path | `/home/thinklinux/.ssh/classroom_agent` |
| user | `teacher` |
| subnet | `192.168.100` |

---

## Логика работы

```
1. Меню → выбор кабинета
2. БД → берём MAC для кабинета
3. ARP → читаем таблицу (или пингуем + читаем)
4. Фильтр → ищем только MAC из БД
5. SSH → подключаемся к найденным ПК
6. Интерактив → отправка команд
```

---

## Пример работы

```
=== Select room ===

  [1] room_403
  [2] room_407
  [3] room_414
  [4] room_418

  [0] Exit

Choice: 1

Selected: room_403
PCs in database: 38

Reading ARP table for room_403 ...
Devices in ARP cache: 15

Matching MAC addresses:
  Found: PC-403-01      IP: 192.168.100.21   MAC: F4:B5:20:66:F4:37
  Found: PC-403-02      IP: 192.168.100.22   MAC: F4:B5:20:66:F5:2C
  ...

Connected: 10/12

Type 'help' for commands, 'exit' to quit:

>> ip a
>> whoami
>> list
>> exit
```

---

## Зависимости

Установка на Arch Linux:
```bash
sudo pacman -S libssh gcc pkgconf make sqlite3
```

Проверка libssh:
```bash
pkg-config --cflags --libs libssh
```
