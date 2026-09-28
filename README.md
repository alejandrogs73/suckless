# suckless

**Español** · [English](README-en.md)

Mis dotfiles para Void Linux: dwm, st, dmenu, slstatus, slock, scroll y
clipmenu, más la configuración de la sesión (xinitrc, bash, picom, dunst...).
Tema Everforest y fuente Fira Code en todos.

## Estructura

| Carpeta | Contenido |
|---|---|
| `suckless/` | Los programas, cada uno con su `config.h` y sus parches |
| `home/` | Archivos que van en `~`, con la misma ruta (se enlazan) |
| `sistema/` | Archivos que van en `/` (acpid, zzz, turnstile, udev, doas, xbps, runit) |
| `install.sh` | Lo instala todo en un Void recién instalado |

## Instalar

En un Void recién instalado, como tu usuario (pide la contraseña de root
cuando hace falta: con sudo hasta que doas está listo, luego con doas):

    git clone ssh://forgejo@ssh.alejandrogs.es/alejandrogs73/suckless.git ~/suckless
    cd ~/suckless && ./install.sh

Hace siete pasos, que también se pueden lanzar por separado
(`./install.sh enlaces`, por ejemplo) y repetir sin problema:

1. `paquetes`: instala con xbps todo lo que usa la configuración.
2. `suckless`: compila e instala los programas en `/usr/local`.
3. `enlaces`: enlaza cada archivo de `home/` en `~`. Si ya existía un archivo
   distinto, lo guarda como `.bak`. Como son enlaces, editar `~/.bashrc` es
   editar el repo. Los enlaces a archivos que se han borrado del repo se
   quitan. También crea las carpetas del usuario y deja `~/.ssh` en
   700.
