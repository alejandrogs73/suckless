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
 * Los bytes \001, \002 y \003 delimitan las zonas clicables de la barra
 * (parche statuscmd de dwm): la red, el volumen y el bluetooth. Lo que hace
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
	{ run_command,      "\002[ VOL: %s ]\002 ",       "wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{ printf \"%d%%\", $2 * 100 + 0.5 } /MUTED/ { printf \" MUT\" }'; "
	                                                  "wpctl get-volume @DEFAULT_AUDIO_SOURCE@ | grep -q MUTED && printf ' MIC OFF'" },
	/* OFF si el adaptador está apagado; si no, el primer dispositivo conectado */
	{ run_command,      "\003[ BT: %s ]\003 ",        "bluetoothctl show | grep -q 'Powered: yes' || { echo OFF; exit; }; "
	                                                  "bluetoothctl devices Connected | head -1 | cut -d' ' -f3-" },
	{ datetime,         "[ %s ]",                     "%d/%m/%y %H:%M" },
};
