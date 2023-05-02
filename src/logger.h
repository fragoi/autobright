#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>

#define LOGGER(_logger)         _logger && _logger
#define LOGGER_DEBUG(_logger)   _logger.isDebug() && _logger.debug()
#define LOGGER_INFO(_logger)    _logger.isInfo() && _logger.info()
#define LOGGER_WARN(_logger)    _logger.isWarn() && _logger.warn()
#define LOGGER_ERROR(_logger)   _logger.isError() && _logger.error()

class Logger {
    using ostream = std::ostream;

  public:
    static ostream *out;
    static ostream *err;

    enum Level {
      DEBUG,
      DEFAULT,
      INFO,
      WARN,
      ERROR
    };

  private:
    const char *prefix;
    Level level;

    ostream& _prefix(ostream &out, const char *sep = " ") const {
      return prefix ? out << prefix << sep : out;
    }

  public:
    Logger(const char *prefix = nullptr, Level level = DEFAULT) :
        prefix(prefix), level(level) {
    }

    Logger& setLevel(Level level) {
      this->level = level;
      return *this;
    }

    template<typename T>
    ostream& operator<<(const T &value) const {
      return _prefix(*out) << value;
    }

    template<typename T>
    ostream& operator<<(const T *value) const {
      return _prefix(*out) << value;
    }

    ostream& debug() const {
      return _prefix(*out) << "DEBUG: ";
    }

    ostream& info() const {
      return _prefix(*out) << "INFO: ";
    }

    ostream& warn() const {
      return _prefix(*out) << "WARN: ";
    }

    ostream& error() const {
      return _prefix(*err) << "ERROR: ";
    }

    explicit operator bool() const {
      return level <= DEFAULT;
    }

    bool isDebug() const {
      return level <= DEBUG;
    }

    bool isInfo() const {
      return level <= INFO;
    }

    bool isWarn() const {
      return level <= WARN;
    }

    bool isError() const {
      return level <= ERROR;
    }
};

std::ostream& operator<<(std::ostream&, const std::exception_ptr&);

#endif /* LOGGER_H_ */
