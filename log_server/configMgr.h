#pragma once
#include <cstddef>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include <iostream>
#include <mysql_connection.h>

using namespace std;
using namespace rapidjson;

namespace configMgr {

	struct Config {
		int port;
		string env;
		string mySqlHost;
		string mysqlUser;
		string mysqlPassword;
	};

	string readText(const string& filePath) {
		ifstream infile;
		infile.open(filePath.data());   //将文件流对象与文件连接起来 

		string result;
		string s;
		while (getline(infile, s))
		{
			result += s + "\n";
		}
		infile.close();
		return result;
	}

	Config config;

	void init() {
		auto configText = readText("config.json");
		Document doc;
		doc.Parse(configText.c_str());

		config.port = doc["port"].GetInt();
		config.mySqlHost = doc["mysql_host"].GetString();
		config.mysqlUser = doc["mysql_user"].GetString();
		config.mysqlPassword = doc["mysql_password"].GetString();
	}

	Config& instance() {
		if (config.port == 0) {
			init();
		}
		return config;
	}
}