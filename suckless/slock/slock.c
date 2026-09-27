/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include "util.h"

char *argv0;

enum {
	INIT,
	INPUT,
	FAILED,
	NUMCOLS
};

enum {
	COLBG,
	COLFG,
	COLDIM,
	COLCHARGE,
	COLLOW,
	NUMUICOLS
};

struct lock {
	int screen;
	Window root, win;
	Pixmap pmap;
	unsigned long colors[NUMCOLS];
	/* dibujado: doble búfer del tamaño de la pantalla */
	int w, h;
	Pixmap buf;
	XftDraw *draw;
	XftColor bar[NUMCOLS];
	XftColor ui[NUMUICOLS];
	XftFont *fclock, *fdate, *ftext, *ficon;
};

/* estado que se muestra en la pantalla de bloqueo */
struct state {
	unsigned int len;   /* caracteres tecleados */
	int color;          /* INIT, INPUT o FAILED */
	int failure;        /* ha habido una contraseña incorrecta */
	char clock[16], date[64], bat[16];
	int batlevel, charging;
};

static char username[64];

struct xrandr {
	int active;
	int evbase;
	int errbase;
};

#include "config.h"

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

#ifdef __linux__
#include <fcntl.h>
#include <linux/oom.h>

static void
dontkillme(void)
{
	FILE *f;
	const char oomfile[] = "/proc/self/oom_score_adj";

	if (!(f = fopen(oomfile, "w"))) {
		if (errno == ENOENT)
			return;
		die("slock: fopen %s: %s\n", oomfile, strerror(errno));
	}
	fprintf(f, "%d", OOM_SCORE_ADJ_MIN);
	if (fclose(f)) {
		if (errno == EACCES)
			die("slock: unable to disable OOM killer. "
			    "Make sure to suid or sgid slock.\n");
		else
			die("slock: fclose %s: %s\n", oomfile, strerror(errno));
	}
}
#endif

