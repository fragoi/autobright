const { Gio, GLib } = imports.gi;

function newForBus(busType, flags, info, name, path, ifaceName, cancellable) {
  return new Promise((resolve, reject) => {
    Gio.DBusProxy.new_for_bus(
      busType,
      flags,
      info,
      name,
      path,
      ifaceName,
      cancellable,
      (_source, res) => {
        try {
          resolve(Gio.DBusProxy.new_for_bus_finish(res));
        } catch (e) {
          reject(e);
        }
      }
    );
  });
}

function call(proxy, methodName, parameters,
  flags = Gio.DBusCallFlags.NONE,
  timeout = -1,
  cancellable = null) {
  return new Promise((resolve, reject) => {
    proxy.call(
      methodName,
      parameters,
      flags,
      timeout,
      cancellable,
      (_source, res) => {
        try {
          resolve(proxy.call_finish(res));
        } catch (e) {
          reject(e);
        }
      }
    );
  });
}

function newMethod(proxy, methodName, paramsTypeString,
  flags = Gio.DBusCallFlags.NONE,
  timeout = -1,
  cancellable = null) {
  return async (...args) => {
    let res = await call(
      proxy,
      methodName,
      paramsTypeString ? new GLib.Variant(paramsTypeString, args) : null,
      flags,
      timeout,
      cancellable
    );
    res = res.recursiveUnpack();
    switch (res.length) {
      case 0: return;
      case 1: return res[0];
      default: return res;
    }
  };
}

function newGetter(proxy, propertyName,
  flags = Gio.DBusCallFlags.NONE,
  timeout = -1,
  cancellable = null) {
  const method = newMethod(
    proxy,
    'org.freedesktop.DBus.Properties.Get',
    '(ss)',
    flags,
    timeout,
    cancellable
  );
  const interfaceName = proxy.get_interface_name();
  return async () => await method(
    interfaceName,
    propertyName
  );
}

function newSetter(proxy, propertyName, propertyType,
  flags = Gio.DBusCallFlags.NONE,
  timeout = -1,
  cancellable = null) {
  const method = newMethod(
    proxy,
    'org.freedesktop.DBus.Properties.Set',
    '(ssv)',
    flags,
    timeout,
    cancellable
  );
  const interfaceName = proxy.get_interface_name();
  return async (value) => await method(
    interfaceName,
    propertyName,
    new GLib.Variant(propertyType, value)
  );
}

function bindProps(proxy, target) {
  const handlerId = bindPropsChanged(proxy, target);
  copyProps(proxy, target);
  return handlerId;
}

function copyProps(proxy, target) {
  const props = proxy.get_cached_property_names();
  if (!props) {
    return;
  }
  for (const p of props) {
    if (p in target) {
      const value = proxy.get_cached_property(p);
      target[p] = value ? value.recursiveUnpack() : null;
    }
  }
}

function bindPropsChanged(proxy, target) {
  const handler = onPropsChangedCopyTo(target);
  return proxy.connect('g-properties-changed', handler);
}

function onPropsChangedCopyTo(target) {
  return (_proxy, changedProps, _invalidatedProps) => {
    changedProps = changedProps.unpack();
    for (const p in changedProps) {
      if (p in target) {
        target[p] = changedProps[p].recursiveUnpack();
      }
    }
  };
}

function bindSignals(proxy, target) {
  const handler = onSignalCallOn(target);
  return proxy.connect('g-signal', handler);
}

function onSignalCallOn(target) {
  return (_proxy, _senderName, signalName, parameters) => {
    if (target[signalName]) {
      const args = parameters.recursiveUnpack();
      target[signalName](...args);
    }
  };
}

if (typeof module === 'object') {
  module.exports = {
    __esModule: true,
    newForBus,
    call,
    newMethod,
    newGetter,
    newSetter,
    bindProps,
    copyProps,
    bindPropsChanged,
    onPropsChangedCopyTo,
    bindSignals,
    onSignalCallOn
  };
}
