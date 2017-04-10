#include <vector>
#include <iostream>
#include "veb.h"
#include <cassert>
#include <type_traits>
#include <unordered_map>

#include <set>
#include <random>
#include <ctime>
#include <string>

namespace {
template<unsigned int S>
class int_type_selector {
public:
//	using type = unsigned long long;
	using type = typename std::conditional<S <= 31, unsigned int, unsigned long long>::type;
};
}

template<unsigned int S>
class VEBTree/*: AbstractVEBTree<S>*/{
private:
	template<unsigned int T>
	friend class VEBTree;
	using int_type = typename int_type_selector<S>::type;
	using int_type_low = typename int_type_selector<S / 2>::type;
	using int_type_high = typename int_type_selector<(S + 1) / 2>::type;
	constexpr const static int_type NOPE = (int_type) NO;
	int_type __min = NOPE, __max = NOPE;
	VEBTree<(S + 1) / 2> aux;
	std::unordered_map<int_type, VEBTree<S / 2>> children;
	int_type min() const {
		return __min;
	}
	int_type & min() {
		return __min;
	}
	int_type max() const {
		return __max;
	}
	int_type & max() {
		return __max;
	}
	bool empty() const {
		return (min() == NOPE);
	}
	static int_type_high high(const int_type x) {
		return int_type_high(x >> (S / 2));
	}
	constexpr const static int_type_low low_mask = (1ULL << (S / 2)) - 1;
	static int_type_low low(const int_type x) {
		return x & low_mask;
	}
	void inner_add(const int_type x) {
		if (min() == x || max() == x) {
			return;
		}
		if (empty()) {
			min() = max() = x;
		} else if (min() == max()) {
			if (x > max()) {
				max() = x;
			} else {
				min() = x;
			}
		} else if (min() > x) {
			int_type old_min = min();
			min() = x;
			inner_add(old_min);
		} else if (max() < x) {
			int_type old_max = max();
			max() = x;
			inner_add(old_max);
		} else {
			VEBTree<S / 2> & lower_tree = children[high(x)];
			if (lower_tree.empty()) {
				aux.inner_add(high(x));
			}
			lower_tree.inner_add(low(x));
		}
		/**
		 * integrity check
		 */
//		std::set<unsigned long long> ch;
//		aux.to_set(ch, 0);
//		ch.erase(NO);
//		for (auto elem_add : ch) {
//			assert(children.count(elem_add));
//		}
	}

	void inner_remove(int_type x) {
		if (empty() || x < min() || x > max()) {
			return;
		}
		if (min() == x && max() == x) {
			min() = max() = NOPE;
		} else if (min() == x) {
			if (aux.empty()) {
				min() = max();
			} else {
				assert(children.count(aux.min()));
				VEBTree<S / 2> * lower_tree =
						&(children.find(aux.min())->second);
				int_type_low lower_min = lower_tree->min();
				min() = (aux.min() << (S / 2)) + lower_min;
				lower_tree->inner_remove(lower_min);
				if (lower_tree->empty()) {
					children.erase(aux.min());
					aux.inner_remove(aux.min());
				}
			}
		} else if (max() == x) {
			if (aux.empty()) {
				max() = min();
			} else {
				assert(children.count(aux.max()));
				VEBTree<S / 2> * lower_tree =
						&(children.find(aux.max())->second);
				int_type_low lower_max = lower_tree->max();
				max() = (aux.max() << (S / 2)) + lower_max;
				lower_tree->inner_remove(lower_max);
				if (lower_tree->empty()) {
					children.erase(aux.max());
					aux.inner_remove(aux.max());
				}
			}
		} else {
			auto lower_tree_it = children.find(high(x));
			if (lower_tree_it != children.end()) {
				lower_tree_it->second.inner_remove(low(x));
				if (lower_tree_it->second.empty()) {
					children.erase(lower_tree_it);
					aux.inner_remove(high(x));
				}
			}
		}
		/**
		 * integrity check
		 */
//		std::set<unsigned long long> ch;
//		aux.to_set(ch, 0);
//		ch.erase(NO);
//		for (auto elem : ch) {
//			assert(children.count(elem));
//		}
	}
	int_type inner_next(int_type x) const {
		if (x >= max()) {
			return NOPE;
		} else if (x < min()) {
			return min();
		} else if (aux.empty()) {
			return max();
		} else {
			auto lower_tree_it = children.find(high(x));
			if (lower_tree_it != children.end()
					&& lower_tree_it->second.max() > low(x)) {
				return (int_type(high(x)) << (S / 2))
						+ lower_tree_it->second.inner_next(low(x));
			} else {
				int_type next_high = aux.inner_next(high(x));
				if (next_high == aux.NOPE) {
					return max();
				}
				if (children.count(next_high) == 0) {
					std::cerr << "in vebtree" << S << " accessed index "
							<< next_high << " from " << x << "\n";
				}
				return (next_high << (S / 2)) + children.at(next_high).min();
			}
		}
	}
	int_type inner_prev(int_type x) const {
		if (x <= min()) {
			return NOPE;
		} else if (x > max()) {
			return max();
		} else if (aux.empty()) {
			return min();
		} else {
			auto lower_tree_it = children.find(high(x));
			if (lower_tree_it != children.end()
					&& lower_tree_it->second.min() < low(x)) {
				return (int_type(high(x)) << (S / 2))
						+ lower_tree_it->second.inner_prev(low(x));
			} else {
				int_type prev_high = aux.inner_prev(high(x));
				if (prev_high == aux.NOPE) {
					return min();
				}
				if (children.count(prev_high) == 0) {
					std::cerr << "in vebtree" << S << " accessed index "
							<< prev_high << " from " << x << "\n";
				}
				return (prev_high << (S / 2)) + children.at(prev_high).max();
			}
		}
	}
	void to_set(std::set<unsigned long long> &res,
			unsigned long long high_part) const {
		res.insert(high_part + min());
		res.insert(high_part + max());
		for (auto child : children) {
			child.second.to_set(res, high_part + (child.first << (S / 2)));
		}
	}
public:
	std::set<unsigned long long> to_set() {
		std::set<unsigned long long> result;
		to_set(result, 0);
		return result;
	}
	VEBTree() {
		static_assert(S <= 63, "VEBtree cannot store more than 63 bit integers");
	}
	void add(unsigned long long x) {
		assert(x < (1ULL << S));
		inner_add(x);
	}

