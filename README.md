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
| `sistema/` | Archivos que van en `/` (acpid, zzz, elogind, udev, sudoers, runit) |
| `install.sh` | Lo instala todo en un Void recién instalado |

## Instalar

En un Void recién instalado, como tu usuario (pide sudo cuando hace falta):

    git clone ssh://forgejo@ssh.alejandrogs.es/alejandrogs73/suckless.git ~/suckless
    cd ~/suckless && ./install.sh

Hace seis pasos, que también se pueden lanzar por separado
(`./install.sh enlaces`, por ejemplo) y repetir sin problema:

1. `paquetes`: instala con xbps todo lo que usa la configuración.
2. `suckless`: compila e instala los programas en `/usr/local`.
3. `enlaces`: enlaza cada archivo de `home/` en `~`. Si ya existía un archivo
   distinto, lo guarda como `.bak`. Como son enlaces, editar `~/.bashrc` es
   editar el repo. También crea las carpetas del usuario y deja `~/.ssh` en
   700.
4. `gtk`: compila el tema [Everforest GTK](https://github.com/Fausto-Korpsvart/Everforest-GTK-Theme)
   (verde, oscuro, paleta medium) en `~/.themes`, en una versión fija, y lo
   enlaza para las apps de GTK 4. Corrige los colores de la parte de GTK 2
   (el tema trae los de Gruvbox) y pone verdes las carpetas de Papirus. Los
   iconos son Papirus-Dark en todo. Las apps Qt 5 y Qt 6 usan qt5ct/qt6ct
   (`QT_QPA_PLATFORMTHEME` en `.xinitrc`) con el estilo Fusion y una paleta
   Everforest (`home/.config/qt*ct/`). Si una actualización de
   `papirus-icon-theme` devuelve las carpetas a azul, basta con repetir este
   paso.
5. `gnupg`: para después de copiar `~/.gnupg` a mano (de un USB, por
   ejemplo). Arregla los permisos, pone `pinentry-gtk` (en una tty cae a
   curses solo), activa el agente SSH de gpg, añade a `sshcontrol` las
   subclaves de autenticación y hace que recuerde la contraseña una hora.
   `.bashrc` ya apunta `SSH_AUTH_SOCK` al agente. Si `~/.gnupg` no existe
   todavía, no hace nada.
6. `sistema`: copia `sistema/` en `/`, activa los servicios de runit (dbus,
   elogind, polkitd, NetworkManager, bluetoothd, acpid, chronyd, tlp,
   automontaje, cupsd), quita dhcpcd y wpa_supplicant (NetworkManager ya
   gestiona la red) y añade el usuario a los grupos audio, video, input,
   network, bluetooth y lpadmin (para gestionar impresoras). Los archivos de
   `sudoers.d` se validan con `visudo` y se instalan con modo 440.

Después hay que cerrar sesión y volver a entrar (por los grupos) y lanzar
`startx`.

Nix no se instala: lo poco que viene de ahí (webcord) se instala a mano.

## Recompilar tras cambiar la configuración

    cd suckless/dwm && make && sudo make install

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

Las capturas se guardan en `~/Images/Screenshots`, se copian al portapapeles
y muestran una notificación.

## Sesión

- Al entrar en tty1 se lanza `startx` solo (`.bash_profile`); en las demás
  tty no.
- `.xinitrc` arranca también `gammastep` (luz cálida de 20:00 a 8:00, con
  transición de una hora) y `bateria` (avisa al 15 % y, en rojo, al 5 %; al
  3 % suspende con `sudo -n zzz`). Suspender, apagar y reiniciar no piden
  contraseña (`sistema/etc/sudoers.d/energia`).
- Pantallas: `autorandr` las pone en fila (la del portátil a la izquierda y
  como principal) al arrancar y cada vez que se conecta o desconecta una
  (regla de udev en `sistema/`), y vuelve a pintar el fondo.
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
  memoria en la que se ha escrito, `sync` (o `sudo umount /mnt/...`).
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
- st muestra imágenes con el protocolo de gráficos de kitty; yazi lo detecta
  solo y enseña las vistas previas sin ueberzugpp.
- slock está modificado a mano (no es un parche): reloj, fecha, batería y
  una barra inferior con el color del estado.
