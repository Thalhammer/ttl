#pragma once
#include <type_traits>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <forward_list>
#include <stack>

namespace thalhammer {
	namespace linq_detail {
		template<typename T, typename Func>
		class whereiterator;

		template<typename T, typename Func, typename Out = typename std::result_of<Func(T)>::type>
		class selectiterator;

		template<typename T, typename Func, typename Key = typename std::result_of<Func(T)>::type>
		class groupiterator;

		template<typename T, typename Getter, typename comparer>
		class orderiterator;

		template<typename T>
		class iterator {
		public:
			virtual const T& element() = 0;
			virtual bool next() = 0;
			virtual bool is_end() = 0;

			template<typename Func>
			auto where(Func func) {
				return whereiterator<T, Func>(func, *this);
			}

			template<typename Func>
			auto select(Func func) {
				return selectiterator<T, Func>(func, *this);
			}

			template<typename Func>
			auto groupby(Func func) {
				return groupiterator<T, Func>(func, *this);
			}

			template<typename Func>
			auto orderby(Func func) {
				struct comparer {
					Func fn;
					bool operator()(const T& lhs, const T& rhs) {
						return fn(lhs) < fn(rhs);
					}
				};
				return orderiterator<T, Func, comparer>(func, *this);
			}

			template<typename U>
			auto except(const U& other) {
				return where([&other](const T& elem) {
					return std::cend(other) == std::find(std::cbegin(other), std::cend(other), elem);
				});
			}

			template<typename Func>
			auto orderby_descending(Func func) {
				struct comparer {
					Func fn;
					bool operator()(const T& lhs, const T& rhs) {
						return fn(lhs) > fn(rhs);
					}
				};
				return orderiterator<T, Func, comparer>(func, *this);
			}

			std::vector<T> to_vector() {
				if (is_end())
					return{};
				std::vector<T> res;
				do {
					res.push_back(this->element());
				} while (next());
				return res;
			}

			std::set<T> to_set() {
				if (is_end())
					return{};
				std::set<T> res;
				do {
					res.insert(this->element());
				} while (next());
				return res;
			}

			std::list<T> to_list() {
				if (is_end())
					return{};
				std::list<T> res;
				do {
					res.push_back(this->element());
				} while (next());
				return res;
			}

			std::forward_list<T> to_forward_list() {
				if (is_end())
					return{};
				std::forward_list<T> res;
				res.push_front(this->element());
				auto it = res.cbegin();
				while(next()) {
					it = res.insert_after(it, this->element());
				}
				return res;
			}

			std::stack<T> to_stack() {
				if (is_end())
					return{};
				std::stack<T> res;
				do {
					res.push(this->element());
				} while (next());
				return res;
			}

			auto single() {
				if (this->is_end())
					throw std::logic_error("Resultset is empty");
				auto res = this->element();
				if (this->next())
					throw std::logic_error("Resultset contains multiple entries");
				return res;
			}

			auto single_or_default(T def) {
				if (this->is_end())
					return def;
				auto res = this->element();
				if (this->next())
					throw std::logic_error("Resultset contains multiple entries");
				return res;
			}

			auto first() {
				if (this->is_end())
					throw std::logic_error("Resultset is empty");
				return this->element();
			}

			auto first_or_default(T def) {
				if (this->is_end())
					return def;
				return this->element();
			}

			auto last() {
				if (this->is_end())
					throw std::logic_error("Resultset is empty");
				T res = this->element();
				while (next()) res = this->element();
				return res;
			}

			auto last_or_default(T def) {
				if (this->is_end())
					return def;
				T res = this->element();
				while (next()) res = this->element();
				return res;
			}

			auto element_at(size_t i) {
				if (this->is_end())
					throw std::logic_error("Resultset is empty");
				for (; i != 0; i--)
					if (!next()) throw std::logic_error("Resultset too small");
				return this->element();
			}

			auto element_at_or_default(size_t i, T def) {
				if (this->is_end())
					return def;
				for (; i != 0; i--)
					if (!next()) return def;
				return this->element();
			}

			auto count() {
				size_t cnt = 0;
				if (is_end()) return cnt;
				do { cnt++; } while (next());
				return cnt;
			}

			template<class U = T, class = typename std::enable_if<std::is_integral<U>::value>::type>
			auto sum() {
				T res = 0;
				do { res += this->element(); } while (next());
				return res;
			}

			template<class U = T, class = typename std::enable_if<std::is_integral<U>::value>::type>
			auto avg() {
				if (is_end())
					return double(0);
				double sum = 0;
				size_t cnt = 0;
				do { sum += this->element(); cnt++; } while (next());
				return sum / cnt;
			}

			template<class U = T, class = typename std::enable_if<std::is_integral<U>::value>::type>
			auto max() {
				if (is_end())
					return T(0);
				T max = this->element();
				while (next()) {
					auto& el = this->element();
					if (max < el)
						max = el;
				}
				return max;
			}

