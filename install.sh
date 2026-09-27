#!/bin/sh
# Deja un Void Linux recién instalado con este escritorio: paquetes, programas
# de suckless, enlaces de la configuración en ~ y archivos de sistema.
#
# Uso: ./install.sh [paquetes] [suckless] [enlaces] [gnupg] [sistema]
# Sin argumentos hace los cinco pasos, en ese orden. Se ejecuta como el
# usuario normal; pide sudo cuando hace falta. Se puede repetir sin problema.

set -eu

DIR=$(cd "$(dirname "$0")" && pwd)

PAQUETES="
	base-devel pkg-config
	libX11-devel libXft-devel libXinerama-devel libXrandr-devel libXext-devel
	libXfixes-devel libxcb-devel freetype-devel fontconfig-devel harfbuzz-devel
	libXrender-devel xorgproto imlib2-devel zlib-devel libxcrypt-devel

	xorg xinit setxkbmap xrandr dbus elogind polkit
	picom dunst libnotify feh maim xclip
	pipewire wireplumber libspa-bluetooth alsa-pipewire rtkit pavucontrol
	NetworkManager bluez blueman acpid chrony
	font-firacode nerd-fonts-symbols-ttf papirus-icon-theme

	gnupg pinentry-gtk
	firefox thunderbird neovim git fuse-sshfs unzip
	yazi file ffmpeg 7zip jq poppler fd ripgrep fzf zoxide resvg ImageMagick
"

SUCKLESS="dwm st dmenu slstatus slock scroll clipmenu"
SERVICIOS="dbus elogind polkitd NetworkManager bluetoothd acpid chronyd"
# NetworkManager gestiona la red él solo; estos servicios se pelean con él.
SERVICIOS_FUERA="dhcpcd wpa_supplicant"
GRUPOS="audio video input network bluetooth"

msg() { printf '\033[1;32m==>\033[0m %s\n' "$*"; }
die() { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }

paquetes() {
	msg "Instalando paquetes"
	sudo xbps-install -Syu xbps
	# shellcheck disable=SC2086
	sudo xbps-install -Syu $PAQUETES
}

suckless() {
	for p in $SUCKLESS; do
		msg "Compilando $p"
		make -C "$DIR/suckless/$p" clean >/dev/null
		make -C "$DIR/suckless/$p"
		sudo make -C "$DIR/suckless/$p" install
	done
}

# Enlaza cada archivo de home/ en la misma ruta dentro de ~. Si ya hay un
# archivo que no es el enlace, se guarda como .bak en vez de pisarlo.
enlaces() {
	msg "Enlazando la configuración en $HOME"
	cd "$DIR/home"
	find . -type f | while read -r f; do
		f=${f#./}
		src="$DIR/home/$f"
		dst="$HOME/$f"
		if [ "$(readlink "$dst" 2>/dev/null)" = "$src" ]; then
			continue
		fi
		mkdir -p "$(dirname "$dst")"
		if [ -e "$dst" ] || [ -L "$dst" ]; then
			mv "$dst" "$dst.bak"
			echo "  $f -> $f.bak"
		fi
		ln -s "$src" "$dst"
		echo "  $f"
	done
	cd "$DIR"
}

# ~/.gnupg se copia a mano (de un USB, por ejemplo). Deja los permisos como
# los quiere gpg, usa un pinentry gráfico (en una tty cae a curses solo) y
# activa el agente SSH con las claves de autenticación.
gnupg() {
	g="$HOME/.gnupg"
	if [ ! -d "$g" ]; then
		msg "No hay ~/.gnupg: cópialo y ejecuta ./install.sh gnupg"
		return 0
	fi

	msg "Arreglando permisos de $g"
	sudo chown -R "$(id -u):$(id -g)" "$g"
	find "$g" -type d -exec chmod 700 {} +
	find "$g" -type f -exec chmod 600 {} +

	msg "Configurando gpg-agent (pinentry gráfico y SSH)"
	conf="$g/gpg-agent.conf"
	touch "$conf"
	chmod 600 "$conf"
	sed -i '/^pinentry-program/d' "$conf"
	echo "pinentry-program /usr/bin/pinentry-gtk-2" >>"$conf"
	grep -qx enable-ssh-support "$conf" || echo enable-ssh-support >>"$conf"

	# sshcontrol dice qué claves usa el agente para SSH: se añaden las
	# subclaves de autenticación [A] que falten.
	touch "$g/sshcontrol"
	chmod 600 "$g/sshcontrol"
	gpg --with-colons --with-keygrip -K 2>/dev/null |
		awk -F: '/^(sec|ssb):/ { auth = ($12 ~ /a/) } /^grp:/ && auth { print $10 }' |
		while read -r kg; do
			if ! grep -q "^$kg" "$g/sshcontrol"; then
				echo "$kg" >>"$g/sshcontrol"
				echo "  clave SSH: $kg"
			fi
		done

	# Reiniciar el agente para que lea la configuración nueva.
	gpgconf --kill gpg-agent
}

sistema() {
	msg "Copiando archivos de sistema"
	cd "$DIR/sistema"
	find . -type f | while read -r f; do
		f=${f#.}
		sudo install -D -m "$(stat -c %a ".$f")" ".$f" "$f"
		echo "  $f"
	done
	cd "$DIR"

	msg "Activando servicios"
	for s in $SERVICIOS; do
		[ -d "/etc/sv/$s" ] || die "no existe el servicio $s (¿faltan paquetes?)"
		if [ ! -e "/var/service/$s" ]; then
			sudo ln -s "/etc/sv/$s" /var/service/
		fi
	done
	for s in $SERVICIOS_FUERA; do
		if [ -e "/var/service/$s" ]; then
			sudo rm "/var/service/$s"
		fi
	done

	msg "Añadiendo $USER a los grupos: $GRUPOS"
	for g in $GRUPOS; do
		if getent group "$g" >/dev/null; then
			sudo usermod -aG "$g" "$USER"
		fi
	done
}

[ -f /etc/void-release ] || command -v xbps-install >/dev/null ||
	die "esto es solo para Void Linux"
[ "$(id -u)" -ne 0 ] || die "ejecútalo como tu usuario, no como root"

[ $# -gt 0 ] || set -- paquetes suckless enlaces gnupg sistema
for paso; do
	case $paso in
	paquetes|suckless|enlaces|gnupg|sistema) "$paso" ;;
	*) die "paso desconocido: $paso" ;;
	esac
done

msg "Hecho. Cierra sesión y vuelve a entrar para aplicar los grupos; luego startx."
