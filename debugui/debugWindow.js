const { Gtk } = imports.gi;

var DebugWindow = class {
  constructor() {
    const lightLevelLabel = this._newLabel('Light Level (lux)');
    const normalizedLabel = this._newLabel('Normalized');
    const pressureLabel = this._newLabel('Pressure');
    const filteredLabel = this._newLabel('Filtered');
    const valueLabel = this._newLabel('Value');
    const offsetLabel = this._newLabel('Offset');
    const brightnessLabel = this._newLabel('Brightness');
    const flagsLabel = this._newLabel('Flags');
    const windowLogLabel = this._newLabel('Window Log');

    this._lightLevelValue = this._newLabel('', Gtk.Align.END);
    this._normalizedSlider = this._newSlider();
    this._pressureSlider = this._newSlider(-100);
    this._filteredSlider = this._newSlider();
    this._valueSlider = this._newSlider();
    this._offsetSlider = this._newSlider(-100);
    this._brightnessSlider = this._newSlider();
    this._flagsValue = this._newLabel('', Gtk.Align.END);
    this._windowLogSwitch = this._newSwitch(
      this._toggleWindowLog.bind(this)
    );

    const grid = this._newGrid([
      [lightLevelLabel, this._lightLevelValue],
      [normalizedLabel, this._normalizedSlider],
      [pressureLabel, this._pressureSlider],
      [filteredLabel, this._filteredSlider],
      [valueLabel, this._valueSlider],
      [offsetLabel, this._offsetSlider],
      [brightnessLabel, this._brightnessSlider],
      [flagsLabel, this._flagsValue],
      [windowLogLabel, this._windowLogSwitch]
    ]);

    const box = new Gtk.Box();
    box.set_orientation(Gtk.Orientation.VERTICAL);
    box.add(grid);

    this._logWindow = new LogWindow(box);

    const window = new Gtk.Window();
    window.set_title('Autobright');
    window.set_default_size(500, -1);
    window.connect('destroy', () => this.onDestroy());
    window.add(box);

    this.window = window;
  }

  /**
   * @param {string} text
   */
  _newLabel(text, halign = Gtk.Align.START) {
    const label = new Gtk.Label({ label: text });
    label.set_halign(halign);
    return label;
  }

  _newSlider(min = 0, max = 100) {
    const slider = new Gtk.Scale();
    slider.set_range(min, max);
    slider.set_size_request(max - min, -1);
    slider.set_sensitive(false);
    slider.set_hexpand(true);
    slider.set_digits(0);
    return slider;
  }

  /**
   * @param {() => void} handler
   */
  _newSwitch(handler) {
    const s = new Gtk.Switch();
    s.set_halign(Gtk.Align.END);
    s.set_valign(Gtk.Align.CENTER);
    if (handler) {
      s.connect('notify::active', handler);
    } else {
      s.set_sensitive(false);
    }
    return s;
  }

  /**
   * @param {any[][]} matrix
   */
  _newGrid(matrix) {
    const grid = new Gtk.Grid({ margin: 20 });
    grid.set_row_homogeneous(true);
    grid.set_column_spacing(10);
    for (let r = 0; r < matrix.length; r++) {
      for (let c = 0; c < matrix[r].length; c++) {
        grid.attach(matrix[r][c], c, r, 1, 1);
      }
    }
    return grid;
  }

  show() {
    this.window.show_all();
  }

  onDestroy() { }

  /**
   * @param {number} value
   */
  set LightLevel(value) {
    this._lightLevelValue.set_label(String(value));
  }

  /**
   * @param {number} value
   */
  set Normalized(value) {
    this._normalizedSlider.set_value(value);
  }

  /**
   * @param {number} value
   */
  set Pressure(value) {
    this._pressureSlider.set_value(value * 100);
  }

  /**
   * @param {number} value
   */
  set Filtered(value) {
    this._filteredSlider.set_value(value);
  }

  /**
   * @param {number} value
   */
  set Value(value) {
    this._valueSlider.set_value(value);
  }

  /**
   * @param {number} value
   */
  set Offset(value) {
    this._offsetSlider.set_value(value);
  }

  /**
   * @param {number} value
   */
  set Brightness(value) {
    this._brightnessSlider.set_value(value);
  }

  /**
   * @param {number} value
   */
  set Flags(value) {
    this._flagsValue.set_label(String(value));
  }

  /**
   * @param {string} msg
   */
  log(msg) {
    if (this._windowLogSwitch.get_active()) {
      this._logWindow.log(msg);
    }
  }

  _toggleWindowLog() {
    if (this._windowLogSwitch.get_active()) {
      this._logWindow.enable();
    } else {
      this._logWindow.disable();
    }
  }
}