static const char *
gethash(void)
{
	const char *hash;
	struct passwd *pw;

	/* Check if the current user has a password entry */
	errno = 0;
	if (!(pw = getpwuid(getuid()))) {
		if (errno)
			die("slock: getpwuid: %s\n", strerror(errno));
		else
			die("slock: cannot retrieve password entry\n");
	}
	hash = pw->pw_passwd;

#if HAVE_SHADOW_H
	if (!strcmp(hash, "x")) {
		struct spwd *sp;
		if (!(sp = getspnam(pw->pw_name)))
			die("slock: getspnam: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = sp->sp_pwdp;
	}
#else
	if (!strcmp(hash, "*")) {
#ifdef __OpenBSD__
		if (!(pw = getpwuid_shadow(getuid())))
			die("slock: getpwnam_shadow: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = pw->pw_passwd;
#else
		die("slock: getpwuid: cannot retrieve shadow entry. "
		    "Make sure to suid or sgid slock.\n");
#endif /* __OpenBSD__ */
	}
#endif /* HAVE_SHADOW_H */

	return hash;
}

static XftFont *
loadfont(Display *dpy, int screen, const char *name)
{
	XftFont *f;

	if (!(f = XftFontOpenName(dpy, screen, name)) &&
	    !(f = XftFontOpenName(dpy, screen, "monospace")))
		die("slock: cannot load font %s\n", name);
	return f;
}

static void
setupdraw(Display *dpy, struct lock *lock)
{
	Visual *vis = DefaultVisual(dpy, lock->screen);
	Colormap cmap = DefaultColormap(dpy, lock->screen);
	const char *uinames[NUMUICOLS] = {
		[COLBG] = bgcolor, [COLFG] = fgcolor, [COLDIM] = dimcolor,
		[COLCHARGE] = colorname[INPUT], [COLLOW] = colorname[FAILED],
	};
	int i;

	for (i = 0; i < NUMCOLS; i++)
		if (!XftColorAllocName(dpy, vis, cmap, colorname[i], &lock->bar[i]))
			die("slock: cannot allocate color %s\n", colorname[i]);
	for (i = 0; i < NUMUICOLS; i++)
		if (!XftColorAllocName(dpy, vis, cmap, uinames[i], &lock->ui[i]))
			die("slock: cannot allocate color %s\n", uinames[i]);

	lock->fclock = loadfont(dpy, lock->screen, clockfont);
	lock->fdate = loadfont(dpy, lock->screen, datefont);
	lock->ftext = loadfont(dpy, lock->screen, textfont);
	lock->ficon = loadfont(dpy, lock->screen, iconfont);

	lock->w = DisplayWidth(dpy, lock->screen);
	lock->h = DisplayHeight(dpy, lock->screen);
	lock->buf = XCreatePixmap(dpy, lock->root, lock->w, lock->h,
	                          DefaultDepth(dpy, lock->screen));
	lock->draw = XftDrawCreate(dpy, lock->buf, vis, cmap);
}

static void
resizedraw(Display *dpy, struct lock *lock, int w, int h)
{
	if (w == lock->w && h == lock->h)
		return;
	lock->w = w;
	lock->h = h;
	XFreePixmap(dpy, lock->buf);
	lock->buf = XCreatePixmap(dpy, lock->root, w, h,
	                          DefaultDepth(dpy, lock->screen));
	XftDrawChange(lock->draw, lock->buf);
}

static int
textw(Display *dpy, XftFont *f, const char *s)
{
	XGlyphInfo ext;

	XftTextExtentsUtf8(dpy, f, (const FcChar8 *)s, strlen(s), &ext);
	return ext.xOff;
}

static int
iconw(Display *dpy, XftFont *f, FcChar32 icon)
{
	XGlyphInfo ext;

	if (!icon || !XftCharExists(dpy, f, icon))
		return 0;
	XftTextExtents32(dpy, f, &icon, 1, &ext);
	return ext.xOff;
}

/* icono (opcional) + separación + texto, con la línea base en y */
static int
drawlabel(Display *dpy, struct lock *lock, XftFont *f, XftColor *c,
          int x, int y, FcChar32 icon, const char *s)
{
	int iw = iconw(dpy, lock->ficon, icon), gap = iw ? f->height / 2 : 0;

	if (iw)
		XftDrawString32(lock->draw, c, lock->ficon, x, y, &icon, 1);
	XftDrawStringUtf8(lock->draw, c, f, x + iw + gap, y,
	                  (const FcChar8 *)s, strlen(s));
	return iw + gap + textw(dpy, f, s);
}

static int
labelw(Display *dpy, struct lock *lock, XftFont *f, FcChar32 icon,
       const char *s)
{
	int iw = iconw(dpy, lock->ficon, icon);

	return iw + (iw ? f->height / 2 : 0) + textw(dpy, f, s);
}

/* Actualiza hora, fecha y batería. Devuelve 1 si algo ha cambiado. */
static int
updatestate(struct state *st)
{
	static const char *wdays[] = { "domingo", "lunes", "martes",
		"miércoles", "jueves", "viernes", "sábado" };
	static const char *months[] = { "enero", "febrero", "marzo", "abril",
		"mayo", "junio", "julio", "agosto", "septiembre", "octubre",
		"noviembre", "diciembre" };
	char clk[sizeof(st->clock)], date[sizeof(st->date)], bat[sizeof(st->bat)];
	char path[128], status[32] = "";
	int level = -1, charging = 0, changed;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	FILE *f;

	strftime(clk, sizeof(clk), "%H:%M", tm);
	snprintf(date, sizeof(date), "%s, %d de %s", wdays[tm->tm_wday],
	         tm->tm_mday, months[tm->tm_mon]);
	date[0] = toupper((unsigned char)date[0]);

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity",
	         battery);
	if ((f = fopen(path, "r"))) {
		if (fscanf(f, "%d", &level) != 1)
			level = -1;
		fclose(f);
	}
	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status",
	         battery);
	if ((f = fopen(path, "r"))) {
		if (fscanf(f, "%31s", status) != 1)
			status[0] = '\0';
		fclose(f);
	}
	charging = !strcmp(status, "Charging") || !strcmp(status, "Full");
	if (level >= 0)
		snprintf(bat, sizeof(bat), "%d%%", level);
	else
		bat[0] = '\0';

	changed = strcmp(clk, st->clock) || strcmp(date, st->date) ||
	          strcmp(bat, st->bat) || charging != st->charging;
	memcpy(st->clock, clk, sizeof(clk));
	memcpy(st->date, date, sizeof(date));
	memcpy(st->bat, bat, sizeof(bat));
	st->batlevel = level;
	st->charging = charging;
	return changed;
}

/* Símbolos de Nerd Font (Material Design) */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ICON_LOCK     0xF033E
#define ICON_ALERT    0xF0026
#define ICON_USER     0xF0004
#define ICON_CHARGING 0xF0084
#define ICON_BAT10    0xF007A   /* 10 % .. 90 % seguidos; 100 % es 0xF0079 */
#define ICON_BAT100   0xF0079

