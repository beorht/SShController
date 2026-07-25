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

---

## Уведомления

### Сообщение на экран (3 секунды)
```bash
export DISPLAY=:0; notify-send -t 3000 "Учитель" "Текст сообщения"
```

### Сообщение на экран (10 секунд)
```bash
export DISPLAY=:0; notify-send -t 10000 "Учитель" "Текст сообщения"
```

### Критическое уведомление (пока не закроют)
```bash
export DISPLAY=:0; notify-send -u critical "Внимание!" "Текст сообщения"
```

### Уведомление с иконкой
```bash
export DISPLAY=:0; notify-send -t 5000 -i dialog-information "Заголовок" "Текст"
```

---

## Запуск программ (Wayland / KDE Plasma)

### Запуск программы на ученике
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block <имя_приложения>
```

### Firefox
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block firefox
```

### Dolphin (файловый менеджер)
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block dolphin
```

### Konsole (терминал)
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block konsole
```

### LibreOffice
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block libreoffice
```

### Kate (редактор)
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block kate
```

---

## Закрытие программ (Wayland / KDE Plasma)

### Закрыть программу по имени
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block pkill <имя_приложения>
```

### Закрыть все копии программы
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block killall <имя_приложения>
```

### Закрыть Firefox
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block pkill firefox
```

### Закрыть Konsole
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block pkill konsole
```

### Закрыть Dolphin
```bash
sudo -u student env XDG_RUNTIME_DIR=/run/user/1001 systemd-run --user --no-block pkill dolphin
```
