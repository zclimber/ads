#include <vector>
#include <iostream>
#include "veb.h"

template<unsigned int S>
class VEBTree: AbstractVEBTree<S> {
private:
  std::vector<bool> f;
public:
  VEBTree() : f(1ULL << S) {

  }
  void add(unsigned long long x) {
    f[x] = true;
  }

  void remove(unsigned long long x) {
    f[x] = false;
  }

  unsigned long long next(unsigned long long x) const {
    ++x;
    while (x + 1 < (1ULL << S) && !f[x]) {
      ++x;
    }
    return (f[x] ? x : NO);
  }

  unsigned long long prev(unsigned long long x) const {
    if (x == 0) return -1;
    while (x > 0 && !f[x]) {
      --x;
    }
    return f[x] ? x : NO;
  }

  unsigned long long getMin() const {
    unsigned long long x = 0;
    while (x + 1 < (1ULL << S) && !f[x]) {
      ++x;
    }
    return f[x] ? x : NO;
  }

  unsigned long long getMax() const {
    unsigned long long x = (1ULL << S) - 1;
    while (x > 0 && !f[x]) {
      --x;
    }
    return f[x] ? x : NO;
  }
};

int main() {
  VEBTree<20> tree;
  tree.add(5);
  tree.add(11);
  tree.add(10);
  std::cout << tree.next(5) << std::endl;
  tree.remove(10);
  std::cout << tree.next(5) << std::endl;
}
