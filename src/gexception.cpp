#include <utility>

#include "gexception.h"

inline static GError* copy_error(GError *error) noexcept {
  return error ? g_error_copy(error) : NULL;
}

GException::GException(GError *error) noexcept : ptr(error) {
}

GException::~GException() noexcept {
  if (ptr)
    g_error_free(ptr);
}

GException::GException(const GException &other) noexcept :
    ptr(copy_error(other.ptr)) {
}

GException::GException(GException &&other) noexcept : ptr() {
  swap(*this, other);
}

GException& GException::operator=(GException other) noexcept {
  swap(*this, other);
  return *this;
}

GError& GException::operator *() noexcept {
  return *ptr;
}

const GError& GException::operator *() const noexcept {
  return *ptr;
}

GError* GException::operator ->() noexcept {
  return ptr;
}

const GError* GException::operator ->() const noexcept {
  return ptr;
}

GException::operator bool() const noexcept {
  return ptr;
}

GError** GException::get() noexcept {
  return &ptr;
}

const char* GException::what() const noexcept {
  return ptr ? ptr->message : "No reason";
}

bool operator==(const GException &a, const GException &b) noexcept {
  return a.ptr == b.ptr;
}

bool operator!=(const GException &a, const GException &b) noexcept {
  return a.ptr != b.ptr;
}

void swap(GException &a, GException &b) noexcept {
  using std::swap;
  swap(a.ptr, b.ptr); // @suppress("Invalid arguments")
}
