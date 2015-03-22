
#include "database.h"
#include <chrono>
#include <thread>
#define KEEPALIVE 300

DatabasePool::DatabasePool()
{
    shuttingDown = false;

}
DatabasePool::~DatabasePool()
{

    printf("Database closed\n");
}

void DatabasePool::setup(sql::Driver * myDriver)
{
lastQ = time(NULL);
   forum_add = myDriver->connect(config["mysql"]["url"].asString(), config["mysql"]["username"].asString(), config["mysql"]["password"].asString());
   forum_add->setSchema(config["mysql"]["schema"].asString());
   user_create = forum_add->prepareStatement("INSERT INTO  `user`(`username`,`email`,`password`,`birth`) VALUES (?,?,?,UNIX_TIMESTAMP(NOW()));");
   user_id_get = forum_add->prepareStatement("SELECT `id` FROM `user` WHERE `username` = ? LIMIT 1;");
   fuser_create = forum_add->prepareStatement("INSERT INTO `forum_user` (`id`) VALUES (?);");


   fpost_create = forum_add->prepareStatement("INSERT INTO `forum_post` (board_id,poster_id,topic_id,subject,body,date) VALUE(?,?,?,?,?,?)");
   ftopic_create = forum_add->prepareStatement("CALL `mustyoshi`.FORUM_TOPIC(?,?,?,?,?)");
    keep_alive = forum_add->prepareStatement("SELECT 1;");
}

void DatabasePool::addAction(Json::Value * newVal)
{
    std::unique_lock<std::mutex> mylock(listMut);
    this->actList.push_back(newVal);

}
void DatabasePool::RunThread()
{
    printf("Keep alive started\n");
    while(!shuttingDown)
    {
        if(time(NULL) - lastQ > KEEPALIVE){ //This less than elegent solution
                //Will have to do.

            sql::ResultSet * res = this->keep_alive->executeQuery();
            while(res->next()){


            }
            //Hmm? This should fix the commands out of sync?
            delete res;
            lastQ = time(NULL);
        }
        bool got_act = false;
        Json::Value myAct;
        Json::Value *pAct = NULL;
        {
            std::unique_lock<std::mutex> lock(listMut);
            if(actList.size() > 0)
            {
                pAct = actList.front();
                myAct = *(pAct);
                actList.pop_front();
            }
            lock.unlock();
        }
        if(pAct != NULL)
        {

            if(pAct != NULL)
                delete pAct;
        }

        else
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));


    }

}



