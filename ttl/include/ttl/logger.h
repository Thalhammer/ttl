#pragma once
#include <string>
#include <mutex>
#include <sstream>
#include <memory>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <functional>

namespace ttl {
	// A threadsafe logger implementation replacement for std::cout
	// https://stackoverflow.com/questions/7839565/logging-levels-logback-rule-of-thumb-to-assign-log-levels
	enum class loglevel {
		TRACE = 0,
		DEBUG,
		INFO,
		WARN,
		ERR
	};
	struct logmodule {
		std::string name;
		explicit logmodule(std::string n) : name(n) {}
	};
	class logger {
		std::atomic<loglevel> m_level;
		mutable std::mutex m_mtx;
		std::function<bool(loglevel, const std::string&, const std::string&)> m_check_fn;
	protected:
		virtual void write(loglevel l, const std::string& module, const std::string& msg) = 0;
	public:
		typedef std::function<bool(loglevel l, const std::string& module, const std::string& message)> check_function_t;
		logger()
		{
			set_loglevel(loglevel::INFO);
		}

		explicit logger(loglevel level)
		{
			set_loglevel(level);
		}

		logger(loglevel level, check_function_t fn)
		{
			set_loglevel(level);
			set_check_function(fn);
		}

		virtual ~logger() {}

		void set_loglevel(loglevel l) {
			m_level = l;
		}

		loglevel get_loglevel() {
			return m_level;
		}

		void set_check_function(check_function_t fn) {
			std::unique_lock<std::mutex> lck(m_mtx);
			m_check_fn = fn;
		}

		check_function_t get_check_function() const {
			std::unique_lock<std::mutex> lck(m_mtx);
			return m_check_fn;
		}

		void log(loglevel l, const std::string& module, const std::string& message)
		{
			if (l >= m_level) {
				{
					std::unique_lock<std::mutex> lck(m_mtx);
					if(m_check_fn && !m_check_fn(l, module, message))
						return;
				}
				this->write(l,module, message);
			}
		}

		void trace(const std::string& module, const std::string& message) { log(loglevel::TRACE, module, message); }
		void debug(const std::string& module, const std::string& message) { log(loglevel::DEBUG, module, message); }
		void info(const std::string& module, const std::string& message) { log(loglevel::INFO, module, message); }
		void warn(const std::string& module, const std::string& message) { log(loglevel::WARN, module, message); }
		void error(const std::string& module, const std::string& message) { log(loglevel::ERR, module, message); }

		class stream {
			loglevel level = loglevel::INFO;
			loglevel level_output;
			std::string module;
			std::ostringstream str;
			logger* log;
		public:
			typedef std::shared_ptr<stream> ptr;
			explicit stream(logger* l) {
				log = l;
				level_output = log->get_loglevel(); // Safe loglevel at creation
				level = level_output;
			}
			template<typename T>
			stream(logger* l, T const& e) {
				log = l;
				level_output = log->get_loglevel(); // Safe loglevel at creation
				level = level_output;
				if (level >= level_output) {
					str << e;
				}
			}
			stream(logger* l, const logmodule& e) {
				log = l;
				level_output = log->get_loglevel(); // Safe loglevel at creation
				level = level_output;
				module = e.name;
			}
			stream(logger* l, const loglevel& e) {
				log = l;
				level_output = log->get_loglevel(); // Safe loglevel at creation
				level = e;
			}
			stream(logger* l, const logmodule& e, const loglevel& lvl) {
				log = l;
				level_output = log->get_loglevel(); // Safe loglevel at creation
				level = lvl;
				module = e.name;
			}
			stream(const stream&) = delete;
			stream(stream&&) = delete;
			~stream() {
				log->log(level, module, str.str());
			}
			loglevel get_loglevel() {
				return level;
			}
			// NOTE: We would want to have those functions take a non const reference as first arg,
			// but C++11 does not allow for that.
			template<typename T>
			friend const stream& operator<<(const stream& lhs, T const& rhs);
			friend const stream& operator<<(const stream& lhs, const loglevel& ll);
			friend const stream& operator<<(const stream& lhs, const logmodule& ll);
		};

		stream operator()(loglevel lvl, const std::string& module);
	};

	template<typename T>
	inline logger::stream operator<<(logger& lhs, T const& rhs)
	{
		return {&lhs, rhs};
	}

	inline logger::stream operator<<(logger& lhs, loglevel ll)
	{
		return {&lhs, ll};
	}

	inline logger::stream logger::operator()(loglevel lvl, const std::string& module) {
		return {this, logmodule(module), lvl};
	}

	template<typename T>
	inline const logger::stream& operator<<(const logger::stream& lhs, const T& rhs)
	{
		auto& e = const_cast<logger::stream&>(lhs);
		if (e.level >= e.level_output) {
			e.str << rhs;
		}
		return lhs;
	}

	inline const logger::stream& operator<<(const logger::stream& lhs, const logmodule& rhs)
	{
		auto& e = const_cast<logger::stream&>(lhs);
		e.module = rhs.name;
		return lhs;
	}

	inline const logger::stream& operator<<(const logger::stream& lhs, const loglevel& rhs)
	{
		auto& e = const_cast<logger::stream&>(lhs);
		e.level = rhs;
		return lhs;
	}

	class streamlogger: public logger {
		std::ostream& ostream;
		std::atomic<bool> autoflush;
		std::string time_format = "%c";
		mutable std::mutex mtx;

		void write(loglevel l, const std::string& module, const std::string& message) override
		{
			std::string strlevel = "?????";
			switch (l) {
			case loglevel::ERR: strlevel = "ERROR"; break;
			case loglevel::WARN: strlevel = "WARN "; break;
			case loglevel::INFO: strlevel = "INFO "; break;
			case loglevel::DEBUG: strlevel = "DEBUG"; break;
			case loglevel::TRACE: strlevel = "TRACE"; break;
			}
			time_t nt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm t;
#ifdef _WIN32
			localtime_s(&t, &nt);
#else
			localtime_r(&nt, &t);
#endif
			{
				std::unique_lock<std::mutex> lck(mtx);
				ostream << std::put_time(&t, time_format.c_str()) << " | " << strlevel << " | " << module << " | " << message << std::endl;
				if(autoflush)
					ostream.flush();
			}
			
		}
	public:
		typedef std::function<bool(loglevel l, const std::string& module, const std::string& message)> check_function_t;
		explicit streamlogger(std::ostream& stream)
			: logger(loglevel::INFO), ostream(stream)
		{
			set_autoflush(true);
		}

		streamlogger(std::ostream& stream, loglevel level)
			: logger(level), ostream(stream)
		{
		}

		streamlogger(std::ostream& stream, loglevel level, check_function_t fn)
			: logger(level, fn), ostream(stream)
		{
		}

		void set_autoflush(bool b) {
			autoflush = b;
		}

		bool get_autoflush() const {
			return autoflush;
		}

		std::ostream& get_ostream() {
			return ostream;
		}

		void set_timeformat(std::string str) {
			std::unique_lock<std::mutex> lck(mtx);
			time_format = std::move(str);
		}

		std::string get_timeformat() const {
			std::unique_lock<std::mutex> lck(mtx);
			return time_format;
		}
	};
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
