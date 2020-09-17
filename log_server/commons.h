#pragma once

#include <string>
#include <windows.h>
#include <sstream>
#include <string>
#include <memory>
#include "data.h"
#include <vector>

using namespace std;

std::string UTF8ToGBEx(const std::string& utf8)
{
    if (utf8.empty())
        return "";

    std::stringstream ss;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    ss << str;
    delete[]wstr;
    delete[]str;
    return ss.str();
}


std::string GBToUTF8Ex(const std::string& gb2312)
{
    if (gb2312.empty())
        return "";

    std::stringstream ss;
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    ss << str;
    delete[]wstr;
    delete[]str;
    return ss.str();
}

bool isDigit(const std::string& str) {
    for (auto item : str) {
        if (!isdigit(item)) {
            return false;
        }
    }
    return true;
}

class StringBuf : public std::streambuf
{
public:
    StringBuf(const std::string& buffer)
    {
        char* d = (char*)buffer.data();
        setg(d, d, d + buffer.size());
    }
};


typedef struct times
{
    int Year;
    int Mon;
    int Day;
    int Hour;
    int Min;
    int Second;
}Times;

shared_ptr<string> stampToString(time_t tick) {
    struct std::tm tm;
    char s[100];
    Times standard;

    tm = *localtime(&tick);
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);


    standard.Year = atoi(s);
    standard.Mon = atoi(s + 5);
    standard.Day = atoi(s + 8);
    standard.Hour = atoi(s + 11);
    standard.Min = atoi(s + 14);
    standard.Second = atoi(s + 17);

    shared_ptr<string> result = make_shared<string>();
    result->append(to_string(standard.Year));
    result->append("-");
    result->append(to_string(standard.Mon));
    result->append("-");
    result->append(to_string(standard.Day));
    result->append(" ");
    result->append(to_string(standard.Hour));
    result->append(":");
    result->append(to_string(standard.Min));
    result->append(":");
    result->append(to_string(standard.Second));

    return result;
};

static void split(const string& s, vector<string>& tokens, const string& delimiters = " ") {
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11 
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}