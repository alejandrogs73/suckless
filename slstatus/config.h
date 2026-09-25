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
    { wifi_essid,       "[ WiFi: %s ] ",        "wlp4s0" },
    { battery_perc,     "[ BAT: %s%% ] ",       "BAT1" },
    { run_command,      "[ VOL: %s ] ",         "v=$(pactl get-sink-volume @DEFAULT_SINK@ | awk '{print $5}'); m=$(pactl get-sink-mute @DEFAULT_SINK@ | awk '{print $2}'); [ \"$m\" = \"yes\" ] && echo \"$v MUT\" || echo \"$v\"" },
    { run_command,      "[ BT: %s ] ",          "bluetoothctl devices Connected | head -1 | cut -d' ' -f3-" },
    { datetime,         "[ %s ]",               "%d/%m/%y %H:%M:%S" },
};
