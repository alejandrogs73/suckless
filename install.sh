#!/bin/sh
# Deja un Void Linux recién instalado con este escritorio: paquetes, programas
# de suckless, enlaces de la configuración en ~ y archivos de sistema.
#
# Uso: ./install.sh [paquetes] [suckless] [enlaces] [gtk] [gnupg] [sistema] [quitar]
# Sin argumentos hace los siete pasos, en ese orden. Se ejecuta como el
# usuario normal; pide la contraseña de root (doas, o sudo en un Void recién
# instalado) cuando hace falta. Se puede repetir sin problema.

set -eu

DIR=$(cd "$(dirname "$0")" && pwd)

PAQUETES="
	base-devel pkg-config
	libX11-devel libXft-devel libXinerama-devel libXrandr-devel libXext-devel
	libXfixes-devel libxcb-devel freetype-devel fontconfig-devel harfbuzz-devel
	libXrender-devel xorgproto imlib2-devel zlib-devel libxcrypt-devel

	xorg xinit setxkbmap xrandr dbus turnstile
	picom dunst libnotify xwallpaper nsxiv maim xclip sct xssstate
	pipewire wireplumber libspa-bluetooth alsa-pipewire
	wpa_supplicant dhcpcd bluez acpid openntpd tlp zramen
	cups cups-filters hplip
	font-firacode nerd-fonts-symbols-ttf papirus-icon-theme papirus-folders
	sassc gnome-themes-extra qt5ct

	opendoas gnupg pinentry-dmenu
	firefox thunderbird mpv neovim git fuse-sshfs unzip
	yazi file bsdtar ffmpeg 7zip jq poppler fd ripgrep fzf zoxide resvg ImageMagick
"

SUCKLESS="dwm st dmenu slstatus slock scroll clipmenu"
# Tema GTK Everforest, en una versión fija para que siempre salga igual
GTK_TEMA_REPO=https://github.com/Fausto-Korpsvart/Everforest-GTK-Theme
GTK_TEMA_COMMIT=9b8be4d6648ae9eaae3dd550105081f8c9054825
SERVICIOS="dbus turnstiled wpa_supplicant dhcpcd bluetoothd acpid openntpd tlp zramen automontaje cupsd"
# La red la llevan wpa_supplicant y dhcpcd. polkitd no hace falta como
# servicio: si algo lo pide (libvirt), D-Bus lo arranca. turnstiled sustituye
# a elogind y openntpd a chronyd. avahi-daemon (mDNS) no hace falta: la
# impresora va por IP fija. Al portátil no se entra por SSH (sshd) y Nix
# (nix-daemon) ya no se usa.
SERVICIOS_FUERA="NetworkManager polkitd elogind chronyd avahi-daemon sshd nix-daemon"
# _pipewire da prioridad de tiempo real al audio sin rtkit
# (/etc/security/limits.d/25-pw-rlimits.conf, de pipewire).
GRUPOS="audio video input network bluetooth lpadmin _pipewire"
# Lo que sustituyen otras cosas del repo: sudo (doas), feh (xwallpaper y
# nsxiv), gammastep (sct), autorandr (pantallas), blueman y pavucontrol
# (scripts bluetooth y volumen), pinentry-gtk (pinentry-dmenu), vlc (mpv),
# NetworkManager (wpa_supplicant, dhcpcd y el script wifi), rtkit (el grupo
# _pipewire), elogind (turnstile; Xorg arranca como root con Xorg.wrap y la
# tapa y los botones ya los lleva acpid), xss-lock (inactivo, con xssstate),
# chrony (openntpd), avahi y nss-mdns (nada los usa) y nemo con upower (yazi;
# con nemo se van gvfs y udisks2), obs y qt6ct, y nix (webcord: Discord va en
# Firefox).
QUITAR="
	sudo feh gammastep autorandr blueman pavucontrol pinentry-gtk
	gtk-engine-murrine vlc btop NetworkManager tlp-rdw rtkit elogind
	xss-lock chrony avahi nss-mdns nemo upower obs qt6ct nix
"

msg() { printf '\033[1;32m==>\033[0m %s\n' "$*"; }
die() { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }

