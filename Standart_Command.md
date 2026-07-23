# Стандартные команды для клиентов

Команды, отправляемые через `Control.c` на компьютеры учеников.

---

## Система

### Отключение спящего режима
```bash
sudo systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target
```
Запрещает сон, спящий режим, гибернацию и гибридный сон.

### Включение спящего режима
```bash
sudo systemctl unmask sleep.target suspend.target hibernate.target hybrid-sleep.target
```
Разрешает сон и спящий режим обратно.

### Перезагрузка
```bash
sudo reboot
```

### Выключение
```bash
sudo poweroff
```

### Статус системы
```bash
uname -a && uptime
```
Версия ядра и время работы.

---

## Сеть

### Проверка IP
```bash
ip a
```

### Проверка подключения к интернету
```bash
ping -c 3 8.8.8.8
```

### Проверка DNS
```bash
ping -c 3 google.com
```

### Сетевые интерфейсы
```bash
ip link show
```

---

## Процессы

### Список процессов
```bash
ps aux --sort=-%mem | head -20
```
Топ-20 процессов по использованию памяти.

### Загрузка CPU
```bash
top -bn1 | head -15
```

---

## Файлы

### Проверка диска
```bash
df -h
```

### Содержимое папки
```bash
ls -la /home/teacher/
```

### Создание папки
```bash
mkdir -p /home/teacher/shared
```

---

## Обновление

### Обновление пакетов
```bash
sudo apt update && sudo apt upgrade -y
```

### Установка пакета
```bash
sudo apt install -y <имя_пакета>
```

---

## Управление пользователями

### Список пользователей
```bash
cat /etc/passwd | grep -v nologin | grep -v false
```

### Текущий пользователь
```bash
whoami
```

### Кто онлайн
```bash
who
```
