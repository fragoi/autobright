#ifndef GOBJECTMM_H_
#define GOBJECTMM_H_

#include <memory>
#include <stdexcept>
#include <glib-object.h>

template<typename T>
class gobject_ptr: public std::shared_ptr<T> {
    using B = std::shared_ptr<T>;

    static void deleter(T *ptr) noexcept {
      if (ptr != NULL && G_IS_OBJECT(ptr))
        g_object_unref(ptr);
    }

  public:
    explicit gobject_ptr(T *ptr = NULL) noexcept : B(ptr, deleter) {
      g_warn_if_fail(ptr == NULL || G_IS_OBJECT(ptr));
    }
};

template<typename T, typename D = void (*)(T*)>
class gboxed_ptr: public std::unique_ptr<T, D> {
    using B = std::unique_ptr<T, D>;

    static const D& ensureDeleter(const D &deleter) {
      if (!deleter)
        throw std::invalid_argument("Free function is required");
      return deleter;
    }

  public:
    gboxed_ptr(T *ptr, const D &deleter) :
        B(ptr, ensureDeleter(deleter)) {
    }
};

template<typename T, void (*D)(T*)>
class gshared_ptr: public std::shared_ptr<T> {
    using B = std::shared_ptr<T>;

    static void deleter(T *ptr) {
      if (ptr)
        D(ptr);
    }

  public:
    explicit gshared_ptr(T *ptr = NULL) : B(ptr, deleter) {
    }
};

template<typename T, void (*D)(T*)>
class gunique_ptr: public std::unique_ptr<T, typeof(D)> {
    using B = std::unique_ptr<T, typeof(D)>;

    static void deleter(T *ptr) {
      if (ptr)
        D(ptr);
    }

  public:
    explicit gunique_ptr(T *ptr = NULL) : B(ptr, deleter) {
    }
};

#endif /* GOBJECTMM_H_ */
