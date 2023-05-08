# Autobright

Automatically adjust screen brightness using light sensor.

Uses D-Bus to gather informations and change brightness:
* iio-sensor-proxy to read light level (`net.hadess.SensorProxy`)
* Gnome power settings to change brightness and listen for manual user adjustments (`org.gnome.SettingsDaemon.Power`)
* Mutter idle monitor to know when user is not active and brightness changes may be made by the system, for example because of power saving settings (`org.gnome.Mutter.IdleMonitor`)

Uses GSettings to store manual brightness adjustments (`fragoi.autobright`)

## Install

TODO: publish PPA package and add instructions

## Compile

Uses Meson build system.

Example for a recent version of meson:

```
meson setup buiddir
meson compile -C buiddir
```

### Debian

When option `debian` is true, a shell script is generated in the build directory to create a debian package.
The package is created inside of the build directory where the script was compiled.

```
<path_to_builddir>/debian/build-deb.sh
```

The package will be in `<path_to_builddir>`.

### Systemd

Service should be started as a user service.

The following commands are required to be run only once after installation.

**None of the following commands should be run as root.**

Refresh the daemon cache:

```
systemctl --user daemon-reload
```

Enable service:

```
systemctl --user enable fragoi-autobright.service
```

Start the service:

```
systemctl --user start fragoi-autobright.service
```
