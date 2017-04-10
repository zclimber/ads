#include <vector>
#include <iostream>
#include "veb.h"
#include <cassert>
#include <type_traits>
#include <unordered_map>

#include <set>

template<unsigned int S>
class VEBTree/*: AbstractVEBTree<S>*/{
private:
	using int_type = typename std::conditional<S <= 31, unsigned int, unsigned long long>::type;
	constexpr const static int_type NOPE = (int_type) NO;
	int_type min = NOPE, max = NOPE;
	VEBTree<(S + 1) / 2> aux;
	std::unordered_map<int_type, VEBTree<S / 2>> children;
	bool empty() {
		return (min == NOPE);
	}
	int_type high(int_type x) {
		return (x >> (S / 2));
	}
	constexpr const static int_type low_mask = (1 << (S / 2)) - 1;
	int_type low(int_type x) {
		return x & low_mask;
	}
	void add(const int_type x) {
		if (empty()) {
			min = max = x;
		} else if (min == max) {
			if (x > max) {
				x = max;
			} else {
				x = min;
			}
		} else {
			if (min == x || max == x) {
				return;
			}
			if (min > x) {
				swap(min, x);
			} else if (max < x) {
				swap(max, x);
			}
			VEBTree<S / 2> & lower_tree = children[high(x)];
			if (lower_tree.empty()) {
				aux.inner_add(high(x));
			}
			lower_tree.insert(low(x));
		}
	}

	void remove(int_type x) {
		if (!empty()) {
			if (min == x) {
				if (max == x) {
					min = max = NOPE;
					return;
				} else {
					if (aux.min == NOPE) {
						min = max;
					} else {
						VEBTree<S / 2> & lower_tree = children[aux.min];
						int_type lower_min = lower_tree.min;
						min = (aux.min << (S / 2)) + lower_min;
						lower_tree.remove(lower_min);
						if (lower_tree.empty()) {
							children.erase(aux.min);
							aux.remove(aux.min);
						}
					}
				}
			} else {
				if (max == x) {
					VEBTree<S / 2> & lower_tree = children[aux.max];
					int_type lower_max = lower_tree.max;
					max = (aux.max << (S / 2)) + lower_max;
					lower_tree.remove(lower_max);
					if (lower_tree.empty()) {
						children.erase(aux.max);
						aux.remove(aux.max);
					}
				} else {
					auto lower_tree_it = children.find(high(x));
					if (lower_tree_it != children.end()) {
						lower_tree_it->second.remove(low(x));
						if (lower_tree_it->second.empty()) {
							children.erase(lower_tree_it);
							aux.remove(high(x));
						}
					}
				}
			}
		}
	}
	unsigned long long next(int_type x) const {
		if (x >= max) {
			return NO;
		} else if (x < min) {
			return min;
		} else if (aux.empty()) {
			return max;
		} else {
			auto lower_tree_it = children.find(high(x));
			if (lower_tree_it != children.end()
					&& lower_tree_it->second.max > low(x)) {
				return (high(x) << (S / 2)) + lower_tree_it->second.next(low(x));
			} else {
				int_type next_high = aux.next(high(x));
				return (next_high << (S / 2)) + children[next_high].min;
			}
		}
	}
	unsigned long long prev(int_type x) const {
		if (x <= min) {
			return NO;
		} else if (x > max) {
			return max;
		} else if (aux.empty()) {
			return min;
		} else {
			auto lower_tree_it = children.find(high(x));
			if (lower_tree_it != children.end()
					&& lower_tree_it->second.min < low(x)) {
				return (high(x) << (S / 2)) + lower_tree_it->second.prev(low(x));
			} else {
				int_type prev_high = aux.prev(high(x));
				return (prev_high << (S / 2)) + children[prev_high].max;
			}
		}
	}
	void to_set(std::set<unsigned long long> res,
			unsigned long long high_part) const {
		res.insert(min);
		res.insert(max);
		for (auto child : children) {
			child.second.to_set(res, high_part + (child.first << (S / 2)));
		}
	}
public:
	VEBTree() {
		static_assert(S <= 63, "VEBtree cannot store more than 63 bit integers");
	}
	/*
	 void outer_add(unsigned long long x) {
	 assert(x < (1 << S));
	 add(x);
	 }

	 void outer_remove(unsigned long long x) {
	 if (x < (1 << S)) {
	 remove(x);
	 }
	 }

	 unsigned long long outer_next(unsigned long long x) const {
	 if (x >= (1ULL << S)) {
	 return NO;
	 } else {
	 return next(x);
	 }
	 }

	 unsigned long long outer_prev(unsigned long long x) const {
	 if (x >= (1ULL << S)) {
	 return max == NOPE ? NO : max;
	 } else {
	 return prev(x);
	 }
	 }

	 unsigned long long outer_getMin() const {
	 return min == NOPE ? NO : min;
	 }

	 unsigned long long outer_getMax() const {
	 return max == NOPE ? NO : max;
	 }
	 */
};

int main() {
	VEBTree<62> vt();
}
