#!/bin/bash

KEY_PATH="$HOME/.ssh/classroom_agent.pub"
USER="teacher"
IPS_FILE=""

show_help() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -f <file>    File with IP addresses (one per line)"
    echo "  -u <user>    SSH username (default: teacher)"
    echo "  -k <path>    Path to public key (default: ~/.ssh/classroom_agent.pub)"
    echo "  -h           Show this help"
    echo ""
    echo "Example:"
    echo "  $0 -f room_403_ips.txt"
    echo "  $0 -f ips.txt -u student -k ~/.ssh/mykey.pub"
}

while getopts "f:u:k:h" opt; do
    case $opt in
        f) IPS_FILE="$OPTARG" ;;
        u) USER="$OPTARG" ;;
        k) KEY_PATH="$OPTARG" ;;
        h) show_help; exit 0 ;;
        *) show_help; exit 1 ;;
    esac
done

if [ -z "$IPS_FILE" ]; then
    echo "Error: No IP file specified"
    echo ""
    show_help
    exit 1
fi

if [ ! -f "$IPS_FILE" ]; then
    echo "Error: File $IPS_FILE not found"
    exit 1
fi

if [ ! -f "$KEY_PATH" ]; then
    echo "Error: Key $KEY_PATH not found"
    exit 1
fi

echo "=== SSH Key Distribution ==="
echo "Key: $KEY_PATH"
echo "User: $USER"
echo "IPs file: $IPS_FILE"
echo ""

TOTAL=0
SUCCESS=0
FAILED=0

while IFS= read -r ip; do
    [ -z "$ip" ] && continue
    TOTAL=$((TOTAL + 1))

    echo -n "  $ip ... "

    if ssh-copy-id -i "$KEY_PATH" -o StrictHostKeyChecking=no -o ConnectTimeout=5 "$USER@$ip" 2>/dev/null; then
        echo -e "\033[32mOK\033[0m"
        SUCCESS=$((SUCCESS + 1))
    else
        echo -e "\033[31mFAILED\033[0m"
        FAILED=$((FAILED + 1))
    fi
done < "$IPS_FILE"

echo ""
echo "=== Results ==="
echo "Total:   $TOTAL"
echo -e "Success: \033[32m$SUCCESS\033[0m"
echo -e "Failed:  \033[31m$FAILED\033[0m"
