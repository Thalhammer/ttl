#pragma once
#include <mutex>
#include <memory>

namespace thalhammer {
	// A type to allow for easy rcu (read,copy,update)
	template<typename T>
	class rcu {
		// We only need the mutex to prevent concurrent modifications.
		// We could do atomic exchange but this is intended for low write high read structures
		// so we just use a mutex because it makes modification a lot easier.
		std::mutex write_mtx;
		std::shared_ptr<T> data;
	public:
		rcu()
			: data(std::make_shared<T>())
		{}

		template<typename... Args>
		rcu(Args&&... args)
			: data(std::make_shared<T>(std::forward<Args>(args)...))
		{}

		// Threadsafe update of data
		template<typename Func>
		void update(Func f)
		{
			std::lock_guard<std::mutex> lck(write_mtx);
			// Get local pointer
			std::shared_ptr<const T> pdata = std::atomic_load(&data);
			// Copy it
			auto mdata = std::make_shared<T>(*pdata);
			f(*mdata);
			// Copy to class instance
			std::atomic_store(&data, mdata);
		}

		// Get the current data
		// The returned data will not change during its lifetime.
		// To get updates you need to call get() again
		std::shared_ptr<const T> get() const {
			return std::atomic_load(&data);
		}
	};
}