static FcChar32
baticon(const struct state *st)
{
	if (st->charging)
		return ICON_CHARGING;
	if (st->batlevel >= 95)
		return ICON_BAT100;
	if (st->batlevel < 10)
		return ICON_ALERT;
	return ICON_BAT10 + st->batlevel / 10 - 1;
}

/* Dibuja la pantalla de bloqueo en el rectángulo de un monitor */
static void
drawmon(Display *dpy, struct lock *lock, const struct state *st,
        int mx, int my, int mw, int mh)
{
	XftColor *fg = &lock->ui[COLFG], *dim = &lock->ui[COLDIM], *msgc;
	char msg[128];
	FcChar32 icon = 0;
	int x, y, w, pad = mh / 24, low;
	unsigned int i, n;

	/* reloj y fecha, algo por encima del centro */
	y = my + mh * 2 / 5;
	x = mx + (mw - textw(dpy, lock->fclock, st->clock)) / 2;
	XftDrawStringUtf8(lock->draw, fg, lock->fclock, x, y,
	                  (const FcChar8 *)st->clock, strlen(st->clock));
	y += lock->fdate->height * 3 / 2;
	x = mx + (mw - textw(dpy, lock->fdate, st->date)) / 2;
	XftDrawStringUtf8(lock->draw, dim, lock->fdate, x, y,
	                  (const FcChar8 *)st->date, strlen(st->date));

	/* mensaje de estado: indicación, puntos o error */
	msgc = dim;
	if (st->color == INPUT) {
		n = MIN(st->len, maxdots);
		for (i = 0, msg[0] = '\0'; i < n; i++)
			strcat(msg, i ? " ●" : "●");
		msgc = &lock->bar[INPUT];
	} else if (st->color == FAILED && st->failure) {
		icon = ICON_ALERT;
		snprintf(msg, sizeof(msg), "Contraseña incorrecta");
		msgc = &lock->bar[FAILED];
	} else {
		icon = ICON_LOCK;
		snprintf(msg, sizeof(msg), "Escribe la contraseña");
	}
	y = my + mh * 3 / 5 + lock->ftext->ascent;
	x = mx + (mw - labelw(dpy, lock, lock->ftext, icon, msg)) / 2;
	drawlabel(dpy, lock, lock->ftext, msgc, x, y, icon, msg);

	/* abajo: usuario a la izquierda, batería a la derecha */
	y = my + mh - barheight - pad;
	drawlabel(dpy, lock, lock->ftext, dim, mx + pad, y, ICON_USER, username);
	if (st->bat[0]) {
		low = !st->charging && st->batlevel <= batterylow;
		w = labelw(dpy, lock, lock->ftext, baticon(st), st->bat);
		drawlabel(dpy, lock, lock->ftext,
		          low ? &lock->ui[COLLOW] :
		          st->charging ? &lock->ui[COLCHARGE] : fg,
		          mx + mw - pad - w, y, baticon(st), st->bat);
	}

	/* barra con el color del estado */
	XftDrawRect(lock->draw, &lock->bar[st->color], mx, my + mh - barheight,
	            mw, barheight);
}

static void
drawlock(Display *dpy, struct lock *lock, const struct state *st)
{
	XRRMonitorInfo *mons;
	int i, n = 0;

	XftDrawRect(lock->draw, &lock->ui[COLBG], 0, 0, lock->w, lock->h);
	mons = XRRGetMonitors(dpy, lock->root, True, &n);
	for (i = 0; i < n; i++)
		drawmon(dpy, lock, st, mons[i].x, mons[i].y,
		        mons[i].width, mons[i].height);
	if (mons)
		XRRFreeMonitors(mons);
	if (n <= 0)
		drawmon(dpy, lock, st, 0, 0, lock->w, lock->h);
	XCopyArea(dpy, lock->buf, lock->win, DefaultGC(dpy, lock->screen),
	          0, 0, lock->w, lock->h, 0, 0);
}

static void
drawall(Display *dpy, struct lock **locks, int nscreens,
        const struct state *st)
{
	int screen;

	for (screen = 0; screen < nscreens; screen++)
		drawlock(dpy, locks[screen], st);
	XFlush(dpy);
}

