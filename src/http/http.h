#ifndef HTTP_STUFF
#define HTTP_STUFF
#include <websocketpp/server.hpp>


#include <jsoncpp/json/json.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <openssl/sha.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <string>
#include <cstring>
#include <websocketpp/config/asio_no_tls.hpp>
#include "database.h"
#include "../../main.h"
#include "../../msty/strtools.h"
#include "../forum/Forum.h"
extern "C" {
#include <openssl/sha.h>
}
typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;
struct action
{
    action (connection_hdl h, Json::Value m)
        : hdl(h), jmsg(m),timestart(time(NULL)) {};

    Json::Value jmsg;
    websocketpp::connection_hdl hdl;
    unsigned long timestart;

};

class ybb_handler
{
public:
    ybb_handler();
    void run(uint16_t port);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void process_messages();
    bool validate(connection_hdl hdl);
    void setDB(DatabasePool * newDB);
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    DatabasePool * dbpool;
    server m_server;
    con_list m_connections;
    std::queue<action> m_actions;

    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
};





#endif
