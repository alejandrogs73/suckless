/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
const unsigned int interval = 2000;

/* text to show if no value can be retrieved */
static const char unknown_str[] = "N/A";

/* maximum output string length */
#define MAXLEN 2048

/*
 * function            description                     argument (example)
 *
 * battery_perc        battery percentage              battery name (BAT0)
 * battery_state       battery charging state          battery name (BAT0)
 * cpu_perc            cpu usage in percent            NULL
 * datetime            date and time                   format string (%F %T)
 * disk_free           free disk space in GB           mountpoint path (/)
 * ram_perc            ram percentage                  NULL
 * run_command         output of a shell command       command
 * wifi_essid          WiFi network name               interface name (wlan0)
 */
/*
 * Red sin lanzar procesos: ETH si hay cable en enp3s0; si no, la red WiFi de
 * wlp4s0 (se lee por netlink con el componente wifi_essid de slstatus).
 */
static const char *
red(const char *unused)
{
	const char *s;

	if ((s = cat("/sys/class/net/enp3s0/operstate")) && !strcmp(s, "up"))
		return "ETH";
	if (!(s = wifi_essid("wlp4s0")))
		return "WiFi: N/A";
	return bprintf("WiFi: %s", s);
}

/*
 * run_command, pero solo cada LENTO segundos o en cuanto llega SIGUSR1 (lo
 * mandan los scripts volumen, bluetooth y wifi y los clics en la barra). Así
 * VOL y BT no lanzan wpctl y bluetoothctl cada 2 s; lo que cambie por otro
 * lado tarda como mucho LENTO segundos en verse. La cuenta de SIGUSR1 (usr1)
 * la lleva slstatus.c.
 */
#define LENTO 10

static const char *
lento(const char *cmd)
{
	static struct {
		const char *cmd;
		char out[256];
		int ok;
		sig_atomic_t usr1;
		time_t t;
	} c[4];
	struct timespec ahora;
	const char *res;
	size_t i;

	clock_gettime(CLOCK_MONOTONIC, &ahora);
	for (i = 0; i < LEN(c) && c[i].cmd && c[i].cmd != cmd; i++)
		;
	if (i == LEN(c))
		return run_command(cmd);
	if (c[i].cmd && c[i].usr1 == usr1 && ahora.tv_sec - c[i].t < LENTO)
		return c[i].ok ? c[i].out : NULL;

	c[i].cmd = cmd;
	c[i].usr1 = usr1;
	c[i].t = ahora.tv_sec;
	if ((c[i].ok = (res = run_command(cmd)) != NULL))
		snprintf(c[i].out, sizeof(c[i].out), "%s", res);
	return c[i].ok ? c[i].out : NULL;
}

/*
 * Los bytes \001 a \004 delimitan las zonas clicables de la barra
 * (parche statuscmd de dwm): la red, el volumen, el bluetooth y la fecha. Lo que hace
 * cada clic está en statuscmds, en el config.h de dwm.
 */
static const struct arg args[] = {
	/* function         format                        argument */
	{ cpu_perc,         "[ CPU: %s%% ] ",             NULL },
	{ ram_perc,         "[ RAM: %s%% ] ",             NULL },
	{ red,              "\001[ %s ]\001 ",            NULL },
	{ battery_perc,     "[ BAT: %s%%",                "BAT1" },
	{ battery_state,    " %s ] ",                     "BAT1" },
	/* mismo volumen que maneja wpctl (PipeWire); MIC OFF si el micro está silenciado */
	{ lento,            "\002[ VOL: %s ]\002 ",       "wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{ printf \"%d%%\", $2 * 100 + 0.5 } /MUTED/ { printf \" MUT\" }'; "
	                                                  "wpctl get-volume @DEFAULT_AUDIO_SOURCE@ | grep -q MUTED && printf ' MIC OFF'" },
	/* OFF si el adaptador está apagado; si no, el primer dispositivo conectado
	 * y su batería, si la da ("Battery Percentage: 0x64 (100)" -> " 100%") */
	{ lento,            "\003[ BT: %s ]\003 ",        "bluetoothctl show | grep -q 'Powered: yes' || { echo OFF; exit; }; "
	                                                  "set -- $(bluetoothctl devices Connected | head -1); [ $# -gt 0 ] || exit; "
	                                                  "m=$2; shift 2; printf %s \"$*\"; "
	                                                  "bluetoothctl info \"$m\" | awk -F'[()]' '/Battery Percentage/ { printf \" %s%%\", $2 }'" },
	{ datetime,         "\004[ %s ]\004",            "%d/%m/%y %H:%M" },
};