static void
readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens,
       const char *hash)
{
	XRRScreenChangeNotifyEvent *rre;
	char buf[32], passwd[256], *inputhash;
	int num, screen, running, xfd;
	struct state st = { 0 };
	struct timeval tv;
	KeySym ksym;
	XEvent ev;
	fd_set fds;

	running = 1;
	st.color = INIT;
	xfd = ConnectionNumber(dpy);
	updatestate(&st);
	drawall(dpy, locks, nscreens, &st);

	while (running) {
		/* sin eventos pendientes: esperar como mucho 1 s y refrescar
		 * reloj y batería si han cambiado (también tras suspender) */
		if (!XPending(dpy)) {
			FD_ZERO(&fds);
			FD_SET(xfd, &fds);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			if (select(xfd + 1, &fds, NULL, NULL, &tv) <= 0 &&
			    updatestate(&st))
				drawall(dpy, locks, nscreens, &st);
			continue;
		}
		XNextEvent(dpy, &ev);
		if (ev.type == KeyPress) {
			explicit_bzero(&buf, sizeof(buf));
			num = XLookupString(&ev.xkey, buf, sizeof(buf), &ksym, 0);
			if (IsKeypadKey(ksym)) {
				if (ksym == XK_KP_Enter)
					ksym = XK_Return;
				else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
					ksym = (ksym - XK_KP_0) + XK_0;
			}
			if (IsFunctionKey(ksym) ||
			    IsKeypadKey(ksym) ||
			    IsMiscFunctionKey(ksym) ||
			    IsPFKey(ksym) ||
			    IsPrivateKeypadKey(ksym))
				continue;
			switch (ksym) {
			case XK_Return:
				passwd[st.len] = '\0';
				errno = 0;
				if (!(inputhash = crypt(passwd, hash)))
					fprintf(stderr, "slock: crypt: %s\n", strerror(errno));
				else
					running = !!strcmp(inputhash, hash);
				if (running) {
					XBell(dpy, 100);
					st.failure = 1;
				}
				explicit_bzero(&passwd, sizeof(passwd));
				st.len = 0;
				break;
			case XK_Escape:
				explicit_bzero(&passwd, sizeof(passwd));
				st.len = 0;
				break;
			case XK_BackSpace:
				if (st.len)
					passwd[--st.len] = '\0';
				break;
			default:
				if (num && !iscntrl((unsigned char)buf[0]) &&
				    (st.len + num < sizeof(passwd))) {
					memcpy(passwd + st.len, buf, num);
					st.len += num;
				} else if (buf[0] == '\025') { /* ctrl-u clears input */
					explicit_bzero(&passwd, sizeof(passwd));
					st.len = 0;
				}
				break;
			}
			st.color = st.len ? INPUT :
			           ((st.failure || failonclear) ? FAILED : INIT);
			if (running) {
				updatestate(&st);
				drawall(dpy, locks, nscreens, &st);
			}
		} else if (ev.type == Expose) {
			if (ev.xexpose.count == 0)
				drawall(dpy, locks, nscreens, &st);
		} else if (rr->active && ev.type == rr->evbase + RRScreenChangeNotify) {
			rre = (XRRScreenChangeNotifyEvent*)&ev;
			for (screen = 0; screen < nscreens; screen++) {
				if (locks[screen]->win == rre->window) {
					if (rre->rotation == RR_Rotate_90 ||
					    rre->rotation == RR_Rotate_270) {
						XResizeWindow(dpy, locks[screen]->win,
						              rre->height, rre->width);
						resizedraw(dpy, locks[screen],
						           rre->height, rre->width);
					} else {
						XResizeWindow(dpy, locks[screen]->win,
						              rre->width, rre->height);
						resizedraw(dpy, locks[screen],
						           rre->width, rre->height);
					}
					drawlock(dpy, locks[screen], &st);
					break;
				}
			}
		} else {
			for (screen = 0; screen < nscreens; screen++)
				XRaiseWindow(dpy, locks[screen]->win);
		}
	}
}

