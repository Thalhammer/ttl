#pragma once
#include <memory>
#include "noncopyable.h"
#include <mutex>
#include <set>

namespace ttl {
	template<typename MutexType, typename... Args>
	class signal_base {
		class signal_data : public std::enable_shared_from_this<signal_data>, public noncopyable {
		private:
			class delegate_base {
				std::shared_ptr<signal_data> _signal;
			public:
				explicit delegate_base(std::shared_ptr<signal_data> sig)
					:_signal(sig)
				{
					_signal->add(this);
				}
				virtual ~delegate_base() {
					_signal->remove(this);
					_signal.reset();
				}
				virtual void invoke(Args... args) = 0;
			};
			template<typename Func>
			class delegate : public delegate_base {
				Func func;
			public:
				delegate(Func f, std::shared_ptr<signal_data> evt)
					: delegate_base(evt), func(f)
				{}
				virtual void invoke(Args... args) override {
					func(std::forward<Args>(args)...);
				}
			};

			MutexType mutex;
			std::set<delegate_base*> delegates;

			void add(delegate_base* del) {
				std::lock_guard<MutexType> lck(mutex);
				delegates.insert(del);
			}
			void remove(delegate_base* del) {
				std::lock_guard<MutexType> lck(mutex);
				delegates.erase(del);
			}
		public:
			typedef std::shared_ptr<void> delegate_ptr;

			template<typename Func>
			delegate_ptr add(Func f) {
				return std::make_shared<delegate<Func>>(f, this->shared_from_this());
			}

			void invoke(Args... args) {
				std::lock_guard<MutexType> lck(mutex);
				for (delegate_base* e : delegates) {
					e->invoke(args...);
				}
			}
		};
		std::shared_ptr<signal_data> data;
	public:
		signal_base()
			: data(std::make_shared<signal_data>())
		{}

		template<typename Func>
		std::shared_ptr<void> add(Func f) {
			return data->add(f);
		}

		template<typename Func>
		std::shared_ptr<void> operator+=(Func f) {
			return data->add(f);
		}

		void invoke(Args... args) {
			data->invoke(std::forward<Args>(args)...);
		}

		void operator()(Args... args) {
			data->invoke(std::forward<Args>(args)...);
		}
	};

	template<typename... Args>
	using signal = signal_base<std::mutex, Args...>;
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
