#include <src/logger.h>

using namespace std;

int main() {
  Logger logger;
  logger.setLevel(Logger::DEBUG);

  logger << "default operation" << endl;
  logger.debug() << "some debug" << endl;
  logger.info() << "some info" << endl;
  logger.warn() << "some warn" << endl;
  logger.error() << "some error" << endl;

  logger = Logger("[Prefix]", Logger::DEBUG);
  logger << "default operation" << endl;
  logger.debug() << "some debug" << endl;
  logger.info() << "some info" << endl;
  logger.warn() << "some warn" << endl;
  logger.error() << "some error" << endl;

  logger = Logger("[OtherPrefix]");

  logger && logger << "default with check" << endl;
  logger.isDebug() && logger.debug() << "debug with check (disabled)" << endl;
  logger.isInfo() && logger.info() << "info with check" << endl;
  logger.isWarn() && logger.warn() << "warn with check" << endl;
  logger.isError() && logger.error() << "error with check" << endl;

  LOGGER(logger) << "default with macro" << endl;
  LOGGER_DEBUG(logger) << "debug with macro (disabled)" << endl;
  LOGGER_INFO(logger) << "info with macro" << endl;
  LOGGER_WARN(logger) << "warn with macro" << endl;
  LOGGER_ERROR(logger) << "error with macro" << endl;

  logger.setLevel(Logger::WARN);

  logger && logger << "default with check (disabled)" << endl;
  logger.isDebug() && logger.debug() << "debug with check (disabled)" << endl;
  logger.isInfo() && logger.info() << "info with check (disabled)" << endl;

  LOGGER(logger) << "default with macro (disabled)" << endl;
  LOGGER_DEBUG(logger) << "debug with macro (disabled)" << endl;
  LOGGER_INFO(logger) << "info with macro (disabled)" << endl;

  logger = Logger("[Different Types]");

  LOGGER(logger) << 1 << endl;
  LOGGER(logger) << true << endl;
  LOGGER(logger) << 'a' << endl;
  LOGGER(logger) << 0.1 << endl;
}