static struct lock *
lockscreen(Display *dpy, struct xrandr *rr, int screen)
{
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, ptgrab, kbgrab;
	struct lock *lock;
	XColor color, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;

	if (dpy == NULL || screen < 0 || !(lock = malloc(sizeof(struct lock))))
		return NULL;

	lock->screen = screen;
	lock->root = RootWindow(dpy, lock->screen);

	for (i = 0; i < NUMCOLS; i++) {
		XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen),
		                 colorname[i], &color, &dummy);
		lock->colors[i] = color.pixel;
	}

	setupdraw(dpy, lock);

	/* init */
	wa.override_redirect = 1;
	wa.background_pixel = lock->ui[COLBG].pixel;
	wa.event_mask = ExposureMask;
	lock->win = XCreateWindow(dpy, lock->root, 0, 0,
	                          DisplayWidth(dpy, lock->screen),
	                          DisplayHeight(dpy, lock->screen),
	                          0, DefaultDepth(dpy, lock->screen),
	                          CopyFromParent,
	                          DefaultVisual(dpy, lock->screen),
	                          CWOverrideRedirect | CWBackPixel | CWEventMask,
	                          &wa);
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap,
	                                &color, &color, 0, 0);
	XDefineCursor(dpy, lock->win, invisible);

	/* Try to grab mouse pointer *and* keyboard for 600ms, else fail the lock */
	for (i = 0, ptgrab = kbgrab = -1; i < 6; i++) {
		if (ptgrab != GrabSuccess) {
			ptgrab = XGrabPointer(dpy, lock->root, False,
			                      ButtonPressMask | ButtonReleaseMask |
			                      PointerMotionMask, GrabModeAsync,
			                      GrabModeAsync, None, invisible, CurrentTime);
		}
		if (kbgrab != GrabSuccess) {
			kbgrab = XGrabKeyboard(dpy, lock->root, True,
			                       GrabModeAsync, GrabModeAsync, CurrentTime);
		}

		/* input is grabbed: we can lock the screen */
		if (ptgrab == GrabSuccess && kbgrab == GrabSuccess) {
			XMapRaised(dpy, lock->win);
			if (rr->active)
				XRRSelectInput(dpy, lock->win, RRScreenChangeNotifyMask);

			XSelectInput(dpy, lock->root, SubstructureNotifyMask);
			return lock;
		}

		/* retry on AlreadyGrabbed but fail on other errors */
		if ((ptgrab != AlreadyGrabbed && ptgrab != GrabSuccess) ||
		    (kbgrab != AlreadyGrabbed && kbgrab != GrabSuccess))
			break;

		usleep(100000);
	}

	/* we couldn't grab all input: fail out */
	if (ptgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab mouse pointer for screen %d\n",
		        screen);
	if (kbgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab keyboard for screen %d\n",
		        screen);
	return NULL;
}

static void
usage(void)
{
	die("usage: slock [-v]\n");
}

int
main(int argc, char **argv) {
	struct xrandr rr;
	struct lock **locks;
	struct passwd *pwd;
	struct group *grp;
	uid_t duid;
	gid_t dgid;
	const char *hash;
	Display *dpy;
	int s, nlocks, nscreens;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		puts("slock-"VERSION);
		return 0;
	} else if (argc > 1) {
		usage();
	}

	/* validate drop-user and -group */
	errno = 0;
	if (!(pwd = getpwnam(user)))
		die("slock: getpwnam %s: %s\n", user,
		    errno ? strerror(errno) : "user entry not found");
	duid = pwd->pw_uid;
	errno = 0;
	if (!(grp = getgrnam(group)))
		die("slock: getgrnam %s: %s\n", group,
		    errno ? strerror(errno) : "group entry not found");
	dgid = grp->gr_gid;

#ifdef __linux__
	dontkillme();
#endif

	/* nombre del usuario que bloquea, para mostrarlo */
	if ((pwd = getpwuid(getuid())))
		snprintf(username, sizeof(username), "%s", pwd->pw_name);

	hash = gethash();
	errno = 0;
	if (!crypt("", hash))
		die("slock: crypt: %s\n", strerror(errno));

	if (!(dpy = XOpenDisplay(NULL)))
		die("slock: cannot open display\n");

	/* drop privileges */
	if (setgroups(0, NULL) < 0)
		die("slock: setgroups: %s\n", strerror(errno));
	if (setgid(dgid) < 0)
		die("slock: setgid: %s\n", strerror(errno));
	if (setuid(duid) < 0)
		die("slock: setuid: %s\n", strerror(errno));

	/* check for Xrandr support */
	rr.active = XRRQueryExtension(dpy, &rr.evbase, &rr.errbase);

	/* get number of screens in display "dpy" and blank them */
	nscreens = ScreenCount(dpy);
	if (!(locks = calloc(nscreens, sizeof(struct lock *))))
		die("slock: out of memory\n");
	for (nlocks = 0, s = 0; s < nscreens; s++) {
		if ((locks[s] = lockscreen(dpy, &rr, s)) != NULL)
			nlocks++;
		else
			break;
	}
	XSync(dpy, 0);

	/* did we manage to lock everything? */
	if (nlocks != nscreens)
		return 1;

	/* everything is now blank. Wait for the correct password */
	readpw(dpy, &rr, locks, nscreens, hash);

	return 0;
}
