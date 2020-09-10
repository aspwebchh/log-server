
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
    namespace pool {
        struct PoolItem {
            sql::Connection* conn;
            bool use;
            std::time_t lastUseTime;
        };

        vector<PoolItem> pool;

        const int MAX_POOL_SIZE = 10;
        const int MIN_POOL_SIZE = 2;

        const int IDLE_SECOND = 30;

        std::time_t currRecoveryTime = std::time(0);

        std::mutex mutex;
        std::condition_variable condition;

        sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();;

        sql::Connection* createConnection() {
            sql::Connection* conn = driver->connect("localhost", "root", "123456");;
            return conn;
        }

        sql::Connection* newConnection() {
            sql::Connection* conn = createConnection();
            
            PoolItem poolItem;
            poolItem.conn = conn;
            poolItem.use = true;
            poolItem.lastUseTime = std::time(0);
            pool.push_back(poolItem);

            return conn;
        }

        sql::Connection* getIdleConn() {
            for (auto &poolItem : pool) {
                if (!poolItem.use) {
                    poolItem.use = true;
                    poolItem.lastUseTime = std::time(0);

                    return poolItem.conn;
                }
            }
            return NULL;
        }

        bool hasIdleConn() {
            for (auto &poolItem : pool) {
                if (!poolItem.use) {
                    return true;
                }
            }
            return false;
        }

        
        void recoveryConn() {
            auto now = time(0);
            if (now - currRecoveryTime < IDLE_SECOND) {
                return;
            }
            vector<PoolItem>::iterator it;
            for (it = pool.begin(); it != pool.end();) {
                auto item = *it;

                auto lastUseTime = item.lastUseTime;
                auto now = std::time(0);
                auto diff = now - lastUseTime;
                if (!item.use && diff >= IDLE_SECOND && pool.size() > MIN_POOL_SIZE) {
                    item.conn->close();
                    it = pool.erase(it);
                }
                else {
                    it++;
                }
            }
            currRecoveryTime = std::time(0);
        }


        sql::Connection* getConnection() {
            std::unique_lock<std::mutex> lock(mutex);

            recoveryConn();

            if (pool.size() < MIN_POOL_SIZE) {
                sql::Connection* conn = newConnection();
                return conn;
            }

            sql::Connection* idleConn = getIdleConn();
            if (idleConn != NULL) {
                return idleConn;
            }

            if (pool.size() < MAX_POOL_SIZE) {
                sql::Connection* conn = newConnection();
                return conn;
            }
            else {
                condition.wait(lock, [] {return hasIdleConn(); });
                return getIdleConn();
            }
        }

        void returnConnection(sql::Connection* conn) {
            std::unique_lock<std::mutex> lock(mutex);
            for (auto &item : pool) {
                if (item.conn == conn) {
                    item.use = false;
                    condition.notify_one();
                    break;
                }
            }
        }
    }

    

    void saveLog(const string& content) {
        auto conn = pool::getConnection();

        conn->setSchema("log");
        auto statement = conn->prepareStatement("insert into log (content,add_time) values (?,now())");

        auto sb = new StringBuf(content);
        auto is = new istream(sb);

        statement->setBlob(1, is);
        auto execResult = statement->execute();
        statement->close();
        
        pool::returnConnection(conn);

        delete is;
        delete sb;
    }

    void debugPool() {
        
    }

    vector<pair<bool,time_t>> mysqlPoolInfo() {
        vector<pair<bool, time_t>> result;
        for (auto& item : pool::pool) {
            auto resultItem = make_pair(item.use, item.lastUseTime);
            result.push_back(resultItem);
        }
        return result;
    }
}