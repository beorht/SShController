# Classroom Control

Система удалённого управления компьютерным классом через SSH — аналог
NetSupport School / Veyon, реализуемый на C с использованием libssh.

## Описание проекта

Один главный сервер (компьютер преподавателя) управляет несколькими клиентами
(компьютеры учеников) через SSH:

- отправка и выполнение произвольных команд терминала на клиентах;
- получение скриншотов экрана клиентов (в перспективе — живой стрим экрана);
- масштабирование на класс из 20–30 машин.

## Структура проекта

```
ControlSystem/
├── src/
│   ├── Control.h / Control.c   ← SSH управление
│   ├── DataBase.h / DataBase.c ← Работа с БД (SQLite)
│   ├── Scan.h / Scan.c         ← ARP-сканер сети
│   └── Main.c                  ← Точка входа
├── data/
│   └── database.sqlite         ← БД с кабинетами и ПК
├── build/
│   └── debug/
│       └── control             ← Собранный бинарник
├── docs/
│   ├── architect.md            ← Архитектура проекта
│   └── DEVELOPMENT_PLAN.md     ← План разработки
├── Standart_Command.md         ← Стандартные команды для клиентов
└── StartCom.md                 ← Команды сборки и запуска
```

## Модули

| Модуль | Файлы | Описание |
|--------|-------|----------|
| **Control** | `Control.h` + `Control.c` | SSH: подключение, выполнение команд, broadcast |
| **DataBase** | `DataBase.h` + `DataBase.c` | SQLite: запросы к БД, списки кабинетов и ПК |
| **Scan** | `Scan.h` + `Scan.c` | Сеть: ping sweep, ARP-таблица, фильтр по MAC |
| **Main** | `Main.c` | Точка входа: меню, оркестрация модулей |

## Зависимости

- `libssh` — аутентификация/транспорт по SSH
- `sqlite3` — работа с базой данных
- `gcc`, `pkg-config`

Установка на Arch Linux:

```bash
sudo pacman -S libssh gcc pkgconf make sqlite3
```

## Сборка

```bash
gcc -g -Wall -Wextra src/Main.c src/Control.c src/DataBase.c src/Scan.c \
    -o build/debug/control \
    $(pkg-config --cflags --libs libssh) -lsqlite3
```

## Запуск

### Быстрый (без пинга)

```bash
sudo ./build/debug/control
```

### С предварительным пингом всей подсети

```bash
sudo ./build/debug/control --scan
```

### С параметрами

```bash
sudo ./build/debug/control [db_path] [key_path] [user] [subnet] [--scan]
```

### Параметры по умолчанию

| Параметр | Значение |
|----------|----------|
| db_path | `data/database.sqlite` |
| key_path | `/home/thinklinux/.ssh/classroom_agent` |
| user | `teacher` |
| subnet | `192.168.100` |

## Использование

### Выбор кабинета

При запуске программа показывает список кабинетов из базы данных:

```
=== Select room ===

  [1] room_403
  [2] room_407
  [3] room_414
  [4] room_418

  [0] Exit

Choice: 1
```

### Интерактивные команды

| Команда | Описание |
|---------|----------|
| `<command>` | Отправить команду на все ПК кабинета |
| `list` | Показать подключённые ПК кабинета |
| `scan` | Показать все устройства в сети (ARP-таблица) |
| `help` | Список команд |
| `exit` / `quit` / `q` | Отключение и выход |

### Примеры команд

```bash
>> ip a              # показать IP-адреса на клиентах
>> whoami            # текущий пользователь
>> uname -a          # информация о системе
>> df -h             # использование диска
>> sudo reboot       # перезагрузка всех ПК кабинета
>> scan              # показать все устройства в сети
>> list              # показать подключённые ПК
>> exit              # выход
```

## Логика работы

```
1. Меню → выбор кабинета
2. БД → берём MAC-адреса для кабинета
3. ARP → читаем таблицу (или пингуем + читаем)
4. Фильтр → ищем только MAC из БД
5. SSH → подключаемся к найденным ПК
6. Интерактив → отправка команд
```

## Настройка SSH-ключей

Аутентификация — только по ключам, отдельная пара под проект:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/classroom_agent -N ""
```

Права доступа:

```bash
chmod 700 ~/.ssh
chmod 600 ~/.ssh/classroom_agent
```

Публичный ключ (`classroom_agent.pub`) копируется на каждый клиент в
`~/.ssh/authorized_keys` соответствующего пользователя.

## База данных

Структура SQLite базы `data/database.sqlite`:

| Таблица | Столбцы | Описание |
|---------|---------|----------|
| `pc_address` | id, name, mac_address, ip | Все ПК |
| `room_403` | id, name, mac_address, ip | ПК кабинета 403 |
| `room_407` | id, name, mac_address, ip | ПК кабинета 407 |
| `room_414` | id, name, mac_address, ip | ПК кабинета 414 |
| `room_418` | id, name, mac_address, ip | ПК кабинета 418 |

## Безопасность

- Аутентификация только по ключам
- Отдельный ограниченный пользователь на клиенте (без sudo)
- Проверка `known_hosts` в проде
- Логирование всех выполненных команд

## Известные проблемы (libssh)

- `SSH_OPTIONS_IDENTITY`必须设置在 `ssh_connect()` 之前
- `SSH_OPTIONS_PORT` 需要传指针: `&(int){22}`
- Приватный ключ必须有 `chmod 600`，`~/.ssh` — `chmod 700`

## План разработки

См. [`docs/architect.md`](docs/architect.md) и [`docs/DEVELOPMENT_PLAN.md`](docs/DEVELOPMENT_PLAN.md).