# Como root: con doas cuando ya está configurado; antes (Void recién
# instalado) con sudo.
root() {
	if command -v doas >/dev/null && [ -f /etc/doas.conf ]; then
		doas "$@"
	else
		sudo "$@"
	fi
}

paquetes() {
	msg "Instalando paquetes"
	root xbps-install -Syu xbps
	# shellcheck disable=SC2086
	root xbps-install -Syu $PAQUETES
}

suckless() {
	for p in $SUCKLESS; do
		msg "Compilando $p"
		make -C "$DIR/suckless/$p" clean >/dev/null
		make -C "$DIR/suckless/$p"
		root make -C "$DIR/suckless/$p" install
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

	# Los enlaces a archivos que ya no están en home/ (borrados del repo)
	find "$HOME" -xdev -type l -lname "$DIR/home/*" ! -exec test -e {} \; -print |
		while read -r l; do
			rm "$l"
			rmdir "$(dirname "$l")" 2>/dev/null || true
			echo "  ${l#"$HOME"/} (ya no está en el repo: quitado)"
		done

	# Las carpetas de home/.config/user-dirs.dirs (y la de las capturas)
	mkdir -p "$HOME/Documents" "$HOME/Downloads" "$HOME/Images/Screenshots"
	# ssh no quiere ~/.ssh accesible para otros
	chmod 700 "$HOME/.ssh"
}

# Compila el tema GTK Everforest (verde, oscuro, paleta medium) en ~/.themes
# y lo enlaza para las apps de GTK 4. Qué tema e iconos se usan lo dice
# home/.config/gtk-*/settings.ini. Las carpetas de Papirus se ponen verdes;
# una actualización de papirus-icon-theme las devuelve a azul, y basta con
# repetir este paso.
gtk() {
	msg "Compilando el tema GTK Everforest"
	tmp=$(mktemp -d)
	git clone -q "$GTK_TEMA_REPO" "$tmp"
	git -C "$tmp" checkout -q "$GTK_TEMA_COMMIT"
	"$tmp/themes/install.sh" -t green -c dark --tweaks medium -l >/dev/null
	rm -rf "$tmp"

	msg "Carpetas de Papirus en verde"
	root papirus-folders -C green --theme Papirus-Dark >/dev/null
}

# ~/.gnupg se copia a mano (de un USB, por ejemplo). Deja los permisos como
# los quiere gpg, usa pinentry-dmenu (en una tty, pinentry-curses; lo decide
# home/.local/bin/pinentry-menu) y activa el agente SSH con las claves de
# autenticación.
gnupg() {
	g="$HOME/.gnupg"
	if [ ! -d "$g" ]; then
		msg "No hay ~/.gnupg: cópialo y ejecuta ./install.sh gnupg"
		return 0
	fi

	msg "Arreglando permisos de $g"
	root chown -R "$(id -u):$(id -g)" "$g"
	find "$g" -type d -exec chmod 700 {} +
	find "$g" -type f -exec chmod 600 {} +

	msg "Configurando gpg-agent (pinentry-dmenu y SSH)"
	conf="$g/gpg-agent.conf"
	touch "$conf"
	chmod 600 "$conf"
	sed -i '/^pinentry-program/d; /^default-cache-ttl/d' "$conf"
	echo "pinentry-program $HOME/.local/bin/pinentry-menu" >>"$conf"
	grep -qx enable-ssh-support "$conf" || echo enable-ssh-support >>"$conf"
	# Recordar la contraseña una hora desde el último uso (por defecto, 10 min)
	echo "default-cache-ttl 3600" >>"$conf"
	echo "default-cache-ttl-ssh 3600" >>"$conf"

	# pinentry-dmenu con la fuente y los colores de dmenu
	cat >"$g/pinentry-dmenu.conf" <<-EOF
		asterisk = "*";
		prompt = "Contraseña:";
		font = "Fira Code:size=11";
		prompt_bg = "#a7c080";
		prompt_fg = "#2d353b";
		normal_bg = "#2d353b";
		normal_fg = "#d3c6aa";
		select_bg = "#a7c080";
		select_fg = "#2d353b";
		desc_bg = "#2d353b";
		desc_fg = "#d3c6aa";
	EOF
	chmod 600 "$g/pinentry-dmenu.conf"

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
		case $f in
		/etc/doas.conf)
			# Un doas.conf roto deja sin root: se valida antes de copiarlo.
			doas -C ".$f" || die "doas.conf inválido"
			root install -D -m 400 ".$f" "$f"
			;;
		*) root install -D -m "$(stat -c %a ".$f")" ".$f" "$f" ;;
		esac
		echo "  $f"
	done
	cd "$DIR"

	# wpa_cli sin root para los de wheel (script wifi) y guardar las redes
	# nuevas. El archivo tiene las contraseñas: se edita en su sitio.
	msg "Configurando wpa_supplicant"
	w=/etc/wpa_supplicant/wpa_supplicant.conf
	cabecera="ctrl_interface=/run/wpa_supplicant
