/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

/* colores de la barra inferior según el estado */
static const char *colorname[NUMCOLS] = {
	[INIT] =   "#475258",   /* after initialization (gris Everforest) */
	[INPUT] =  "#a7c080",   /* during input (verde acento) */
	[FAILED] = "#e67e80",   /* wrong password (rojo Everforest) */
};

/* pantalla de bloqueo (paleta Everforest) */
static const char *bgcolor  = "#2d353b";   /* fondo */
static const char *fgcolor  = "#d3c6aa";   /* reloj y texto */
static const char *dimcolor = "#859289";   /* fecha y avisos */

static const char *clockfont = "Fira Code:style=Bold:pixelsize=160:antialias=true";
static const char *datefont  = "Fira Code:pixelsize=34:antialias=true";
static const char *textfont  = "Fira Code:pixelsize=22:antialias=true";
static const char *iconfont  = "Symbols Nerd Font Mono:pixelsize=24:antialias=true";

static const int barheight = 10;           /* alto de la barra inferior */
static const unsigned int maxdots = 24;    /* puntos máximos al teclear */

static const char *battery = "BAT1";       /* /sys/class/power_supply/... */
static const int batterylow = 20;          /* % para mostrarla en rojo */

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;
