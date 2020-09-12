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


using namespace std;
using namespace rapidjson;

namespace configMgr {
	struct Config {
		int port;
	} ;


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


	Config* config;

	Config* instance() {
		if (config == nullptr) {
			auto configText = readText("config.json");
			Document doc;
			doc.Parse(configText.c_str());
			int port = doc["port"].GetInt();

			Config* cfg = new Config;
			cfg->port = port;
			config = cfg;
			return config;
		}
		else {
			return config;
		}
	}
}