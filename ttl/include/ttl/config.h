#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <memory>
#include "string_util.h"

namespace ttl {
	/* A very simple configuration parser */
	class config {
	public:
		class transaction;
		/* Callback type for handling config file includes */
		typedef std::function<bool(const std::string&, transaction&)> include_handler_t;
	private:
		std::unordered_map<std::string, std::string> entries;
		std::string error;

		include_handler_t include_handler;
	public:
		/* Read config entries from a std::istream. */
		inline bool read(std::istream& input);

		/* Read config entries from a std::istream, using an existing transaction. */
		inline bool read(transaction& trans, std::istream& input);

		/* Read config entries from a file. */
		bool read_file(const std::string& file) {
			std::ifstream stream(file, std::ios::in);
			if (!stream.is_open() || !stream.good()) {
				this->error = "Could not open file";
				return false;
			}
			return read(stream);
		}

		/* Read config entries from a file, using an existing transaction. */
		bool read_file(transaction& trans, const std::string& file) {
			std::ifstream stream(file, std::ios::in);
			if (!stream.is_open() || !stream.good()) {
				this->error = "Could not open file";
				return false;
			}
			return read(trans, stream);
		}

		/* Try to get a config entry. Returns true if the key existed, if not found value remains unchanged. */
		bool get(const std::string& key, std::string& value) const {
			auto it = entries.find(key);
			if (it == entries.end())
				return false;
			value = it->second;
			return true;
		}

		/* Try to get a config entry and return an alternative value if not found. */
		std::string get_optional(const std::string& key, const std::string& alternative) const {
			std::string res;
			if (get(key, res)) return res;
			return alternative;
		}

		/* Get a string description of the last error encountered by read. */
		const std::string& errormsg() const {
			return this->error;
		}

		/* Set a config entry. Existing entries get overwritten. */
		void set(const std::string& key, const std::string& value) {
			entries[key]=value;
		}

		auto cbegin() const -> decltype(entries.cbegin()) { return entries.cbegin(); }
		auto cend() const -> decltype(entries.cend()) { return entries.cend(); }
		auto begin() const -> decltype(cbegin()) { return cbegin(); }
		auto end() const -> decltype(cend()) { return cend(); }
		auto size() const -> decltype(entries.size()) { return entries.size(); }

		/* Start a transaction. */
		inline transaction begin_transaction();

		/* Set a function to handle includes. */
		void set_include_handler(include_handler_t fn) { include_handler = fn; }
		/* Get the current include handler. */
		include_handler_t get_include_handler() const { return include_handler; }
	};

	/* A config transaction. */
	class config::transaction {
		std::unordered_map<std::string, std::string> tentries;
		config& cfg;
		friend class config;
		explicit transaction(config& pcfg)
			: cfg(pcfg)
		{

		}
	public:
		/* Rollback all changes. */
		void rollback() {
			tentries.clear();
		}
		/* Commit all changed entries. */
		void commit() {
			if (!changed())
				return;

			// Insert untouched entries
			for (auto& e : cfg) {
				if (!tentries.count(e.first)) {
					tentries.insert(e);
				}
			}
			// And exchange
			cfg.entries.swap(tentries);
			tentries.clear();
		}

		/* Number of changes in this transaction. */
		auto changes() const -> decltype(tentries.size()) { return tentries.size(); }
		/* changes() != 0 */
		bool changed() const { return changes() != 0; }

		/* Try to get a value. Returns the transaction's copy if changed or checks parent config if not. */
		bool get(const std::string& key, std::string& value) const {
			auto it = tentries.find(key);
			if (it == tentries.end()) {
				// Check in original config
				return cfg.get(key, value);
			}
			value = it->second;
			return true;
		}

		/* Try to get a value or return alternative if not found. */
		std::string get_optional(const std::string& key, const std::string& alternative) const {
			std::string res;
			if (get(key, res)) return res;
			return alternative;
		}

		/* Update a value in transaction. This will not affect parent config until commit is called. */
		void set(const std::string& key, const std::string& value) {
			tentries[key]=value;
		}
	};

	bool config::read(std::istream& input) {
		auto trans = this->begin_transaction();
		if (this->read(trans, input))
		{
			trans.commit();
			return true;
		}
		return false;
	}

	bool config::read(transaction& trans, std::istream& input) {
		const static std::string include_start = "include ";

		size_t cnt_line = 0;
		std::string line;
		while (std::getline(input, line)) {
			cnt_line++;
			string::trim(line);
			if (line.substr(0, 1) != "#" && !line.empty()) {
				if (string::starts_with(line, include_start)) {
					if (include_handler) {
						auto file = line.substr(include_start.size());
						string::trim(file);
						auto res = include_handler(file, trans);
						if (!res)
							return false;
					}
				}
				else {
					auto parts = string::split(line, std::string("="), 2);
					if (parts.size() != 2) {
						this->error = "Invalid entry on line " + std::to_string(cnt_line) + ": Missing \"=\"";
						return false;
					}
					string::trim(parts[0]);
					string::trim(parts[1]);
					trans.set(parts[0], parts[1]);
				}
			}
		}
		return true;
	}

	config::transaction config::begin_transaction()
	{
		return transaction(*this);
	}
}

#ifdef TTL_OLD_NAMESPACE
namespace thalhammer = ttl;
#endif
