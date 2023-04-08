#include <cassert>
#include <iostream>

#include <src/splist.h>

using namespace std;
using namespace splist;

int main() {
  List<int> list;

  int v1 = 1;
  list.push_back(v1);
  list.push_back(2);
  list.push_back(3);
  v1 = 2; // ensure value is copied

  auto itr = list.begin();
  assert(*itr == 1);
  assert(*++itr == 2);
  assert(*++itr == 3);

  int c = 0;
  for (const int &i : list) {
    c++;
    assert(i == c);
  }

  list.remove(2);

  itr = list.begin();
  assert(*(itr++) == 1);
  assert(*(itr++) == 3);
  assert(itr == list.end());

  list.remove(3);
  itr = list.begin();
  assert(*(itr++) == 1);
  assert(itr == list.end());

  list.remove(1);
  assert(list.begin() == list.end());

  cout << "OK" << endl;
}
