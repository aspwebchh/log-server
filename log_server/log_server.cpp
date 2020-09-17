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
#include <string>
#include "mysqlHelper.h"
#include "ThreadPool.h"
#include "configMgr.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <Winsock2.h>

#include "httpServer.h"


using namespace httplib;
using namespace std;
using namespace rapidjson;
using namespace httpServer;


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

unique_ptr<string> getParam(const map<string,string>& req, const string& key) {
    return make_unique<string>(req.count(key) > 0 ? req.at(key) : "");
}

unique_ptr<string> getParam(const Request& req, const string& key) {
    return make_unique<string>(req.has_param(key.c_str()) > 0 ? req.get_param_value(key.c_str()) : "");
}

int main(void)
{
    
    threadPool::ThreadPool threadPool(10);

    auto config = configMgr::instance();

    /*Server svr;

    svr.Get("/log", [&threadPool](const Request& req, Response& res) {
        auto paramType = *getParam(req, "type");
        auto paramContent = *getParam(req, "content");
        auto paramLogTime = *getParam(req, "log_time");
        auto paramLevel = *getParam(req, "level");
        auto paramStackTrace = *getParam(req, "stack_trace");
        auto paramServer = *getParam(req, "server");

        string responseText;
        if (paramType == "" || !isDigit(paramType) ) {
            responseText = jsonResult(1, "type参数未指定或者不合法");
        } else  if (paramContent == "") {
            responseText = jsonResult(1, "请指定content参数");
        }
        else {
            int type = stoi(paramType);

            shared_ptr<data::logItem> litem = make_shared< data::logItem>();
            litem->productId = type;
            litem->content = paramContent;
            litem->level = paramLevel;
            litem->server = paramServer;
            litem->logTime = paramLogTime;
            litem->stackTrace = paramStackTrace;
      

            threadPool.execute([litem](){
                mysqlHelper::saveLog(litem.get());
            });
            
            logger::getLogger().info("保存到数据库成功");
            responseText = jsonResult(0, "操作成功");
        }

       
        res.set_content(UTF8ToGBEx(responseText), "text/json");
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
            writer.String(stampToString(item.second)->c_str());
            writer.EndObject();
        }
        writer.EndArray();
        res.set_content(buffer.GetString(), "text/json");
    });

    svr.Get("/debug", [](const Request& req, Response& res) {
       // mysqlHelper::debugPool();
     });
    

    

    svr.listen("127.0.0.1", config.port);
    */

    HttpServer httpServer;
    httpServer.get("/log", [&threadPool](map<string, string> req) {
        auto paramType = *getParam(req, "type");
        auto paramContent = *getParam(req, "content");
        auto paramLogTime = *getParam(req, "log_time");
        auto paramLevel = *getParam(req, "level");
        auto paramStackTrace = *getParam(req, "stack_trace");
        auto paramServer = *getParam(req, "server");


        string responseText;
        if (paramType == "" || !isDigit(paramType)) {
            responseText = jsonResult(1, "type参数未指定或者不合法");
        }
        else  if (paramContent == "") {
            responseText = jsonResult(1, "请指定content参数");
        }
        else {
            int type = stoi(paramType);

            shared_ptr<data::logItem> litem = make_shared< data::logItem>();
            litem->productId = type;
            litem->content = paramContent;
            litem->level = paramLevel;
            litem->server = paramServer;
            litem->logTime = paramLogTime;
            litem->stackTrace = paramStackTrace;


            threadPool.execute([litem]() {
                mysqlHelper::saveLog(litem.get());
            });

            //logger::getLogger().info("保存到数据库成功");
            responseText = jsonResult(0, "操作成功");
            return responseText;
        }
    });


    httpServer.get("/thread_pool_info", [&threadPool](map<string, string> req) {
        auto taskCount = threadPool.taskCount();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartObject();
        writer.Key("taskCount");
        writer.Int64(taskCount);
        writer.EndObject();
        return buffer.GetString();
    });

    httpServer.get("/mysql_pool_info", [&threadPool](map<string, string> req) {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        writer.StartArray();
        mysqlHelper::mysqlPoolInfo([&writer](bool use, time_t lastUseTime) {
            writer.StartObject();
            writer.Key("use");
            writer.Bool(use);
            writer.Key("lastUseTime");
            writer.String(stampToString(lastUseTime)->c_str());
            writer.EndObject();
        });
        writer.EndArray();
        auto result = buffer.GetString();
        return string(result);
    });
    
    httpServer.get("/debug", [&threadPool](map<string, string> req) {
        // mysqlHelper::debugPool();
        return "";
    });


    httpServer.server(config.port);
}