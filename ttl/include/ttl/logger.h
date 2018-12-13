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
		std::atomic<loglevel> level;
		mutable std::mutex mtx;
		std::function<bool(loglevel, const std::string&, const std::string&)> check_fn;
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

		void set_loglevel(loglevel l) {
			level = l;
		}

		loglevel get_loglevel() {
			return level;
		}

		void set_check_function(check_function_t fn) {
			std::unique_lock<std::mutex> lck(mtx);
			check_fn = fn;
		}

		check_function_t get_check_function() const {
			std::unique_lock<std::mutex> lck(mtx);
			return check_fn;
		}

		void log(loglevel l, const std::string& module, const std::string& message)
		{
			if (l >= level) {
				{
					std::unique_lock<std::mutex> lck(mtx);
					if(check_fn && !check_fn(l, module, message))
						return;
				}
				this->write(l,module, message);
			}
		}

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
			}
			~stream() {
				log->log(level, module, str.str());
			}
			loglevel get_loglevel() {
				return level;
			}
			template<typename T>
			friend ptr operator<<(ptr lhs, T const& rhs);
			friend ptr operator<<(ptr lhs, const loglevel& ll);
			friend ptr operator<<(ptr lhs, const logmodule& ll);
		};

		stream::ptr operator()(loglevel lvl, const std::string& module);
	};

	template<typename T>
	inline logger::stream::ptr operator<<(logger& lhs, T const& rhs)
	{
		auto stream = std::make_shared<logger::stream>(&lhs);
		stream << rhs;
		return stream;
	}

	template<typename T>
	inline logger::stream::ptr operator<<(logger& lhs, loglevel ll)
	{
		auto stream = std::make_shared<logger::stream>(&lhs);
		stream->level = ll;
		return stream;
	}

	inline logger::stream::ptr operator<<(logger::stream::ptr lhs, const logmodule& rhs)
	{
		lhs->module = rhs.name;
		return lhs;
	}

	inline logger::stream::ptr operator<<(logger::stream::ptr lhs, const loglevel& rhs)
	{
		lhs->level = rhs;
		return lhs;
	}

	template<typename T>
	inline logger::stream::ptr operator<<(logger::stream::ptr lhs, const T& rhs)
	{
		if (lhs->level >= lhs->level_output) {
			lhs->str << rhs;
		}
		return lhs;
	}

	inline logger::stream::ptr logger::operator()(loglevel lvl, const std::string& module) {
		auto stream = std::make_shared<logger::stream>(this);
		stream << lvl << logmodule(module);
		return stream;
	}

	class streamlogger: public logger {
		std::ostream& ostream;
		std::atomic<bool> autoflush;
		std::string time_format = "%c";
		mutable std::mutex mtx;

		void write(loglevel l, const std::string& module, const std::string& message) override
		{
			std::string strlevel;
			switch (l) {
			case loglevel::ERR: strlevel = "ERROR"; break;
			case loglevel::WARN: strlevel = "WARN "; break;
			case loglevel::INFO: strlevel = "INFO "; break;
			case loglevel::DEBUG: strlevel = "DEBUG"; break;
			case loglevel::TRACE: strlevel = "TRACE"; break;
			default: strlevel = "?????"; break;
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