ctrl_interface_group=wheel
update_config=1"
	# shellcheck disable=SC2016
	root sh -c 'umask 077; touch "$1"
		{ printf "%s\n" "$2"; grep -v -e "^ctrl_interface" -e "^update_config" "$1"; } >"$1.nuevo"
		mv "$1.nuevo" "$1"' sh "$w" "$cabecera"

	msg "Activando servicios"
	for s in $SERVICIOS; do
		[ -d "/etc/sv/$s" ] || die "no existe el servicio $s (¿faltan paquetes?)"
		if [ ! -e "/var/service/$s" ]; then
			root ln -s "/etc/sv/$s" /var/service/
		fi
	done
	for s in $SERVICIOS_FUERA; do
		if [ -e "/var/service/$s" ]; then
			root rm "/var/service/$s"
		fi
	done

	msg "Añadiendo $USER a los grupos: $GRUPOS"
	for g in $GRUPOS; do
		if getent group "$g" >/dev/null; then
			root usermod -aG "$g" "$USER"
		fi
	done
}

# Desinstala lo que el repo ya no usa (QUITAR), si está instalado. sudo solo
# se quita cuando doas ya funciona (paso sistema).
quitar() {
	[ -f /etc/doas.conf ] || die "falta /etc/doas.conf: ejecuta antes el paso sistema"
	fuera=
	for p in $QUITAR; do
		xbps-query "$p" >/dev/null 2>&1 && fuera="$fuera $p"
	done
	if [ -z "$fuera" ]; then
		return 0
	fi
	msg "Quitando:$fuera"
	# shellcheck disable=SC2086
	root xbps-remove -Ry $fuera

	# Sin nss-mdns, "mdns" sobra en la línea hosts de nsswitch.conf
	if grep -q '^hosts:.*mdns' /etc/nsswitch.conf; then
		root sed -i '/^hosts:/s/ mdns[a-z0-9_]*\( \[NOTFOUND=return\]\)\{0,1\}//g' \
			/etc/nsswitch.conf
	fi

	# Sin el paquete nix, su almacén (/nix) y los enlaces del perfil sobran
	if ! xbps-query nix >/dev/null 2>&1 && [ -d /nix ]; then
		msg "Borrando /nix y el perfil de Nix"
		root rm -rf /nix
		rm -rf "$HOME/.nix-profile" "$HOME/.nix-defexpr" "$HOME/.nix-channels" \
			"$HOME/.local/state/nix" "$HOME/.cache/nix"
	fi

	# Lo que puso aquí una versión anterior de sistema/ para elogind
	if [ -e /etc/elogind/logind.conf.d/10-acpid.conf ]; then
		root rm /etc/elogind/logind.conf.d/10-acpid.conf
	fi
}

[ -f /etc/void-release ] || command -v xbps-install >/dev/null ||
	die "esto es solo para Void Linux"
[ "$(id -u)" -ne 0 ] || die "ejecútalo como tu usuario, no como root"

[ $# -gt 0 ] || set -- paquetes suckless enlaces gtk gnupg sistema quitar
for paso; do
	case $paso in
	paquetes|suckless|enlaces|gtk|gnupg|sistema|quitar) "$paso" ;;
	*) die "paso desconocido: $paso" ;;
	esac
done

msg "Hecho. Cierra sesión y vuelve a entrar para aplicar los grupos; luego startx."
