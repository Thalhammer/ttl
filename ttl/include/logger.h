#pragma once
#include <string>
#include <mutex>
#include <sstream>
#include <memory>
#include <iomanip>
#include <chrono>

namespace thalhammer {
	// A threadsafe logger implementation replacement for std::cout
	// https://stackoverflow.com/questions/7839565/logging-levels-logback-rule-of-thumb-to-assign-log-levels
	enum class loglevel {
		TRACE = 0,
		DEBUG,
		INFO,
		WARN,
		ERR
	};
	class logger {
		loglevel level;
		std::ostream& ostream;
		std::mutex mtx;

		std::string time_format = "%C";
	public:
		logger(std::ostream& stream)
			: ostream(stream)
		{
			set_loglevel(loglevel::INFO);
		}

		void set_loglevel(loglevel l) {
			level = l;
		}

		loglevel get_loglevel() {
			return level;
		}

		void log(loglevel l, const std::string& module, const std::string& message)
		{
			if (l >= level) {
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
					ostream << std::put_time(&t, "%c") << " | " << strlevel << " | " << module << " | " << message << std::endl;
				}
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
			stream(logger* l) {
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
			friend ptr& operator<<(ptr& lhs, T const& rhs);
		};

		stream::ptr operator()(loglevel lvl, const std::string& module);
	};

	struct logmodule {
		std::string name;
		logmodule(std::string n) : name(n) {}
	};

	template<typename T>
	inline logger::stream::ptr operator<<(logger& lhs, T const& rhs)
	{
		auto stream = std::make_shared<logger::stream>(&lhs);
		stream << rhs;
		return stream;
	}

	template<>
	inline logger::stream::ptr& operator<<(logger::stream::ptr& lhs, logmodule const& rhs)
	{
		lhs->module = rhs.name;
		return lhs;
	}

	template<>
	inline logger::stream::ptr& operator<<(logger::stream::ptr& lhs, loglevel const& rhs)
	{
		lhs->level = rhs;
		return lhs;
	}

	template<typename T>
	inline logger::stream::ptr& operator<<(logger::stream::ptr& lhs, T const& rhs)
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
}