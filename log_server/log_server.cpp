#include "httplib.h"
#include "logger.h"
#include <string>
#include <windows.h>
#include <sstream>
#include "commons.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "winsock.h"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include "mysqlHelper.h"
#include "ThreadPool.h"
#include "configMgr.h"



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

int main(void)
{
    Server svr;
    
    threadPool::ThreadPool threadPool(10);

    svr.Get("/log", [&threadPool](const Request& req, Response& res) {
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
            
            threadPool.execute([type, paramContent](){
                mysqlHelper::saveLog(type, paramContent);
            });
            
            logger::getLogger().info("保存到数据库成功");
            responseText = jsonResult(0, "操作成功");
        }

       
        res.set_content(GBToUTF8Ex(responseText), "text/json");
    });

    svr.Get("/thread_pool_info", [&threadPool](const Request& req, Response& res) {
        auto taskCount = threadPool.taskCount();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartObject();
        writer.Key("taskCount");
        writer.Int64(taskCount);
        writer.EndObject();
        res.set_content(buffer.GetString(), "text/json");
    });

    svr.Get("/mysql_pool_info", [](const Request& req, Response& res) {
        auto poolInfo = mysqlHelper::mysqlPoolInfo();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartArray();
        for (auto& item : poolInfo) {
            writer.StartObject();
            writer.Key("use");
            writer.Bool(item.first);
            writer.Key("lastUseTime");
            writer.String(stamp2String(item.second).c_str());
            writer.EndObject();
        }
        writer.EndArray();
        res.set_content(buffer.GetString(), "text/json");
    });

    svr.Get("/debug", [](const Request& req, Response& res) {
       // mysqlHelper::debugPool();
     });;

    auto config = configMgr::instance();
    svr.listen("localhost", config->port);
}