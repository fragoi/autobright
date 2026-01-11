#ifndef SPLIST_H_
#define SPLIST_H_

#include <memory>
#include <utility>

namespace _splist {

  using namespace std;

  template<typename T>
  class Node {
      using N = Node<T>;
      using P = shared_ptr<N>;

    public:
      T value;
      P next;

      template<typename V>
      Node(V &&value) : value(forward<V>(value)) {
      }
  };

  template<typename T>
  class Iterator {
      using N = Node<T>;
      using P = shared_ptr<N>;

      P node;

    public:
      Iterator() {
      }

      Iterator(const P &node) : node(node) {
      }

      T& operator*() {
        return node->value;
      }

      const T& operator*() const {
        return node->value;
      }

      Iterator& operator++() {
        node = node->next;
        return *this;
      }

      Iterator operator++(int) {
        Iterator tmp = *this;
        operator ++();
        return tmp;
      }

      friend bool operator==(const Iterator &a, const Iterator &b) {
        return a.node == b.node;
      }

      friend bool operator!=(const Iterator &a, const Iterator &b) {
        return a.node != b.node;
      }
  };

  template<typename T>
  class List {
      using N = Node<T>;
      using P = shared_ptr<N>;
      using Itr = Iterator<T>;

    private:
      P head;
      N *tail = nullptr;

      template<typename V>
      P node(V &&value) {
        return P(new N(forward<V>(value)));
      }

      void push_back(const P &node) {
        if (tail) {
          tail->next = node;
        } else {
          head = node;
        }
        tail = node.get();
      }

      void remove_next(const P &node) {
        if (node) {
          const N *next = node->next.get();
          if (next) {
            node->next = next->next;
            if (tail == next) {
              tail = node.get();
            }
          }
        } else {
          const N *next = head.get();
          if (next) {
            head = next->next;
            if (tail == next) {
              tail = nullptr;
            }
          }
        }
      }

    public:
      ~List() {
        while (head) {
          head = head->next;
        }
        tail = nullptr;
      }

      void push_back(const T &value) {
        push_back(node(value));
      }

      void push_back(T &&value) {
        push_back(node(move(value)));
      }

      void remove(const T &value) {
        P prev;
        P node = head;
        while (node) {
          if (node->value == value) {
            remove_next(prev);
          } else {
            prev = node;
          }
          node = node->next;
        }
      }

      Itr begin() {
        return Itr(head);
      }

      const Itr begin() const {
        return Itr(head);
      }

      Itr end() {
        return Itr();
      }

      const Itr end() const {
        return Itr();
      }
  };

}

namespace splist {

  using _splist::List;

}

#endif /* SPLIST_H_ */