	void remove(unsigned long long x) {
		if (x < (1ULL << S)) {
			inner_remove(x);
		}
	}

	unsigned long long next(unsigned long long x) const {
		if (x >= (1ULL << S)) {
			return NO;
		} else {
			int_type inext = inner_next(x);
			return inext == NOPE ? NO : inext;
		}
	}

	unsigned long long prev(unsigned long long x) const {
		if (x >= (1ULL << S)) {
			return max() == NOPE ? NO : max();
		} else {
			int_type iprev = inner_prev(x);
			return iprev == NOPE ? NO : iprev;
		}
	}

	unsigned long long getMin() const {
		return min() == NOPE ? NO : min();
	}

	unsigned long long getMax() const {
		return max() == NOPE ? NO : max();
	}
};

template<>
class VEBTree<1> {
	template<unsigned int T>
	friend class VEBTree;
	using int_type = typename int_type_selector<0>::type;
	constexpr const static int_type NOPE = (int_type) NO;
	char bits = 0;
	int_type min() const {
		switch (bits) {
		case 0:
			return NOPE;
		case 1:
		case 3:
			return 0;
		case 2:
			return 1;
		default:
			return NOPE;
		}
	}
	int_type max() const {
		switch (bits) {
		case 0:
			return NOPE;
		case 2:
		case 3:
			return 1;
		case 1:
			return 0;
		default:
			return NOPE;
		}
	}
	bool empty() const {
		return (bits == 0);
	}
	void inner_add(const int_type x) {
		bits |= char(1 << x);
	}
	void inner_remove(int_type x) {
		bits &= char(~(1 << x));
	}
	int_type inner_next(int_type x) const {
		if (x == 0 && (bits & 2)) {
			return 1;
		} else {
			return NOPE;
		}
	}
	int_type inner_prev(int_type x) const {
		if (x == 1 && (bits & 1)) {
			return 0;
		} else {
			return NOPE;
		}
	}
	void to_set(std::set<unsigned long long> &res,
			unsigned long long high_part) const {
		if (bits & 1) {
			res.insert(high_part);
		}
		if (bits & 2) {
			res.insert(high_part + 1);
		}
	}
};

int main() {
	std::mt19937_64 rand(std::time(0));
	const int test_size = 1000;
	const int test_count = 10000;
	for (int test = 1; test <= test_count; test++) {
		VEBTree<63> vt;
		std::set<unsigned long long> orig;
		std::set<unsigned long long> result;
		std::string events;
		for (int i = 0; i < test_size; i++) {
			unsigned int r = (unsigned int) rand();
			auto it = orig.upper_bound(r);
			if (r % 3 != 0 || it == orig.end()) {
				events.append("add ");
				events.append(std::to_string(r));
				vt.add(r);
				orig.insert(r);
			} else {
				events.append("rm ");
				events.append(std::to_string(*it));
				vt.remove(*it);
				orig.erase(*it);
			}
			events.append("\n");
		}
		bool test_rs = true;
		std::set<unsigned long long, std::less<unsigned long long> > orig_2(
				orig.begin(), orig.end());
		for (auto x : orig_2) {
			auto it = orig_2.upper_bound(x);
			auto rs = (it == orig_2.end() ? NO : *it);
			auto vtrs = vt.next(x);
			if (rs != vtrs) {
				std::cerr << "NEXT gives " << rs << " for original and " << vtrs
						<< " on real for target " << x << "\n";
				test_rs = false;
				break;
			}
		}
		std::set<unsigned long long, std::greater<unsigned long long> > orig_3(
				orig.begin(), orig.end());
		for (auto x : orig_3) {
			auto it = orig_3.upper_bound(x);
			auto rs = (it == orig_3.end() ? NO : *it);
			auto vtrs = vt.prev(x);
			if (rs != vtrs) {
				std::cerr << "PREV gives " << rs << " for original and " << vtrs
						<< " on real for target " << x << "\n";
				test_rs = false;
				break;
			}
		}
		result = vt.to_set();
		result.erase(NO);
		if (orig != result || !test_rs) {
			std::cerr << "FAIL ON TEST " << test << "\n";
			std::cerr << events << "\n";
			std::cerr << "Original :";
			for (auto x : orig) {
				std::cerr << x << " ";
			}
			std::cerr << "\n";
			std::cerr << "Real :";
			for (auto x : result) {
				std::cerr << x << " ";
			}
			std::cerr << "\n";
			return 0;
		}
		if (test % (test_count / 10) == 0) {
			std::cerr << "PASSED " << test << " OUT OF " << test_count
					<< " TESTS\n";
		}
	}
}
