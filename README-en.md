# suckless

[Español](README.md) · **English**

My dotfiles for Void Linux: dwm, st, dmenu, slstatus, slock, scroll and
clipmenu, plus the session setup (xinitrc, bash, picom, dunst...).
Everforest theme and Fira Code font everywhere.

Scripts, install steps and some folders have Spanish names (`enlaces`,
`volumen`, `marcadores`...). They are kept as they are because those are the
real names you type; this README explains what each one does.

## Layout

| Folder | Contents |
|---|---|
| `suckless/` | The programs, each with its own `config.h` and patches |
| `home/` | Files that go in `~`, at the same path (they are symlinked) |
| `sistema/` | Files that go in `/` (acpid, zzz, turnstile, udev, doas, xbps, runit) |
| `install.sh` | Installs everything on a fresh Void system |

## Install

On a fresh Void install, as your normal user (it asks for the root
password when needed: through sudo until doas is ready, then through doas):

    git clone ssh://forgejo@ssh.alejandrogs.es/alejandrogs73/suckless.git ~/suckless
    cd ~/suckless && ./install.sh

It runs seven steps, which can also be run on their own (`./install.sh enlaces`,
for example) and repeated safely:

1. `paquetes` (packages): installs with xbps everything the setup uses.
2. `suckless`: builds the programs and installs them in `/usr/local`.
3. `enlaces` (links): symlinks every file in `home/` into `~`. If a different
   file was already there, it is kept as `.bak`. Since they are symlinks,
   editing `~/.bashrc` edits the repo. Links to files deleted from the repo
   are removed. It also creates the user folders and
   sets `~/.ssh` to 700.
