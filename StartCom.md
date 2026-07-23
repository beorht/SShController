# Команды сборки и запуска

## Main.c (SSH подключение к одному клиенту)

Сборка:
```bash
gcc -g -Wall -Wextra src/Main.c -o build/debug/main $(pkg-config --cflags --libs libssh)
```

Запуск:
```bash
./build/debug/main
```

---

## Control.c (SSH подключение к нескольким клиентам + интерактивный ввод)

Сборка:
```bash
gcc -g -Wall -Wextra src/Control.c -o build/debug/control $(pkg-config --cflags --libs libssh)
```

Запуск:
```bash
./build/debug/control
```

Использование:
```
Connected to 2 clients. Enter commands (type 'exit' to quit):

>> ip a
>> whoami
>> ls -la
>> exit
```

---

## Scan.c (ARP-сканер локальной сети)

Сборка:
```bash
gcc -g -Wall -Wextra src/Scan.c -o build/debug/scan
```

Запуск (подсеть по умолчанию 192.168.100.0/24):
```bash
sudo ./build/debug/scan
```

Запуск с другой подсетью:
```bash
sudo ./build/debug/scan 10.0.0
```

---

## Зависимости

Установка на Arch Linux:
```bash
sudo pacman -S libssh gcc pkgconf make sqlite
```

Проверка libssh:
```bash
pkg-config --cflags --libs libssh
```