4. `gtk`: compila el tema [Everforest GTK](https://github.com/Fausto-Korpsvart/Everforest-GTK-Theme)
   (verde, oscuro, paleta medium) en `~/.themes`, en una versión fija, y lo
   enlaza para las apps de GTK 4, y pone verdes las carpetas de Papirus. Los
   iconos son Papirus-Dark en todo. Las apps Qt 5 y Qt 6 usan qt5ct/qt6ct
   (`QT_QPA_PLATFORMTHEME` en `.xinitrc`) con el estilo Fusion y una paleta
   Everforest (`home/.config/qt*ct/`). Si una actualización de
   `papirus-icon-theme` devuelve las carpetas a azul, basta con repetir este
   paso.
5. `gnupg`: para después de copiar `~/.gnupg` a mano (de un USB, por
   ejemplo). Arregla los permisos, pone `pinentry-dmenu` con los colores de
   dmenu (en una tty, `pinentry-curses`: lo decide
   `home/.local/bin/pinentry-menu`), activa el agente SSH de gpg, añade a `sshcontrol` las
   subclaves de autenticación y hace que recuerde la contraseña una hora.
   `.bashrc` ya apunta `SSH_AUTH_SOCK` al agente. Si `~/.gnupg` no existe
   todavía, no hace nada.
6. `sistema`: copia `sistema/` en `/`, prepara `wpa_supplicant.conf` para que
   los de wheel usen `wpa_cli` sin root y guarden redes, activa los servicios
   de runit (dbus, turnstiled, wpa_supplicant, dhcpcd, bluetoothd, acpid,
   chronyd, tlp, automontaje, cupsd), quita NetworkManager, elogind y polkitd
   (si algo pide polkit, como udisks2 o libvirt, D-Bus lo arranca solo) y
   añade el usuario a los grupos audio, video, input, network, bluetooth,
   lpadmin (para gestionar impresoras) y `_pipewire` (prioridad de tiempo real
   para el audio sin rtkit). `doas.conf` se valida con `doas -C` antes de
   instalarlo con modo 400.
7. `quitar`: desinstala lo que el repo ya no usa porque lo sustituye otra
   cosa: sudo (doas), feh (xwallpaper y nsxiv), gammastep (sct), autorandr
   (`pantallas`), blueman y pavucontrol (scripts `bluetooth` y `volumen`),
   pinentry-gtk (pinentry-dmenu), vlc (mpv), btop, NetworkManager y tlp-rdw
   (wpa_supplicant, dhcpcd y el script `wifi`), rtkit y elogind (turnstile).
   sudo se puede quitar gracias a `sistema/etc/xbps.d/sin-sudo.conf`
   (base-system depende de él), y solo se quita si `/etc/doas.conf` ya está
   instalado.

Después hay que cerrar sesión y volver a entrar (por los grupos) y lanzar
`startx`.

Nix no se instala: lo poco que viene de ahí (webcord) se instala a mano.

## Sin elogind

elogind es el logind de systemd por separado, y aquí sobra:

- `XDG_RUNTIME_DIR` (`/run/user/UID`) lo crea turnstile al entrar
  (`sistema/etc/turnstile/turnstiled.conf`: solo eso, sin servicios de
  usuario).
- Xorg arranca como root con `Xorg.wrap` (`needs_root_rights = yes` en
  `/etc/X11/Xwrapper.config`, lo trae Void).
- La tapa y los botones de encendido y suspender los lleva acpid, que llama a
  `zzz`. Apagar y reiniciar son los de runit.
- Los dispositivos (sonido, vídeo, entrada, bluetooth) van por los grupos del
  usuario, no por las ACL que ponía elogind a la sesión activa.

Lo que deja de ir: montar discos desde un gestor de archivos (udisks2 pide a
polkit una sesión activa, que ya no hay). Las memorias USB las monta
`automontaje`.

## Recompilar tras cambiar la configuración

    cd suckless/dwm && make && doas make install

## Actualizar desde el proyecto original

Cada programa se importó con `git subtree --squash`:

    git subtree pull --prefix=suckless/dwm https://git.suckless.org/dwm master --squash
    git subtree pull --prefix=suckless/clipmenu https://github.com/cdown/clipmenu develop --squash

Los demás funcionan igual que dwm, cambiando el nombre en `--prefix` y en la URL.

## Atajos de dwm

`Mod` es `Super` (en el dwm original es `Alt`). Los atajos de ratón también usan
`Super`. Solo aparecen los atajos que cambian respecto al original o que son
nuevos; el resto están igual. Los atajos de st, scroll, dmenu y slock no se han
tocado.

### Cambiados

| Acción | Original | Ahora |
|---|---|---|
| Abrir terminal (`st`) | Mod+Shift+Return | Mod+Return |
| zoom (mover la ventana al área principal) | Mod+Return | Mod+Shift+Return |
| Cerrar ventana | Mod+Shift+c | Mod+q |
| Salir de dwm | Mod+Shift+q | Mod+Shift+m |

### Nuevos

| Atajo | Acción |
|---|---|
| Mod+v | clipmenu (misma paleta y fuente que dmenu) |
| Mod+Shift+l | Bloquear la pantalla con `slock` |
| Mod+n | Volver a mostrar la última notificación (historial de dunst) |
| Mod+Shift+n | Cerrar todas las notificaciones |
| Mod+Shift+Escape | Menú de sesión: bloquear, suspender, salir de dwm, reiniciar o apagar (los tres últimos piden confirmación) |
| Mod+`-` / Mod+`+` | Reducir / aumentar los gaps |
| Mod+Shift+`+` | Gaps a 0 |
| Tecla de subir / bajar volumen | Volumen ±5% con `wpctl` (máximo 100%) |
| Tecla de silenciar | Silenciar o activar el audio |
| Tecla de silenciar micro o Mod+ñ | Silenciar o activar el micrófono |
| Impr Pant | Captura de pantalla completa con `maim` |
| Shift+Impr Pant | Captura de una región |

Las teclas de audio (y los clics en VOL de la barra) usan el script `volumen`
(`home/.local/bin/volumen`): cambia el volumen con `wpctl`, refresca slstatus
y muestra una notificación con el nivel.

Los menús de los scripts (`apagado`, `bluetooth`, `volumen salida`...) usan
`menu`, que es dmenu con la fuente y los colores de dwm.

### Clics en la barra

| Zona | Izquierdo | Central | Derecho | Rueda |
|---|---|---|---|---|
| Red | Conectarse a una red (`wifi`) | | `wpa_cli` | |
| VOL | Elegir la salida de audio (`volumen salida`) | Silenciar | Silenciar el micro | Volumen ± |
| BT | Menú de bluetooth (`bluetooth`): conectar, desconectar, buscar y emparejar, apagar | | Encender o apagar | |
| Fecha | Calendario del mes | | | |

Las capturas se guardan en `~/Images/Screenshots`, se copian al portapapeles
y muestran una notificación.

## Sesión

- Al entrar en tty1 se lanza `startx` solo (`.bash_profile`); en las demás
  tty no.
- `.xinitrc` arranca también `luz` (luz cálida con `sct` de 20:00 a 8:00,
  con transición de una hora) y `bateria` (avisa al 15 % y, en rojo, al 5 %;
  al 3 % suspende con `doas -n zzz`). Suspender, apagar y reiniciar no piden
  contraseña (`sistema/etc/doas.conf`).
- Pantallas: `pantallas` (`sistema/usr/local/bin`) las pone en fila con
  `xrandr` (la del portátil a la izquierda y como principal) al arrancar y
  cada vez que se conecta o desconecta una (regla de udev en `sistema/`), y
  vuelve a pintar el fondo con `xwallpaper`.
- WiFi: `wifi` busca redes con `wpa_cli` y las enseña en dmenu por señal
  (`*` la actual). Si la red es nueva pide la contraseña (no se ve al
  escribirla) y la guarda en `/etc/wpa_supplicant/wpa_supplicant.conf` solo
  si la conexión va bien. El cable lo coge dhcpcd solo.
- Clic izquierdo en la fecha de la barra: calendario del mes en una
  notificación (`calendario`), con el día de hoy en verde.
- bash: historial de 10 000 órdenes, sin duplicados y compartido entre
  terminales; el prompt muestra la rama de git (`*` cambios sin añadir,
  `+` añadidos).
- `~/.ssh/config` con alias: `ssh server` y `forgejo`, los dos por el
  dominio para que funcionen dentro y fuera de casa. Sin claves: las da
  gpg-agent.
- Copia de los marcadores de Firefox (`marcadores`, desde `.xinitrc`): sube
  una vez al día al servidor la copia que Firefox ya hace sola
  (`bookmarkbackups/*.jsonlz4`), a `/var/storage/PUBLIC/backup_void/marcadores`:
  `diario/` guarda 7 días y `semanal/` una por semana, todas. Solo lo intenta
  con la clave SSH desbloqueada (para no sacar pinentry) y lo reintenta cada
  hora. `marcadores ya` la sube en el momento. Para restaurar: Firefox >
  Marcadores > Administrar marcadores > Importar y respaldar > Restaurar.
- Tras 30 minutos sin tocar nada se bloquea con slock (`xss-lock`) y un
  minuto después se apaga la pantalla.
- Memorias USB y tarjetas SD: el servicio `automontaje` (runit, como root)
  las monta en `/mnt/ETIQUETA`, o en `/mnt/sdXY` si no tienen etiqueta, y
  las desmonta al sacarlas, con una notificación. FAT, exFAT y NTFS quedan a
  nombre del usuario; los discos internos no se tocan. Antes de sacar una
  memoria en la que se ha escrito, `sync` (o `doas umount /mnt/...`).
- Carpetas del usuario en inglés y sin tildes (`user-dirs.dirs`):
  Documents, Downloads e Images.

## Notas

- La configuración personal está en `config.h`. `config.def.h` es la versión
  original más los parches. Si un parche cambia `config.def.h`, hay que pasar
  el cambio a mano a `config.h`: `make` solo lo copia si `config.h` no existe.
- Los parches aplicados están en `suckless/<programa>/patches/`.
  - dwm: fullgaps, restartsig, preserveonrestart, statuscmd-nosignal y
    swallow.
  - st: kitty-graphics, alpha, glyph-wide-support y ligatures, en ese orden.
    kitty-graphics ya incluye anysize. glyph-wide-support y la combinación
    con ligatures vienen de la rama `graphics-with-patches` de
    [st-graphics](https://github.com/sergei-grechanik/st-graphics), sin
    boxdraw.
- En st, Ctrl+Shift+clic derecho sobre una imagen la abre en `nsxiv`.
- st muestra imágenes con el protocolo de gráficos de kitty; yazi lo detecta
  solo y enseña las vistas previas sin ueberzugpp.
- slock está modificado a mano (no es un parche): reloj, fecha, batería y
  una barra inferior con el color del estado.
