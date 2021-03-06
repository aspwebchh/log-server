
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
#include "configMgr.h"
#include <functional>

using namespace std;
using namespace data;

namespace mysqlHelper {
    namespace pool {
        struct PoolItem {
            sql::Connection* conn;
            bool use;
            std::time_t lastUseTime;
        };

        vector<PoolItem> pool;

        const int MAX_POOL_SIZE = 10;
        const int MIN_POOL_SIZE = 1;

        const int IDLE_SECOND = 30;

        std::time_t currRecoveryTime = std::time(0);

        std::mutex mutex;
        std::condition_variable condition;

        sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();

        sql::Connection* createConnection() {
            auto config = configMgr::instance();
            sql::Connection* conn = driver->connect(config.mySqlHost, config.mysqlUser, config.mysqlPassword);
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
                    delete item.conn;
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


    class SaveLog {
    private:
        sql::Connection* conn ;

    public:
        void save(const logItem * logItem) {
            conn = pool::getConnection();

            conn->setSchema("log");
            auto ptr = conn->prepareStatement("insert into log (type,content,add_time,level,server,stack_trace) values (?,?,?,?,?,?)");
            shared_ptr<sql::PreparedStatement> statement(ptr, [](sql::PreparedStatement * ptr) {
                ptr->close();
                delete ptr;
            });

            auto sb = make_shared<StringBuf>(logItem->content);
            auto is = make_shared<istream>(sb.get());

            statement->setInt(1, logItem->productId);

            statement->setBlob(2, is.get());
            statement->setString(3, logItem->logTime);
            statement->setString(4, logItem->level);
            statement->setString(5, logItem->server);
            statement->setString(6, logItem->stackTrace);


            auto execResult = statement->execute();
        }

        ~SaveLog() {
            if (conn != NULL) {
                pool::returnConnection(conn);
            }
        }
    };
    



    void saveLog(const logItem * logItem) {
        SaveLog sv;
        try {
            sv.save(logItem);
        }
        catch (sql::SQLException e) {
            const string what = e.what();
            logger::getLogger().error(what);
        }
        catch (sql::InvalidArgumentException e) {
            const string what = e.what();
            logger::getLogger().error(what);
        }
        catch (std::bad_alloc e) {
            const string what = e.what();
            logger::getLogger().error(what);
        }
        catch (std::exception e) {
            const string what = e.what();
            logger::getLogger().error(what);
        }
        catch (...) {
            logger::getLogger().error("δ֪�쳣");
        }
    }

    void debugPool() {
        
    }

    void mysqlPoolInfo(function<void(bool,time_t)> onItem) {
        for (auto& item : pool::pool) {
            onItem(item.use, item.lastUseTime);
        }
    }
}