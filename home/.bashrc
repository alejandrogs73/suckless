# =====================================================================
#  BASHRC MINIMALISTA 
# =====================================================================

# --- 1. Entorno y Rutas ---
# Priorizar binarios locales y scripts de usuario
export PATH="$HOME/.local/bin:/usr/local/bin:$PATH"
export SSH_AUTH_SOCK=$(gpgconf --list-dirs agent-ssh-socket)
export GPG_TTY=$(tty)
gpg-connect-agent updatestartuptty /bye >/dev/null 2>&1

# Cargar Nix (Asegura que los comandos de Nix funcionen si entras sin X11)
if [ -e /etc/profile.d/nix.sh ]; then 
    . /etc/profile.d/nix.sh
fi

# Editor por defecto
export EDITOR="nvim" # Cambia a "vim" si te gusta más sufrir/disfrutar

# --- 2. Prompt (PS1) Limpio y Rápido ---
# Define colores básicos (compatibles con tu paleta Everforest en st)
RED='\[\e[0;31m\]'
GREEN='\[\e[0;32m\]'
YELLOW='\[\e[0;33m\]'
BLUE='\[\e[0;34m\]'
NC='\[\e[0m\]' # Sin color

# Rama de git (con * si hay cambios sin añadir y + si hay cambios añadidos)
if [ -f /usr/share/git/git-prompt.sh ]; then
    . /usr/share/git/git-prompt.sh
    GIT_PS1_SHOWDIRTYSTATE=1
else
    __git_ps1() { :; }
fi

# Formato: usuario@maquina:~/directorio (rama)$ (en colores)
PS1="${GREEN}\u@\h${NC}:${BLUE}\w${YELLOW}\$(__git_ps1 ' (%s)')${NC}\$ "

# --- Historial ---
# Grande, sin duplicados y compartido entre terminales: cada orden se guarda
# al momento y cada prompt lee las de las otras terminales.
HISTSIZE=10000
HISTFILESIZE=20000
HISTCONTROL=ignoreboth:erasedups
shopt -s histappend
PROMPT_COMMAND="history -a; history -n${PROMPT_COMMAND:+; $PROMPT_COMMAND}"

# --- 3. Aliases Base (Comodidad y Seguridad) ---
alias ls='ls --color=auto'
alias ll='ls -lh'      # Lista detallada humana
alias la='ls -lha'     # Lista detallada con ocultos
alias ..='cd ..'
alias ...='cd ../..'
alias c='clear'
alias q='exit'

# Red de seguridad: te preguntará antes de sobreescribir o borrar algo
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

# --- 4. Aliases para Void Linux (xbps) ---
# Te ahorrarán teclear 'xbps-lo-que-sea' 40 veces al día
alias xi='sudo xbps-install'        # Instalar paquete
alias xu='sudo xbps-install -Su'    # Actualizar todo el sistema
alias xq='xbps-query -Rs'           # Buscar paquete en el repositorio
alias xl='xbps-query -l'            # Listar paquetes instalados
alias xr='sudo xbps-remove -R'      # Eliminar paquete y sus huérfanos

alias esp32='sudo chmod a+rw /dev/ttyUSB0'

# --- 5. Funciones Útiles ---

# Comando 'ex': Descomprime LITERALMENTE CUALQUIER COSA
# Uso: ex archivo.zip | ex archivo.tar.gz
ex() {
    if [ -f "$1" ]; then
        case $1 in
            *.tar.bz2)   tar xjf "$1"   ;;
            *.tar.gz)    tar xzf "$1"   ;;
            *.bz2)       bunzip2 "$1"   ;;
            *.rar)       unrar x "$1"   ;;
            *.gz)        gunzip "$1"    ;;
            *.tar)       tar xf "$1"    ;;
            *.tbz2)      tar xjf "$1"   ;;
            *.tgz)       tar xzf "$1"   ;;
            *.zip)       unzip "$1"     ;;
            *.Z)         uncompress "$1";;
            *.7z)        7z x "$1"      ;;
            *)           echo "'$1' formato no reconocido por la función ex()" ;;
        esac
    else
        echo "'$1' no es un archivo válido"
    fi
}

# Forzar IPv4 en Nix (IPv6 del sistema está roto)
export NIX_CURL_FLAGS=-4
