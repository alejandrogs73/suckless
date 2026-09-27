#!/bin/sh

# elogind ya crea XDG_RUNTIME_DIR (/run/user/UID); esto es solo por si falta.
if [ -z "$XDG_RUNTIME_DIR" ]; then
	export XDG_RUNTIME_DIR=/tmp/$(id -u)-runtime-dir
	mkdir -p "$XDG_RUNTIME_DIR"
	chmod 0700 "$XDG_RUNTIME_DIR"
fi

# Abrir el bus de sesión D-Bus antes de lanzar nada, para que pipewire,
# wireplumber, dunst, blueman... y dwm compartan el mismo bus.
if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
	exec dbus-run-session -- sh "$HOME/.xinitrc"
fi

# Teclado y monitores primero; el fondo se pinta después con la geometría final.
setxkbmap es
xrandr --output eDP --primary --auto --output HDMI-A-0 --mode 1280x720 --rate 60.00 --right-of eDP
feh --no-fehbg --bg-fill "$HOME/.local/share/wallpaper.jpg" &

pipewire &
wireplumber &
pipewire-pulse &

dunst &
picom &
blueman-applet &
slstatus &
clipmenud &

exec dwm
