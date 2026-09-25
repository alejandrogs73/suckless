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

## Notas

- La configuración personal está en `config.h`. `config.def.h` es la versión
  original más los parches. Si un parche cambia `config.def.h`, hay que pasar
  el cambio a mano a `config.h`: `make` solo lo copia si `config.h` no existe.
- Los parches aplicados están en `<programa>/patches/`.
  - dwm: fullgaps
  - st: ligatures, alpha y anysize (en ese orden; anysize necesita dos
    arreglos a mano, ver el historial de git)
