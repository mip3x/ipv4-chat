#!/usr/bin/bash

BIN="./ipv4-chat"

yesno() {
    local ans

    while true; do
        read -r -p "$1 [y/n]: " ans || exit 1
            case "${ans,,}" in
            y|yes) return 0 ;;
            n|no)  return 1 ;;
        esac
    done
}

show_help() {
    $BIN -h
}

build_if_needed() {
    if [[ -x "$BIN" ]]; then
        return 0
    fi

    echo "Binary file $BIN not found"
    if yesno "build with 'make'?"; then
        if ! command -v make >/dev/null 2>&1; then
            echo "'make' not found. install 'make' and return" >&2
            exit 1
        fi
        make -j"$(nproc)" || { echo "Build failed" >&2; exit 1; }
    else
        echo "Canceled by user"
    exit 0
    fi
}

default_ip() {
      ip -4 -o addr show scope global 2>/dev/null | awk '{print $4}' | cut -d '/' -f1 | head -n1
}

ask_ip() {
    local def
    def=$(default_ip || true)
    if [[ -n "$def" ]] && yesno "Use default IP (${def})?"; then
        echo "$def"; return 0
    fi

    local ip
    read -r -p "Enter IPv4 (e.g., 192.168.0.17): " ip || exit 1
    echo "$ip"; return 0
}

ask_port() {
    local def=42424
    if yesno "Use default port (${def})?"; then
        echo "$def"; return 0
    fi

    local p
    read -r -p "Enter port (1..65535): " p || exit 1
    echo "$p"; return 0
}

main() {
    build_if_needed

    if yesno "Show help message?"; then
        show_help
    fi

    local ip port verbose_flag=0

    ip=$(ask_ip)
    port=$(ask_port)

    if yesno "Run in debug mode (verbose)?"; then
        verbose_flag=1
    fi

    args=("$BIN" -a "$ip" -p "$port")
    (( verbose_flag )) && args+=(-v)

    printf 'Running:'; printf ' %q' "${args[@]}"; printf '\n'
    exec "${args[@]}"
}

main
