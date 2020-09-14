#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include "commons.h"
#include <string>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

using namespace std;

namespace logger {
	class Logger {
	public:
		Logger(){
			string format = "[%Severity%]%Message%";

			logging::add_file_log(
				keywords::auto_flush = true,
				keywords::file_name = "log/log_%Y-%m-%d_%H.%N.log",
				keywords::rotation_size = 10 * 1024 * 1024,
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
				keywords::format = format,
				keywords::min_free_space = 3 * 1024 * 1024
			);
			logging::add_console_log(std::cout, boost::log::keywords::format = format);

			logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
		}

		void trace(const string& msg) {
			BOOST_LOG_TRIVIAL(trace) << (*getMessage(msg));
		}

		void debug(const string& msg) {
			BOOST_LOG_TRIVIAL(debug) << (*getMessage(msg));
		}

		void info(const string& msg) {
			BOOST_LOG_TRIVIAL(info) << (*getMessage(msg));
		}

		void warning(const string& msg) {
			BOOST_LOG_TRIVIAL(warning) << (*getMessage(msg));
		}

		void error(const string& msg) {
			BOOST_LOG_TRIVIAL(error) << (*getMessage(msg));
		}

		void fatal(const string& msg) {
			BOOST_LOG_TRIVIAL(fatal) << (*getMessage(msg));
		}

	private:
		shared_ptr<string> getMessage(const string& msg) {
			auto nowPtr = getNow();
			string & nowString = *nowPtr;
			auto finalMsg = "[" + nowString + "]" + msg;
			return make_shared<string>(finalMsg);
		}

		shared_ptr<string> getNow() {
			auto timestamp = std::time(0);
			return stampToString(timestamp);
		}
	};


	Logger logger;

	Logger& getLogger() {
		return logger;
	}
}