4. `gtk`: builds the [Everforest GTK](https://github.com/Fausto-Korpsvart/Everforest-GTK-Theme)
   theme (green, dark, medium palette) in `~/.themes` from a pinned commit,
   links it for GTK 4 apps and makes the Papirus folders green. Icons are
   Papirus-Dark everywhere. Qt 5 and Qt 6 apps use qt5ct/qt6ct
   (`QT_QPA_PLATFORMTHEME` in `.xinitrc`) with the Fusion style and an
   Everforest palette (`home/.config/qt*ct/`). If a `papirus-icon-theme`
   update turns the folders blue again, just run this step again.
5. `gnupg`: for after copying `~/.gnupg` by hand (from a USB stick, for
   example). It fixes the permissions, sets `pinentry-dmenu` with the dmenu
   colours (`pinentry-curses` on a tty: `home/.local/bin/pinentry-menu`
   picks one), enables gpg's SSH agent, adds the authentication subkeys
   to `sshcontrol` and caches the passphrase for one hour. `.bashrc` already
   points `SSH_AUTH_SOCK` at the agent. If `~/.gnupg` does not exist yet, it
   does nothing.
6. `sistema` (system): copies `sistema/` into `/`, sets up
   `wpa_supplicant.conf` so wheel users can use `wpa_cli` without root and
   save networks, enables the runit services (dbus, turnstiled,
   wpa_supplicant, dhcpcd, bluetoothd, acpid, chronyd, tlp, automontaje,
   cupsd), removes NetworkManager, elogind and polkitd (if something asks for
   polkit, like udisks2 or libvirt, D-Bus starts it on its own) and adds the
   user to the audio, video, input, network, bluetooth, lpadmin (printer
   management) and `_pipewire` (realtime priority for audio without rtkit)
   groups. `doas.conf` is checked with `doas -C` before it is installed with
   mode 400.
7. `quitar` (remove): uninstalls what the repo no longer uses because
   something else replaces it: sudo (doas), feh (xwallpaper and nsxiv),
   gammastep (sct), autorandr (`pantallas`), blueman and pavucontrol (the
   `bluetooth` and `volumen` scripts), pinentry-gtk (pinentry-dmenu), vlc
   (mpv), btop, NetworkManager and tlp-rdw (wpa_supplicant, dhcpcd and the
   `wifi` script), rtkit and elogind (turnstile). sudo can be removed thanks
   to `sistema/etc/xbps.d/sin-sudo.conf` (base-system depends on it), and it
   is only removed once `/etc/doas.conf` is installed.

Afterwards, log out and back in (for the groups) and run `startx`.

Nix is not installed: the little that comes from it (webcord) is installed by
hand.

## Without elogind

elogind is systemd's logind on its own, and it is not needed here:

- `XDG_RUNTIME_DIR` (`/run/user/UID`) is created by turnstile at login
  (`sistema/etc/turnstile/turnstiled.conf`: only that, no user services).
- Xorg runs as root through `Xorg.wrap` (`needs_root_rights = yes` in
  `/etc/X11/Xwrapper.config`, shipped by Void).
- The lid and the power and suspend buttons are handled by acpid, which
  calls `zzz`. Power off and reboot are runit's.
- Devices (sound, video, input, bluetooth) are reached through the user's
  groups, not through the ACLs elogind gave the active session.

What stops working: mounting disks from a file manager (udisks2 asks polkit
for an active session, and there is none). USB sticks are mounted by
`automontaje`.

## Rebuilding after changing the config

    cd suckless/dwm && make && doas make install

## Updating from upstream

Each program was imported with `git subtree --squash`:

    git subtree pull --prefix=suckless/dwm https://git.suckless.org/dwm master --squash
    git subtree pull --prefix=suckless/clipmenu https://github.com/cdown/clipmenu develop --squash

The rest work like dwm, changing the name in `--prefix` and in the URL.

## dwm shortcuts

`Mod` is `Super` (it is `Alt` in stock dwm). Mouse bindings use `Super` too.
Only shortcuts that differ from stock dwm or are new are listed; the rest are
unchanged. The st, scroll, dmenu and slock shortcuts are untouched.

### Changed

| Action | Stock | Now |
|---|---|---|
| Open a terminal (`st`) | Mod+Shift+Return | Mod+Return |
| zoom (move the window to the master area) | Mod+Return | Mod+Shift+Return |
| Close window | Mod+Shift+c | Mod+q |
| Quit dwm | Mod+Shift+q | Mod+Shift+m |

### New

| Shortcut | Action |
|---|---|
| Mod+v | clipmenu (same palette and font as dmenu) |
| Mod+Shift+l | Lock the screen with `slock` |
| Mod+n | Show the last notification again (dunst history) |
| Mod+Shift+n | Close all notifications |
| Mod+Shift+Escape | Session menu: lock, suspend, quit dwm, reboot or power off (the last three ask for confirmation) |
| Mod+`-` / Mod+`+` | Shrink / grow the gaps |
| Mod+Shift+`+` | Gaps to 0 |
| Volume up / down key | Volume ±5% with `wpctl` (100% max) |
| Mute key | Mute or unmute the audio |
| Mic mute key or Mod+ñ | Mute or unmute the microphone (ñ is the key right of L on a Spanish keyboard) |
| Print Screen | Full screenshot with `maim` |
| Shift+Print Screen | Screenshot of a region |

The audio keys (and clicks on VOL in the bar) use the `volumen` script
(`home/.local/bin/volumen`): it changes the volume with `wpctl`, refreshes
slstatus and shows a notification with the level.

The script menus (`apagado`, `bluetooth`, `volumen salida`...) use `menu`,
which is dmenu with the dwm font and colours.

Screenshots are saved in `~/Images/Screenshots`, copied to the clipboard and
shown in a notification.

## Bar (slstatus)

    [ CPU: 12% ] [ RAM: 40% ] [ WiFi: Livebox6 ] [ BAT: 80% + ] [ VOL: 50% ] [ BT: OFF ] [ 28/09/26 12:30 ]

The network, VOL, BT and date blocks are clickable (dwm's `statuscmd`
patch). What each click does is in `statuscmds`, in `suckless/dwm/config.h`,
and which area is which is marked by the `\001`...`\004` bytes in
`suckless/slstatus/config.h`.

| Block | Shows | Left | Middle | Right | Wheel |
|---|---|---|---|---|---|
| CPU, RAM | Usage in % | | | | |
| Network | `ETH` on cable; otherwise `WiFi:` and the network | `wifi` menu: networks by signal, connect, ask for and save the password | | `wpa_cli` in a floating st | |
| BAT | Percentage and state (`+` charging, `-` discharging) | | | | |
| VOL | Volume; `MUT` if muted, `MIC OFF` if the mic is | Pick the audio output (`volumen salida`) | Mute | Mute the mic | Up / down |
| BT | `OFF`, or the connected device and its battery if it reports it (`VJ 901 100%`) | `bluetooth` menu: connect, disconnect, scan and pair, power off | | Power on or off | |
| Date | Day and time | This month's calendar (`calendario`) | | | |

After changing dwm's `config.h`, reinstall and restart it for new clicks to
work: `pkill -HUP dwm` restarts it without closing windows (restartsig and
preserveonrestart patches).

## Session

- Logging in on tty1 starts `startx` automatically (`.bash_profile`); other
  ttys do not.
- `.xinitrc` also starts `luz` (warm light with `sct` from 20:00 to 8:00,
  with a one-hour transition) and `bateria` (warns at 15 % and, in red, at
  5 %; at 3 % it suspends with `doas -n zzz`). Suspend, power off and reboot
  need no password (`sistema/etc/doas.conf`).
- Screens: `pantallas` (`sistema/usr/local/bin`) lines them up with `xrandr`
  (the laptop one on the left and as primary) at startup and whenever one is
  plugged or unplugged (udev rule in `sistema/`), then repaints the
  wallpaper with `xwallpaper`.
- WiFi: `wifi` scans with `wpa_cli` and lists the networks in dmenu by
  signal (`*` is the current one). For a new network it asks for the
  password (hidden while typing) and saves it in
  `/etc/wpa_supplicant/wpa_supplicant.conf` only if the connection works.
  dhcpcd handles the cable on its own.
- Left click on the date in the bar: this month's calendar in a notification
  (`calendario`), with today in green.
- bash: 10 000-entry history, without duplicates and shared between
  terminals; the prompt shows the git branch (`*` unstaged changes, `+`
  staged).
- `~/.ssh/config` with aliases: `ssh server` and `forgejo`, both through the
  domain so they work at home and away. No keys in it: gpg-agent provides
  them.
- Firefox bookmarks backup (`marcadores`, started from `.xinitrc`): once a
  day it uploads to the server the backup Firefox already makes on its own
  (`bookmarkbackups/*.jsonlz4`), to `/var/storage/PUBLIC/backup_void/marcadores`:
  `diario/` keeps 7 days and `semanal/` one per week, forever. It only tries
  when the SSH key is already unlocked (so pinentry never pops up) and
  retries every hour. `marcadores ya` uploads it right away. To restore:
  Firefox > Bookmarks > Manage bookmarks > Import and Backup > Restore.
- After 30 minutes idle the screen locks with slock (`xss-lock`) and one
  minute later it turns off.
- USB sticks and SD cards: the `automontaje` service (runit, as root) mounts
  them in `/mnt/LABEL`, or `/mnt/sdXY` if they have no label, and unmounts
  them when removed, with a notification. FAT, exFAT and NTFS are owned by
  the user; internal disks are never touched. Before pulling a stick you
  wrote to, run `sync` (or `doas umount /mnt/...`).
- User folders in English and without accents (`user-dirs.dirs`):
  Documents, Downloads and Images.

## Notes

- The personal config lives in `config.h`. `config.def.h` is the upstream one
  plus the patches. If a patch changes `config.def.h`, the change has to be
  copied to `config.h` by hand: `make` only copies it if `config.h` does not
  exist.
- The applied patches are in `suckless/<program>/patches/`.
  - dwm: fullgaps, restartsig, preserveonrestart, statuscmd-nosignal and
    swallow.
  - st: kitty-graphics, alpha, glyph-wide-support and ligatures, in that
    order. kitty-graphics already includes anysize. glyph-wide-support and
    the combination with ligatures come from the `graphics-with-patches`
    branch of [st-graphics](https://github.com/sergei-grechanik/st-graphics),
    without boxdraw.
- In st, Ctrl+Shift+right click on an image opens it in `nsxiv`.
- st shows images with the kitty graphics protocol; yazi detects it on its
  own and shows previews without ueberzugpp.
- slock is modified by hand (not a patch): clock, date, battery and a bottom
  bar with the state colour.
