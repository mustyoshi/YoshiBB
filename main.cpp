
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "src/forum/Forum.h"
#include "src/forum/Groups.h"
#include "src/forum/uint256.h"
#include "src/http/http.h"
#include <websocketpp/server.hpp>
#include <time.h>
#include <signal.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <websocketpp/common/thread.hpp>
#include <openssl/sha.h>
#include <iostream>
#include <fstream>

using namespace std;

bool cleanedWsiContext = true;
sql::mysql::MySQL_Driver *myDriver; //Declared in main.
//ThreadPool * poolInstance = NULL;
bool serverup = false;
//server endpoint(server::handler::ptr(new yoshibb::yoshibb_handler()));
bool caught;
void signal_callback_handler(int signum)
{

    if(!caught)
    {
        caught = true;

        //meb.DumpDB();
        printf("Stopped server\n");
        printf("Died pool\n");
        MainDBPool->shuttingDown = true;
        //libwebsocket_context_destroy(wsiContext);

        // Terminate program
        exit(signum);
    }
}
Json::Value config;
Forum forum;
DatabasePool * MainDBPool;
int main()
{

    signal(SIGINT, signal_callback_handler);
    signal(SIGABRT, signal_callback_handler);
    signal(SIGFPE, signal_callback_handler);
    signal(SIGILL, signal_callback_handler);
    signal(SIGSEGV, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);
    signal(SIGHUP, signal_callback_handler);
    signal(SIGQUIT, signal_callback_handler);


    myDriver = sql::mysql::get_mysql_driver_instance();
    MainDBPool = new DatabasePool();
    Json::Reader reader;
    std::ifstream config_file("config.json", std::ifstream::binary);
    if(reader.parse( config_file, config, false ))
    {
        printf("Read config file in\n");
        printf(config["mysql"]["password"].asString().c_str());
    }
    else
    {
        printf(reader.getFormatedErrorMessages().c_str());
        printf("\nFailed to read config.json, stopping now!\n");
        exit(-1);
    }

    sql::Connection * myConn = myDriver->connect(config["mysql"]["url"].asString(), config["mysql"]["username"].asString(), config["mysql"]["password"].asString());

    sql::Statement *stmt;
    sql::ResultSet  *res,*res2,*res3,*res4, *res5, *res6;
// ...
    myConn->setSchema("mustyoshi");
    stmt = myConn->createStatement();

    printf("wat\n");
    forum.postCount = forum.threadCount = 0;
    res = stmt->executeQuery("SELECT id,parent,name,description FROM `mustyoshi`.`forum_board` WHERE 1 ORDER BY parent ASC");
    while(res->next())
    {
        Forum_Board * newBoard = new Forum_Board();
        newBoard->id = res->getUInt("id");
        newBoard->name = res->getString("name");
        newBoard->description = res->getString("description");
        unsigned int pid = res->getUInt("parent");
        if(pid != 0)
        {
            Forum_Board * parent = forum.getBoardById(pid);
            if(parent == NULL)
            {

                printf("How could it be null, this should not happen\n");
            }
            newBoard->parent  = parent;
            parent->children.push_back(newBoard);
            forum.addBoard(newBoard);


        }
        else
        {
            newBoard->parent = NULL;
            forum.addTLBoard(newBoard);
        }
    }
    int count = 0;
    res2 = stmt->executeQuery("SELECT `user`.`id` as id,`user`.`username` as username, `forum_user`.`signature` as signature, `user`.`password` as `password`, `user`.`email` as `email` FROM `mustyoshi`.`user`,`mustyoshi`.`forum_user` WHERE `user`.`id` = `forum_user`.`id` GROUP BY `user`.`id`");
    while(res2->next())
    {

        count++;
        Forum_Acct * newUser = new Forum_Acct();
        newUser->password = 0;
        newUser->id = (unsigned long)res2->getInt("id");
        newUser->username = res2->getString("username");
        printf("Loaded %s\n",newUser->username.c_str());
        //newUser->password = res2->getString("password");
        newUser->signature = res2->getString("signature");
        newUser->email = res2->getString("email");
        std::istream * pof = res2->getBlob("password");
        pof->read((char*)(newUser->password.pn),64);
        delete pof;
        forum.users.insert((Forum_Acct_S*)newUser);
        forum.users_name.insert(newUser);
        if(newUser->id > forum.usersSize)
            forum.usersSize = newUser->id;

    }
    printf("%d users loaded\n");
    count = 0;
    for(int i=0; i<forum.boards.size(); i++)
    {


        Forum_Board * boid = forum.boards[i];
        printf("Filling [%s] ",boid->name.c_str());
        int thrd = 0;
        int psts = 0;
        unsigned long start =time(NULL);
        res3 = stmt->executeQuery("SELECT * FROM forum_topic WHERE board_id = " + std::to_string(boid->id) + " ORDER BY id ASC");
        while(res3->next())
        {
            thrd++;
            Forum_Thread * newThread = new Forum_Thread();
            newThread->id = res3->getInt("id");
            forum.thread_posts.insert((Forum_Thread_S*)newThread);
            res4 = stmt->executeQuery("SELECT * FROM forum_post WHERE topic_id = " + std::to_string(newThread->id) + " ORDER BY id ASC");
            boid->insertThread(newThread);
            while(res4->next())
            {
                psts++;
                Forum_Post * newPost = new Forum_Post();
                newPost->id = res4->getInt("id");

                newPost->body = res4->getString("body");
                newPost->subject = res4->getString("subject");
                newPost->posted = res4->getInt("date");
                Forum_Acct_S * srcher =  new Forum_Acct_S(res4->getInt("poster_id"));
                Forum_Acct* rslts = (Forum_Acct*)(forum.users.find(srcher)->key);

                newPost->poster =rslts;


                delete srcher;
                newThread->insertPost(newPost);
            }
            delete res4;

        }
        printf("%d threads, %d posts in %lu\n",thrd,psts,(unsigned long)time(NULL) - start);
    }
    delete res2;
    res2 = stmt->executeQuery("SELECT * FROM forum_user_group");
    UserGroup * testGroup = NULL;
    while(res2->next())
    {

        UserGroup * newGroup = new UserGroup(res2->getInt("id"),res2->getUInt("default_permission"));
        newGroup->name = res2->getString("name");
        newGroup->desc = res2->getString("description");
        delete res3;
        res3 = stmt->executeQuery("SELECT `id`,`board_id`,`permission_level` FROM forum_user_group_rule WHERE group_id = " + res2->getString("id"));
        while(res3->next()){
            PermRule* newRule = new PermRule();
            newRule->scope = 0;
            newRule->f_id = res3->getInt("board_id");
            newRule->perm =  res3->getUInt("permission_level");
            printf("Board %d perm %d\n",newRule->f_id,newRule->perm);
            newGroup->AddRule(newRule);
        }
        if(newGroup->id == 1)
            testGroup = newGroup;
        newGroup->CompileRules();
        printf("Group %s added\n",newGroup->name.c_str());
        forum.groups.push_front(newGroup);


    }
    delete res2;



    myConn->close();
    printf("Guest perms on 4 = %d\n",testGroup->GetPerm(4));
    forum.guestPerm = testGroup;


    MainDBPool->setup(myDriver);
    ybb_handler server_instance;
    server_instance.setDB(MainDBPool);
    // Start a thread to run the processing loop
    thread t(bind(&ybb_handler::process_messages,&server_instance));
    thread t2(bind(&DatabasePool::RunThread,MainDBPool));

    // Run the asio loop with the main thread


    printf("Started websocket server\n");
    server_instance.run(12346);
    t2.join();

    t.join();




    return 0;
}
