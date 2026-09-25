# suckless

Mi configuración de dwm, st, dmenu, slstatus, slock, scroll y clipmenu.
Tema Everforest y fuente Fira Code en todos.

Cada programa se importó con `git subtree --squash`, así que es un único repo
pero se puede seguir actualizando desde el proyecto original.

## Compilar e instalar

    cd dwm && make && sudo make install

(Igual para `st`, `dmenu`, `slstatus`, `slock`, `scroll` y `clipmenu`.)

## Actualizar desde el proyecto original

    git subtree pull --prefix=dwm https://git.suckless.org/dwm master --squash
    git subtree pull --prefix=clipmenu https://github.com/cdown/clipmenu develop --squash

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
| Mod+`-` / Mod+`+` | Reducir / aumentar los gaps |
| Mod+Shift+`+` | Gaps a 0 |
| Tecla de subir / bajar volumen | Volumen ±5% con `wpctl` (máximo 100%) |
| Tecla de silenciar | Silenciar o activar el audio |
| Tecla de silenciar micro o Mod+ñ | Silenciar o activar el micrófono |
| Impr Pant | Captura de pantalla completa con `maim` |
| Shift+Impr Pant | Captura de una región |

Las capturas se guardan en `~/Imágenes/Capturas`, se copian al portapapeles y
muestran una notificación.

## Notas

- La configuración personal está en `config.h`. `config.def.h` es la versión
  original más los parches. Si un parche cambia `config.def.h`, hay que pasar
  el cambio a mano a `config.h`: `make` solo lo copia si `config.h` no existe.
- Los parches aplicados están en `<programa>/patches/`.
  - dwm: fullgaps
  - st: ligatures, alpha y anysize (en ese orden; anysize necesita dos
    arreglos a mano, ver el historial de git)
