#pragma once

#include <string>
#include <windows.h>
#include <sstream>
#include <string>



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

std::string time2string(Times times) {
    using namespace std;
    return  to_string(times.Year) + "-" + to_string(times.Mon) + "-" + to_string(times.Day) + " " + to_string(times.Hour) + ":" + to_string(times.Min) + ":" + to_string(times.Second);
}

std::string stamp2String(std::time_t tick)
{
    struct std::tm tm;
    char s[100];
    Times standard;

    //tick = time(NULL);
    tm = *localtime(&tick);
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);


    standard.Year = atoi(s);
    standard.Mon = atoi(s + 5);
    standard.Day = atoi(s + 8);
    standard.Hour = atoi(s + 11);
    standard.Min = atoi(s + 14);
    standard.Second = atoi(s + 17);

    return time2string( standard );
}

