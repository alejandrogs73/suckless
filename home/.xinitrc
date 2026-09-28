#!/bin/sh

# elogind ya crea XDG_RUNTIME_DIR (/run/user/UID); esto es solo por si falta.
if [ -z "$XDG_RUNTIME_DIR" ]; then
	export XDG_RUNTIME_DIR=/tmp/$(id -u)-runtime-dir
	mkdir -p "$XDG_RUNTIME_DIR"
	chmod 0700 "$XDG_RUNTIME_DIR"
fi

# Abrir el bus de sesión D-Bus antes de lanzar nada, para que pipewire,
# wireplumber, dunst... y dwm compartan el mismo bus.
if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
	exec dbus-run-session -- sh "$HOME/.xinitrc"
fi

# Apps Qt con el tema de qt5ct/qt6ct (Everforest, Papirus). El plugin de
# qt6ct también responde a "qt5ct", así que vale para Qt 5 y Qt 6.
export QT_QPA_PLATFORMTHEME=qt5ct

# Teclado y pantallas primero. pantallas (sistema/usr/local/bin) las pone en
# fila, con la del portátil a la izquierda, y pinta el fondo; al conectar o
# desconectar una, lo repite solo (regla de udev en sistema/).
setxkbmap es
pantallas

pipewire &
wireplumber &
pipewire-pulse &

dunst &
picom &
slstatus &
clipmenud &
luz &
bateria &
marcadores &

# Bloquear con slock tras 30 minutos sin tocar nada, y apagar la pantalla un
# minuto después. Al suspender ya bloquea el hook de zzz (--ignore-sleep).
xset s 1800 1800
xset dpms 0 0 1860
xss-lock --ignore-sleep -- slock &

exec dwm
