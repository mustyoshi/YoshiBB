#ifndef traderdbb
#define traderdbb
#include <mutex>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#ifndef parr
#define parr(a,b) (*a)[b]
#endif
extern Json::Value config;
class DatabasePool
{

public:

    sql::Connection * forum_add;
    sql::PreparedStatement * user_create;
    sql::PreparedStatement * user_id_get;
    sql::PreparedStatement * fuser_create;

    sql::PreparedStatement * fpost_create;
    sql::PreparedStatement * ftopic_create;
    sql::PreparedStatement * user_in_groups;


    sql::PreparedStatement * keep_alive;
    std::list<Json::Value*> actList;
    std::mutex listMut;
    bool shuttingDown;
    void setup(sql::Driver * myDriver);
    void updateBal(unsigned int id,unsigned int good,unsigned long vol);
    void addAction(Json::Value * newAct);
    void RunThread();
    long lastQ;
    DatabasePool();
    ~DatabasePool();
};
extern DatabasePool * MainDBPool;
#endif
