/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 4;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "Fira Code:size=11:antialias=true:autohint=true" };
static const char dmenufont[]       = "Fira Code:size=11:antialias=true:autohint=true";

/* Paleta Everforest */
static const char col_bg[]          = "#2d353b"; /* Fondo general */
static const char col_fg[]          = "#d3c6aa"; /* Texto general */
static const char col_border[]      = "#475258"; /* Borde ventana inactiva */
static const char col_accent[]      = "#a7c080"; /* Verde acento (borde activo y barra) */
static const char col_accent_fg[]   = "#2d353b"; /* Texto sobre verde acento */

static const char *colors[][3]      = {
	/*               fg             bg          border     */
	[SchemeNorm] = { col_fg,        col_bg,     col_border },
	[SchemeSel]  = { col_accent_fg, col_accent, col_accent },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_bg, "-nf", col_fg, "-sb", col_accent, "-sf", col_accent_fg, NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *clipcmd[]  = { "clipmenu", "-fn", dmenufont, "-nb", col_bg, "-nf", col_fg, "-sb", col_accent, "-sf", col_accent_fg, NULL };
static const char *lockcmd[]  = { "slock", NULL };

/* Audio: tras cada cambio se avisa a slstatus (SIGUSR1) para que refresque la barra al momento */
#define STATUSREFRESH "; pkill -USR1 -x slstatus"
#define VOLUP    SHCMD("wpctl set-volume -l 1.0 @DEFAULT_AUDIO_SINK@ 5%+" STATUSREFRESH)
#define VOLDOWN  SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-" STATUSREFRESH)
#define VOLMUTE  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle" STATUSREFRESH)
#define MICMUTE  SHCMD("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle" STATUSREFRESH)

/* Capturas: se guardan en ~/Imágenes/Capturas y se copian al portapapeles */
#define SCREENSHOT(opts) SHCMD( \
	"d=\"$HOME/Imágenes/Capturas\"; mkdir -p \"$d\"; " \
	"f=\"$d/$(date +%Y-%m-%d_%H-%M-%S).png\"; " \
	"maim " opts " \"$f\" && xclip -selection clipboard -t image/png -i \"$f\" " \
	"&& notify-send -i \"$f\" 'Captura guardada' \"$f\"")

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_v,      spawn,          {.v = clipcmd } },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          {.v = lockcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	/* gaps: en teclado español '+' y '-' están sin Shift (XK_equal no existe sin Shift) */
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_plus,   setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_plus,   setgaps,        {.i = 0  } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_m,      quit,           {0} },

	/* audio */
	{ 0,                            XF86XK_AudioRaiseVolume, spawn, VOLUP },
	{ 0,                            XF86XK_AudioLowerVolume, spawn, VOLDOWN },
	{ 0,                            XF86XK_AudioMute,        spawn, VOLMUTE },
	{ 0,                            XF86XK_AudioMicMute,     spawn, MICMUTE },
	{ MODKEY,                       XK_ntilde, spawn,          MICMUTE },

	/* capturas */
	{ 0,                            XK_Print,  spawn,          SCREENSHOT("") },
	{ ShiftMask,                    XK_Print,  spawn,          SCREENSHOT("-s") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

