#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <set>
#include <atomic>
#include <functional>

namespace ttl {
	class timer {
	public:
		typedef std::shared_ptr<void> token_t;
		typedef std::function<void()> handler_fn_t;

		typedef std::function<void(std::exception_ptr)> exception_fn_t;

		class every {
			friend class timer;
			std::chrono::nanoseconds dur;
		public:
			template<typename T>
			explicit every(T t)
				: dur(std::chrono::duration_cast<std::chrono::nanoseconds>(t))
			{}
		};

	private:
		struct task_t {
			std::chrono::steady_clock::time_point tp;
			handler_fn_t fn;
		};
		std::set<std::shared_ptr<task_t>> m_tasks;
		mutable std::mutex mtx;
		std::condition_variable cv;
		std::thread thread;
		std::atomic<bool> exit_thread;
		exception_fn_t exception_handler;

		// Get all tasks that need to get executed
		std::set<std::shared_ptr<task_t>> get_ready_tasks(bool remove = false) {
			auto now = std::chrono::steady_clock::now();
			std::set<std::shared_ptr<task_t>> res;
			std::unique_lock<std::mutex> lck(mtx);
			for (auto it = m_tasks.begin(); it != m_tasks.end();) {
				if ((*it)->tp <= now) {
					res.insert(*it);
					if (remove)
						it = m_tasks.erase(it);
					else it++;
				}
				else {
					it++;
				}
			}
			return res;
		}

		// Get the next task that needs to get executed
		std::shared_ptr<task_t> get_next_task() {
			std::shared_ptr<task_t> next;
			for (auto& e : m_tasks) {
				if (!next || e->tp < next->tp)
					next = e;
			}
			return next;
		}

		void schedule(std::shared_ptr<task_t> task)
		{
			std::unique_lock<std::mutex> lck(mtx);
			m_tasks.insert(task);
			auto next = get_next_task();
			if (next == task) {
				cv.notify_all();
			}
		}

		void thread_fn() {
			while (!exit_thread) {
				{
					auto tasks = get_ready_tasks(true);
					for (auto& task : tasks) {
						try {
							task->fn();
						}
						catch (...) {
							auto handler = get_exception_handler();
							if(handler)
								handler(std::current_exception());
						}
					}
				}
				{
					std::unique_lock<std::mutex> lck(mtx);
					if (!exit_thread) {
						auto next = get_next_task();
						if (next) {
							auto tp = next->tp;
							next = nullptr;
							cv.wait_until(lck, tp);
						}
						else cv.wait(lck);
					}
				}
			}
		}
	public:
		timer()
			: exit_thread(false)
		{
			thread = std::thread(std::bind(&timer::thread_fn, this));
		}

		~timer() {
			this->clear_all();
			{
				std::unique_lock<std::mutex> lck(mtx);
				exit_thread = true;
				cv.notify_all();
			}
			if (thread.joinable())
				thread.join();
		}

		token_t schedule(handler_fn_t fn, std::chrono::steady_clock::time_point tp)
		{
			auto task = std::make_shared<task_t>();
			task->fn = fn;
			task->tp = tp;
			this->schedule(task);
			return task;
		}

		template<typename Rep, typename Period>
		token_t schedule(handler_fn_t fn, std::chrono::duration<Rep, Period> t)
		{
			return this->schedule(fn, std::chrono::steady_clock::now() + t);
		}

		template<typename Clock, typename Duration>
		token_t schedule(handler_fn_t fn, std::chrono::time_point<Clock, Duration> tp)
		{
			auto dur = tp - Clock::now();
			return this->schedule(fn, dur);
		}

		token_t schedule(handler_fn_t fn, every t)
		{
			auto newtask = std::make_shared<task_t>();
			auto weaktask = std::weak_ptr<task_t>(newtask);
			auto dur = t.dur;
			newtask->fn = [fn, dur, weaktask, this](){
				auto task = weaktask.lock();
				if(task) {
					task->tp = std::chrono::steady_clock::now() + dur;
					this->schedule(task);
				}
				fn();
			};
			newtask->tp = std::chrono::steady_clock::now() + t.dur;
			this->schedule(newtask);
			return newtask;
		}

		void clear(token_t t)
		{
			std::unique_lock<std::mutex> lck(mtx);
			m_tasks.erase(std::static_pointer_cast<task_t>(t));
		}

		void clear_all()
		{
			std::unique_lock<std::mutex> lck(mtx);
			m_tasks.clear();
		}

		void set_exception_handler(exception_fn_t fn)
		{
			std::unique_lock<std::mutex> lck(mtx);
			exception_handler = fn;
		}

		exception_fn_t get_exception_handler() const
		{
			std::unique_lock<std::mutex> lck(mtx);
			return exception_handler;
		}
	};
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
