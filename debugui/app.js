#!/bin/gjs

imports.gi.versions.Gtk = '3.0';

const { Gtk, GLib, Gio } = imports.gi;

imports.searchPath.unshift(
  GLib.path_get_dirname(new Error().fileName)
);

const { DebugWindow } = imports.debugWindow;
const DBusProxyUtils = imports.dbusProxyUtils;

class App {
  constructor() {
    this.application = new Gtk.Application({
      application_id: 'fragoi.autobright.Debug'
    });
    this.application.connect('startup', this._onStartup.bind(this));
    this.application.connect('activate', this._onActivate.bind(this));
    this._proxy = null;
  }

  _onStartup() {
    this._proxy = new DebugWindowProxy();
    this.application.add_window(this._proxy.window);
    this._proxy.connect().catch(logError);
  }

  _onActivate() { }
}

class DebugWindowProxy {
  constructor() {
    this._window = new DebugWindow();
    this._proxy = null;
    this._bindPropsId = null;
  }

  get window() {
    return this._window.window;
  }

  async connect() {
    this._proxy = await DBusProxyUtils.newForBus(
      Gio.BusType.SESSION,
      Gio.DBusProxyFlags.NONE,
      null, // info
      'com.github.fragoi.Autobright',
      '/com/github/fragoi/Autobright/Debug',
      'com.github.fragoi.autobright.Debug',
      null // cancellable
    );
    this._proxy.Enable = DBusProxyUtils.newMethod(
      this._proxy, 'Enable'
    );
    this._proxy.Disable = DBusProxyUtils.newMethod(
      this._proxy, 'Disable'
    );
    this._bindPropsId = DBusProxyUtils.bindProps(this._proxy, this._window);

    await this._proxy.Enable();

    this._window.onDestroy = this._onDestroy.bind(this);
    this._window.show();
  }

  _onDestroy() {
    this._proxy.disconnect(this._bindPropsId);
    this._proxy.Disable().catch(logError);
    delete this._window.onDestroy;
  }
}

const app = new App();
GLib.set_prgname('Autobright');
app.application.run(['', ...ARGV]);
