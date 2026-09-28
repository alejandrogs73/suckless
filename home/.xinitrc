#!/bin/sh

# turnstile ya crea XDG_RUNTIME_DIR (/run/user/UID); esto es solo por si falta.
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

# Apps Qt 5 (las ventanas de hplip) con el tema de qt5ct (Everforest, Papirus).
export QT_QPA_PLATFORMTHEME=qt5ct

# Teclado y pantallas primero. pantallas (sistema/usr/local/bin) las pone en
# fila, con la del portátil a la izquierda, y pinta el fondo; al conectar o
# desconectar una, lo repite solo (regla de udev en sistema/).
setxkbmap es
pantallas

# Audio. Si alguno se cae, vuelve solo al segundo (sin wireplumber no hay
# ni altavoces ni bluetooth: todo sale por "Dummy Output"). Al cerrar X se
# acaba el bucle.
for p in pipewire wireplumber pipewire-pulse; do
	while xset q >/dev/null 2>&1; do "$p"; sleep 1; done &
done

dunst &
picom &
slstatus &
clipmenud &
luz &
bateria &
marcadores &

# Bloquear con slock tras 30 minutos sin tocar nada (salvapantallas de X,
# lo vigila inactivo) y apagar la pantalla un minuto después.
xset s 1800 1800
xset dpms 0 0 1860
inactivo &

exec dwm
