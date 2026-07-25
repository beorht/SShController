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

>> help
Commands:
  <command>   - send command to all clients in room_403
  list        - show connected clients
  scan        - show all devices on network
  reconnect   - rescan network and reconnect to found PCs
  help        - show this help
  exit/quit/q - disconnect and exit

>> scan
Scanning ARP table...

#    IP               MAC
---  ---              ---
1    192.168.100.1    aa:bb:cc:dd:ee:01
2    192.168.100.21   F4:B5:20:66:F4:37
3    192.168.100.22   F4:B5:20:66:F5:2C
...

Total: 15 devices

>> ip a
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

---

## Настройка энергопитания KDE Plasma (справочно)

Не относится к проекту напрямую — заметка про управление настройками
«Управление питанием» KDE Plasma (System Settings) через конфиги/команды,
без открытия графического интерфейса. Полезно, например, для клиентских
машин класса, где нужно массово отключить автосон/выключение экрана.

### Версия Plasma

```bash
plasmaversion            # Plasma 6
plasmashell --version    # Plasma 5 и 6
```

От версии зависит бинарник: `kwriteconfig5`/`kreadconfig5` (Plasma 5)
или `kwriteconfig6`/`kreadconfig6` (Plasma 6).

### Файл настроек

```
~/.config/powermanagementprofilesrc
```

Секции разбиты по профилям питания: `[AC]` (от сети), `[Battery]`,
`[LowBattery]`.

### Соответствие пунктов интерфейса ключам конфига

| Пункт в GUI | Группа | Ключ | Значения |
|---|---|---|---|
| Приостановка сеанса → При неактивности | `SuspendSession` | `idleTime` (мс), `suspendType` | `suspendType`: 1=Sleep, 2=Hibernate, 3=Hybrid |
| При нажатии кнопки питания | `HandleButtonEvents` | `powerButtonAction` | 0=Ничего, 1=Спящий, 2=Гибернация, 4=ShutDown, 16=Экран завершения работы |
| Способ приостановки сеанса | `SuspendSession` | `suspendType` | как выше |
| Затемнение экрана через | `DimDisplay` | `idleTime` | мс |
| Отключение экрана через | `DPMSControl` | `idleTime` | мс, `0` = никогда |
| Профиль энергопотребления | `PowerProfile` | `profile` | `performance` / `balanced` / `power-saver` |
| «При неактивности» (запуск скрипта) | `RunScript` | `idleTime`, `runOnResumeScript`, `suspendScript` | путь к скрипту |

### Команды чтения/записи (пример Plasma 6)

```bash
# Прочитать текущее значение
kreadconfig6 --file powermanagementprofilesrc --group AC --group DPMSControl --key idleTime

# Отключить выключение экрана по таймауту (AC-профиль)
kwriteconfig6 --file powermanagementprofilesrc --group AC --group DPMSControl --key idleTime 0

# Приостановка сеанса через 15 минут (900000 мс), режим Sleep
kwriteconfig6 --file powermanagementprofilesrc --group AC --group SuspendSession --key idleTime 900000
kwriteconfig6 --file powermanagementprofilesrc --group AC --group SuspendSession --key suspendType 1

# Действие кнопки питания — «Экран завершения работы»
kwriteconfig6 --file powermanagementprofilesrc --group AC --group HandleButtonEvents --key powerButtonAction 16

# Профиль энергопотребления — сбалансированный
kwriteconfig6 --file powermanagementprofilesrc --group General --key currentProfile balanced
```

Для Plasma 5 — те же команды, но `kwriteconfig5`/`kreadconfig5`.

### Применить изменения без перелогина

```bash
# Plasma 5
kquitapp5 kded5 && kded5 --replace &

# Plasma 6
systemctl --user restart plasma-powerdevil.service
```

### Разовые действия через D-Bus (без изменения профиля)

```bash
# Уйти в сон прямо сейчас
qdbus org.kde.Solid.PowerManagement /org/kde/Solid/PowerManagement Suspend

# Заблокировать автосон на время выполнения задачи
qdbus org.freedesktop.PowerManagement.Inhibit /org/freedesktop/PowerManagement/Inhibit \
    org.freedesktop.PowerManagement.Inhibit.Inhibit "myapp" "долгая задача"
```

**Важно**: перед ручной правкой конфига сделать резервную копию —
```bash
cp ~/.config/powermanagementprofilesrc ~/.config/powermanagementprofilesrc.bak
```
Неверная секция/ключ в файле молча игнорируется, ошибка не выводится.
