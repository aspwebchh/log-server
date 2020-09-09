#include "httplib.h"
#include <string>
#include <windows.h>
#include <sstream>
#include "commons.h"

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

#include "winsock.h"
#include "mysql.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>

using namespace httplib;
using namespace std;
using namespace rapidjson;

std::string jsonResult(const int & code, const std::string & msg) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("code");
    writer.Int(code);
    writer.Key("message");
    writer.String(msg.c_str());
    writer.EndObject();
    return buffer.GetString();
}


void save2MySql(const int& type, const std::string&  content) {
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* conn;
    sql::Statement* state;
    sql::ResultSet* result;
    driver = sql::mysql::get_driver_instance();
    conn = driver->connect("localhost", "root", "123456");
    conn->setSchema("log");

    auto statement = conn->prepareStatement("insert into log (content,add_time) values (?,now())");
    
    auto sb = new StringBuf(content);
    auto is = new istream(sb);

    statement->setBlob(1, is);
    auto execResult = statement->execute();

    statement->close();
    conn->close();

    delete is;
    delete sb;
}

int main(void)
{
    Server svr;

    svr.Get("/log", [](const Request& req, Response& res) {
        auto paramType = req.has_param("type") ? req.get_param_value("type") : "";
        auto paramContent = req.has_param("content") ? req.get_param_value("content") : "";
        string responseText;
        if (paramType == "" || !isDigit(paramType) ) {
            responseText = jsonResult(1, "type参数未指定或者不合法");
        } else  if (paramContent == "") {
            responseText = jsonResult(1, "请指定content参数");
        }
        else {
            int type = stoi(paramType);
            save2MySql(type, paramContent);
            responseText = jsonResult(0, "操作成功");
        }

       
        res.set_content(GBToUTF8Ex(responseText), "text/json");
    });

   
    svr.listen("localhost", 1234);
}