			template<class U = T, class = typename std::enable_if<std::is_integral<U>::value>::type>
			auto min() {
				if (is_end())
					return T(0);
				T min = this->element();
				while (next()) {
					auto& el = this->element();
					if (el < min)
						min = el;
				}
				return min;
			}

			template<typename Func>
			bool all(Func f) {
				if (is_end())
					return true;
				do {
					if (!f(element()))
						return false;
				} while (next());
				return true;
			}

			template<typename Func>
			bool any(Func f) {
				if (is_end())
					return false;
				do {
					if (f(element()))
						return true;
				} while (next());
				return false;
			}

			bool contains(const T& rhs) {
				return any([&rhs](const T& lhs) {
					return lhs == rhs;
				});
			}
		};

		template<typename T, typename Iterator>
		class sourceiterator : public iterator<T> {
			Iterator start;
			Iterator end;
			Iterator cur;
		public:
			sourceiterator(Iterator pstart, Iterator pend) : start(pstart), end(pend), cur(pstart) {}
			void set_iterators(Iterator pstart, Iterator pend) {
				start = pstart;
				end = pend;
				cur = pstart;
			}
			const T& element() { return *cur; }
			bool next() {
				if (is_end())
					return false;
				cur++;
				return cur != end;
			}
			bool is_end() { return cur == end; }
		};

		template<typename T, typename Func, typename Out>
		class selectiterator : public iterator<Out>
		{
			Func transform;
			iterator<T>& source;
			std::unique_ptr<Out> elem;
		public:
			selectiterator(Func f, iterator<T>& psource) : transform(f), source(psource) {
				if (!is_end())
					elem = std::make_unique<Out>(transform(source.element()));
			}

			virtual const Out& element() {
				return *elem;
			}

			virtual bool next() {
				if (source.is_end())
					return false;
				if (!source.next()) return false;
				elem = std::make_unique<Out>(transform(source.element()));
				return true;
			}

			virtual bool is_end() {
				return source.is_end();
			}
		};

		template<typename T, typename Func>
		class whereiterator : public iterator<T> {
			Func check;
			iterator<T>& source;
		public:
			whereiterator(Func f, iterator<T>& psource) : check(f), source(psource) {
				if (!this->is_end() && !check(this->element())) {
					this->next();
				}
			}

			virtual const T& element() {
				return source.element();
			}

			virtual bool next() {
				if (source.is_end())
					return false;
				while (source.next() && !check(source.element()));
				return !source.is_end();
			}

			virtual bool is_end() {
				return source.is_end();
			}
		};

		template<typename T, typename Func, typename Key>
		class groupiterator : public iterator<std::pair<Key, std::vector<T>>>
		{
			std::map<Key, std::vector<T>> data;
			typename std::map<Key, std::vector<T>>::const_iterator it;
			std::pair<Key, std::vector<T>> elem;
		public:
			groupiterator(Func f, iterator<T>& source) {
				if (!source.is_end()) {
					do {
						auto key = f(source.element());
						data[key].push_back(source.element());
					} while (source.next());
					it = data.cbegin();
					elem = *it;
				}
				else {
					it = data.cend();
				}
			}

			virtual const std::pair<Key, std::vector<T>>& element() {
				return elem;
			}

			virtual bool next() {
				it++;
				if (it != data.cend()) {
					elem = *it;
				}
				else return false;
				return it != data.cend();
			}

			virtual bool is_end() {
				return it == data.cend();
			}
		};

		template<typename T, typename Getter, typename comparer>
		class orderiterator : public iterator<T>
		{
			std::multiset<T, comparer> data;
			typename std::multiset<T, comparer>::const_iterator it;
		public:
			orderiterator(Getter f, iterator<T>& source) : data(comparer{ f }) {
				if (!source.is_end()) {
					do {
						data.insert(source.element());
					} while (source.next());
					it = data.cbegin();
				}
				else {
					it = data.cend();
				}
			}

			virtual const T& element() {
				return *it;
			}

			virtual bool next() {
				it++;
				return it != data.cend();
			}

			virtual bool is_end() {
				return it == data.cend();
			}
		};
	}

	template<typename T, typename Iterator>
	inline auto linq(Iterator start, Iterator end) {
		return linq_detail::sourceiterator<T, Iterator>(start, end);
	}

	template<typename T>
	inline auto linq(const std::vector<T>& data) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}

	template<typename T>
	inline auto linq(const std::set<T>& data) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}

	template<typename T>
	inline auto linq(const std::list<T>& data) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}

	template<typename T>
	inline auto linq(const std::forward_list<T>& data) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}

	template<typename T, size_t Num>
	inline auto linq(const std::array<T, Num>& data) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}

	template<typename T, size_t Num>
	inline auto linq(const T (&data)[Num]) {
		return linq<T>(std::cbegin(data), std::cend(data));
	}
}