/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
const unsigned int interval = 1000;

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
static const struct arg args[] = {
	/* function         format                  argument */
	{ cpu_perc,         "[ CPU: %s%% ] ",       NULL },
	{ ram_perc,         "[ RAM: %s%% ] ",       NULL },
	/* ETH si hay cable en enp3s0; si no, la red WiFi de wlp4s0 */
	{ run_command,      "[ %s ] ",              "if [ \"$(cat /sys/class/net/enp3s0/operstate)\" = up ]; then echo ETH; "
	                                            "else s=$(iw dev wlp4s0 link | sed -n 's/^[[:space:]]*SSID: //p'); echo \"WiFi: ${s:-N/A}\"; fi" },
	{ battery_perc,     "[ BAT: %s%%",          "BAT1" },
	{ battery_state,    " %s ] ",               "BAT1" },
	/* mismo volumen que maneja wpctl (PipeWire); MIC OFF si el micro está silenciado */
	{ run_command,      "[ VOL: %s ] ",         "wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{ printf \"%d%%\", $2 * 100 + 0.5 } /MUTED/ { printf \" MUT\" }'; "
	                                            "wpctl get-volume @DEFAULT_AUDIO_SOURCE@ | grep -q MUTED && printf ' MIC OFF'" },
	{ run_command,      "[ BT: %s ] ",          "bluetoothctl devices Connected | head -1 | cut -d' ' -f3-" },
	{ datetime,         "[ %s ]",               "%d/%m/%y %H:%M:%S" },
};
