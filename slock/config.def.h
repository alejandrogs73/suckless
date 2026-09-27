/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "black",     /* after initialization */
	[INPUT] =  "#005577",   /* during input */
	[FAILED] = "#CC3333",   /* wrong password */
};

/* lock screen */
static const char *bgcolor  = "black";
static const char *fgcolor  = "#eeeeee";
static const char *dimcolor = "#888888";

static const char *clockfont = "monospace:bold:pixelsize=160";
static const char *datefont  = "monospace:pixelsize=34";
static const char *textfont  = "monospace:pixelsize=22";
static const char *iconfont  = "Symbols Nerd Font Mono:pixelsize=24";

static const int barheight = 10;           /* height of the bottom bar */
static const unsigned int maxdots = 24;    /* max dots shown while typing */

static const char *battery = "BAT0";       /* /sys/class/power_supply/... */
static const int batterylow = 20;          /* percent shown as low */

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;
