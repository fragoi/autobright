#ifndef GEXCEPTION_H_
#define GEXCEPTION_H_

#include <exception>
#include <glib.h>

class GException: public std::exception {
  private:
    GError *ptr;

  public:
    explicit GException(GError *error = NULL) noexcept;
    ~GException() noexcept;
    GException(const GException&) noexcept;
    GException(GException&&) noexcept;
    GException& operator=(GException) noexcept;

    GError& operator*() noexcept;
    const GError& operator*() const noexcept;

    GError* operator->() noexcept;
    const GError* operator->() const noexcept;

    explicit operator bool() const noexcept;

    GError** get() noexcept;

    const char* what() const noexcept;

    friend bool operator==(const GException &a, const GException &b) noexcept;

    friend bool operator!=(const GException &a, const GException &b) noexcept;

    friend void swap(GException &a, GException &b) noexcept;
};

#endif /* GEXCEPTION_H_ */
