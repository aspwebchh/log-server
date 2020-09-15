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
	
	sql::SQLString mysql_host_dev("localhost");
	sql::SQLString mysql_user_dev("root");
	sql::SQLString mysql_password_dev("123456");

	sql::SQLString mysql_host_test("localhost");
	sql::SQLString mysql_user_test("root");
	sql::SQLString mysql_password_test("123456");


	struct Config {
		int port;
		string env;
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
		config.env = doc["env"].GetString();
	}

	Config instance() {
		if (config.port == 0) {
			init();
		}
		return config;
	}
}