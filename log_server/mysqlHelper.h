
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <mutex>
#include "commons.h"

using namespace std;

namespace mysqlHelper {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();;
    sql::Connection* conn = driver->connect("localhost", "root", "123456");;
    mutex lock;

    void saveLog(const string& content) {
        lock.lock();

        conn->setSchema("log");
        auto statement = conn->prepareStatement("insert into log (content,add_time) values (?,now())");

        auto sb = new StringBuf(content);
        auto is = new istream(sb);

        statement->setBlob(1, is);
        auto execResult = statement->execute();
        statement->close();

        delete is;
        delete sb;

        lock.unlock();
    }
}