class LogWindow {

  /**
   * @param {any} container
   */
  constructor(container) {
    this._container = container;
  }

  enable() {
    const textBuffer = new Gtk.TextBuffer();
    this._logBuffer = new LogBuffer(textBuffer);
    this._textView = this._newTextView(textBuffer);
    this._container.add(this._textView);
    this._textView.show_all();
  }

  disable() {
    this._textView.destroy();
    this._textView = null;
    this._logBuffer = null;
  }

  /**
   * @param {string} msg
   */
  log(msg) {
    this._logBuffer.append(msg);
  }

  /**
   * @param {any} textBuffer
   */
  _newTextView(textBuffer) {
    const textView = new Gtk.TextView({
      buffer: textBuffer,
      editable: false,
      monospace: true,
    });
    const textWindow = new Gtk.ScrolledWindow({
      child: textView,
      vexpand: true,
      min_content_height: 120,
    });
    this._autoScroll(textWindow.get_vadjustment(), textView);
    return textWindow;
  }

  /**
   * @param {any} adjustment
   * @param {any} scrollable
   */
  _autoScroll(adjustment, scrollable) {
    let enabled = true;
    adjustment.connect('notify::value', (adj) => {
      enabled = adj.get_value() >= adj.get_upper() - adj.get_page_size();
    });
    adjustment.connect('notify::upper', (adj) => {
      if (enabled) {
        const value = adj.get_upper() - adj.get_page_size();
        if (adj.get_value() !== value) {
          adj.set_value(value);
        } else if (value > 0 && scrollable) {
          scrollable.queue_draw();
        }
      }
    });
  }
}

class LogBuffer {

  /**
   * @param {any} textBuffer
   */
  constructor(textBuffer, maxLines = 101) {
    this.textBuffer = textBuffer;
    this.maxLines = maxLines;
  }

  /**
   * @param {string} msg
   */
  append(msg) {
    msg = this._format(msg);

    const end = this.textBuffer.get_end_iter();
    this.textBuffer.insert(end, msg, -1);

    const lines = this.textBuffer.get_line_count();
    if (lines > this.maxLines) {
      this._deleteStartLines(lines - this.maxLines);
    }
  }

  /**
   * @param {string} msg
   */
  _format(msg) {
    const date = this._formatDate(new Date());
    return `(${date}) - ${msg}\n`;
  }

  /**
   * @param {Date} date
   */
  _formatDate(date) {
    const year = date.getFullYear();
    const month = this._num2dgts(date.getMonth() + 1);
    const day = this._num2dgts(date.getDate());
    const hours = this._num2dgts(date.getHours());
    const minutes = this._num2dgts(date.getMinutes());
    const seconds = this._num2dgts(date.getSeconds());
    const millis = this._num2dgts(date.getMilliseconds(), 3);
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}.${millis}`;
  }

  /**
   * @param {number} num
   */
  _num2dgts(num, dgts = 2) {
    return num.toString().padStart(dgts, '0');
  }

  /**
   * @param {number} lines
   */
  _deleteStartLines(lines) {
    const start = this.textBuffer.get_start_iter();
    const end = this.textBuffer.get_iter_at_line(lines);
    this.textBuffer.delete(start, end);
  }
}

if (typeof module === 'object') {
  module.exports = {
    __esModule: true,
    DebugWindow
  };
}
