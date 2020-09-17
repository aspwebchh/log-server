#pragma once

#include <string>

using namespace std;

//const string& content, const string& level, const string& logTime, const string& staceTrace, const string& server

namespace data {
	struct logItem {
		int productId;
		string content;
		string level;
		string logTime;
		string stackTrace;
		string server;
	};
}

