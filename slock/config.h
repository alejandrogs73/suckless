/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "#2d353b",   /* after initialization (fondo Everforest) */
	[INPUT] =  "#a7c080",   /* during input (verde acento) */
	[FAILED] = "#e67e80",   /* wrong password (rojo Everforest) */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;
