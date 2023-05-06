#!/bin/bash -e

WD=$(readlink -f $(dirname $0))
BD=$WD/build/release

echo "Working dir: $WD"
echo "Build dir: $BD"

## Setup build directory

cd $WD
meson setup --buildtype release --wipe $BD

## Build package

cd $BD
meson compile
./debian/build-deb.sh

## Check package

VERSION=$(meson introspect --projectinfo | python3 -c "
import sys, json
obj = json.load(sys.stdin)
print(obj['version'])
")

echo "Version: $VERSION"

diff <(dpkg -c autobright-$VERSION.deb | awk '{ print $6 }') <(cat <<EOF
./
./usr/
./usr/local/
./usr/local/bin/
./usr/local/bin/autobright
./usr/local/lib/
./usr/local/lib/autobright/
./usr/local/lib/autobright/debugui/
./usr/local/lib/autobright/debugui/app.js
./usr/local/lib/autobright/debugui/dbusProxyUtils.js
./usr/local/lib/autobright/debugui/debugWindow.js
./usr/local/lib/autobright/debugui/global.d.ts
./usr/local/lib/autobright/debugui/jsconfig.json
./usr/local/lib/systemd/
./usr/local/lib/systemd/user/
./usr/local/lib/systemd/user/fragoi-autobright.service
./usr/local/share/
./usr/local/share/applications/
./usr/local/share/applications/autobright.desktop
./usr/local/share/glib-2.0/
./usr/local/share/glib-2.0/schemas/
./usr/local/share/glib-2.0/schemas/fragoi.autobright.gschema.xml
EOF
)

## Install package

echo "Type in your password to install:"
sudo -k dpkg -i autobright-$VERSION.deb
