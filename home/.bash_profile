# .bash_profile

# Get the aliases and functions
[ -f $HOME/.bashrc ] && . $HOME/.bashrc

# Al entrar en tty1 se arranca X directamente; al salir de dwm se cierra la sesión.
if [ -z "$DISPLAY" ] && [ "$(tty)" = /dev/tty1 ]; then
	exec startx
fi
