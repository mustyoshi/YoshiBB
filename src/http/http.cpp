
#include "http.h"
/* Todo:
Change the format of the Json Values to what I expect them to be
*/
#include <websocketpp/common/thread.hpp>
#include <iostream>
#include <memory>
#include <boost/algorithm/string/replace.hpp>
#include <openssl/sha.h>
#define subpoint(a,b) ((a))[b]
#include <cmath>
#include "../forum/uint256.h"

const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};
const char SAFE[256] =
{
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

    /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};
std::string toString(int number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
int toInt(std::string number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    int num =0;
    ss >> num;
    return num; //return a string with the contents of the stream
}
std::string UriEncode(const std::string & sSrc)
{
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
    unsigned char * pEnd = pStart;
    const unsigned char * const SRC_END = pSrc + SRC_LEN;

    for (; pSrc < SRC_END; ++pSrc)
    {
        if (SAFE[*pSrc])
            *pEnd++ = *pSrc;
        else
        {
            // escape this char
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
    }

    std::string sResult((char *)pStart, (char *)pEnd);
    delete [] pStart;
    return sResult;
}
std::string UriDecode(const std::string & sSrc)
{
    // Note from RFC1630: "Sequences which start with a percent
    // sign but are not followed by two hexadecimal characters
    // (0-9, A-F) are reserved for future extension"

    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    const unsigned char * const SRC_END = pSrc + SRC_LEN;
    // last decodable '%'
    const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

    char * const pStart = new char[SRC_LEN];
    char * pEnd = pStart;

    while (pSrc < SRC_LAST_DEC)
    {
        if (*pSrc == '%')
        {
            char dec1, dec2;
            if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                    && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
            {
                *pEnd++ = (dec1 << 4) + dec2;
                pSrc += 3;
                continue;
            }
        }

        *pEnd++ = *pSrc++;
    }

    // the last 2- chars
    while (pSrc < SRC_END)
        *pEnd++ = *pSrc++;

    std::string sResult(pStart, pEnd);
    delete [] pStart;
    return sResult;
}
std::string to_hex(unsigned char s)
{
    std::stringstream ss;
    ss << std::hex << (int) s;
    if(ss.str().size() == 0)
        return "00";
    else if(ss.str().size() == 1)
        return "0"+ss.str();
    else
        return ss.str();
}

//using chat_server_handler::connection_hdl;





using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */


ybb_handler::ybb_handler()
{
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_open_handler(bind(&ybb_handler::on_open,this,::_1));
    m_server.set_close_handler(bind(&ybb_handler::on_close,this,::_1));
    m_server.set_message_handler(bind(&ybb_handler::on_message,this,::_1,::_2));
    m_server.set_validate_handler(bind(&ybb_handler::validate,this,::_1));
    m_server.clear_access_channels(websocketpp::log::alevel::all);
}
void ybb_handler::run(uint16_t port)
{
    // listen on specified port
    m_server.listen(port);

    // Start the server accept loop
    m_server.start_accept();

    // Start the ASIO io_service run loop
    try
    {
        m_server.run();
    }
    catch (const std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (websocketpp::lib::error_code e)
    {
        std::cout << e.message() << std::endl;
    }
    catch (...)
    {
        std::cout << "other exception" << std::endl;
    }
}
void ybb_handler::on_open(connection_hdl hdl)
{
    server::connection_ptr con = m_server.get_con_from_hdl(hdl);
    printf("Got connection attempt\n");
    std::cout << "client " << con->get_remote_endpoint() << " " << con->get_origin() << " joined the lobby." << std::endl;
    // m_connections.insert(std::pair<connection_hdl,std::string>(con,get_con_id(con)));
    subpoint(con->metadata,"id") = Json::Value(0);
    subpoint(con->metadata,"lastPost") = Json::Value(-1);
    subpoint(con->metadata,"group") = Json::Value(1);
    subpoint(con->metadata,"name") = Json::Value("Guest");
    subpoint(con->metadata,"session") = Json::Value("");
    subpoint(con->metadata,"boards") = Json::Value(Json::arrayValue);
    subpoint(con->metadata,"userPointer") = Json::Value(0);
    subpoint(con->metadata,"lastActive") = Json::Value(0);
    // send user list and signon message to all clients
    std::string cook = con->get_request_header("Cookie");
    std::vector<std::string> res = msty::explode(cook,";");
    for(int i=0; i<res.size(); i++)
    {
        std::vector<std::string> res2 = msty::explode(res[i],"=");
        if(res2[0] == "PHPSESSID")
        {
            //First rule, no input is safe.
            subpoint(con->metadata,"session") = Json::Value((res2[1])); //Pls don't rape me later. I didn't sanitize.
        }
    }


    Json::Value response;
    response[0u] = Json::Value("hello");
    con->send(response.toStyledString().c_str());

}

void ybb_handler::on_close(connection_hdl hdl)
{
    /*
       server::connection_ptr con = m_server.get_con_from_hdl(hdl);

       std::map<connection_hdl,std::string>::iterator it = m_connections.find(con);

       if (it == m_connections.end())
       {
           // this client has already disconnected, we can ignore this.
           // this happens during certain types of disconnect where there is a
           // deliberate "soft" disconnection preceeding the "hard" socket read
           // fail or disconnect ack message.
           con->send("Server closing\n");
           return;
       }

       const std::string alias = it->second;
       m_connections.erase(it);
       // send user list and signoff message to all clients
    */
}
bool ybb_handler::validate(connection_hdl con)
{

//Do some
//TODO: Spam fighting.
    return true;
}
void ybb_handler::on_message(connection_hdl con, server::message_ptr msg)
{
    //printf("Got message\n");
    if (msg->get_opcode() != websocketpp::frame::opcode::TEXT)
    {
        //silently die.
        return;
    }
    Json::Value jmsg;   // will contains the parsed message.
    Json::Reader reader; //To parse JSON.
    //printf("[%s]\n",msg->get_payload().c_str());
    bool parsingSuccessful = reader.parse( msg->get_payload(), jmsg );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
//silently die
        return;
    }
    unique_lock<mutex> lock(m_action_lock);
    //std::cout << "on_message" << std::endl;
    m_actions.push(action(con,jmsg));

    lock.unlock();
    m_action_cond.notify_one();

}
void ybb_handler::process_messages()
{
    double tts = 0.0;
    unsigned long requests = 0;
    while(this->dbpool->shuttingDown == false)
    {
        unique_lock<mutex> lock(m_action_lock);

        while(m_actions.empty())
        {
            m_action_cond.wait(lock);
        }

        action a = m_actions.front();

        m_actions.pop();
        //TODO: Clean up this giant function
        lock.unlock();
        try
        {
            Json::Value resp;
            bool send_resp = false;
            Json::Value req = a.jmsg;
            server::connection_ptr con = m_server.get_con_from_hdl(a.hdl);
            Forum_Acct * theUser = NULL;
            unsigned long uP = subpoint(con->metadata,"userPointer").asInt64();
            if(uP >0) //If it's greater than zero we have a pointer;
            {
                theUser = reinterpret_cast<Forum_Acct*>(uP);
            }

            unsigned long start = a.timestart;
            requests++;
            if(req[0u].asString() == "login")
            {
                printf("Hmmmm %s\n",req.toStyledString().c_str());
                if(req.size() > 1)
                {
                    Forum_Acct * srcher = new Forum_Acct();
                    Forum_Acct * srch2;
                    srcher->username = req[1].asString();

                    srch2 = (Forum_Acct*)(forum.users_name.find(srcher)->key);

                    printf("Address %p|%p\n",srch2,&srch2);

                    if(srch2 == NULL)
                    {
                        delete srcher;
                        resp[0] = "error";
                        resp[1] ="Username or Password Invalid.";
                        goto send_resp;
                    }
                    else
                    {
                        //srch2 = (Forum_Acct*)(forum.users_name.find(srcher)->key);
                        printf("Checking if password is right\n");
                        std::string psalt = std::to_string(srch2->id).append(srch2->email); //Procedure for changing email will cause password hash to change too.
                        std::string tohash = psalt + srch2->username + psalt + req[2].asString();

                        uint512 testpw = 0;

                        SHA512((unsigned char*)tohash.data(),tohash.length(),(unsigned char*)testpw.pn);
                        if(testpw.CompareTo(srch2->password) == 0)
                        {


                            printf("Password is correct\n");

                            subpoint(con->metadata,"id") = (Json::UInt64)srch2->id;
                            subpoint(con->metadata,"name") = srch2->username;
                            subpoint(con->metadata,"userPointer") = (Json::UInt64)(srch2);
                            srch2->session = subpoint(con->metadata,"session").asString();
                            resp[0] = "login";
                            resp[1] =  srch2->username;
                            resp[2] = (Json::UInt64)srch2->id;
                            resp[3] = 0;
                            MainDBPool->user_in_groups->clearParameters();
                            MainDBPool->user_in_groups->setInt(1,srch2->id);
                            sql::ResultSet * res = MainDBPool->user_in_groups->executeQuery();
                            while(res->next())
                            {
                                UserGroup * pof = forum.getGroupById(res->getInt("group_id"));
                                if(pof != NULL)
                                {
                                    printf("Group board %d\n",pof->id);
                                    srch2->groups.push_back(pof);
                                }
                            }
                            delete res;
                        }
                        else
                        {
                            delete srcher;
                            resp[0] = "login";
                            resp[1] = 0;
                            resp[2] = "Username or Password Invalid.";
                            goto send_resp;
                        }
                    }
                }
                else
                {
                    resp[0] = "login";
                    resp[1] = "Guest";
                    resp[2] = 0;
                    resp[3] = 0;
                }

            }
            else if(req[0u].asString() == "hme1")
            {
                resp[0] = "hme1";
                for(int i=0; i<forum.topboards.size(); i++)
                {
                    Forum_Board * pb = forum.topboards[i];

                    Json::Value sub;
                    sub.append(Json::Value(pb->id));
                    sub.append(Json::Value(pb->name));
                    for(int b = 0; b<pb->children.size(); b++) //Liberal use of copy and paste
                    {

                        Json::Value vec2;
                        Forum_Board * bd = pb->children[b];
                        if(theUser != NULL)
                        {
                            if(theUser->GetPerm(bd->id) < 3)
                            {
                                continue;
                            }
                        }
                        else if(forum.guestPerm->GetPerm(bd->id) <3)
                        {
                            continue;
                        }
                        vec2[0] = bd->id;
                        vec2[1] = bd->name;
                        vec2[2] = bd->description;
                        vec2[3] = bd->topics;
                        vec2[4] = bd->posts;
                        Json::Value lp;
                        Thread_Node * lastpost = NULL;
                        if(bd->laststicky == NULL)
                        {
                            lastpost = bd->threads;

                        }
                        else
                        {
                            lastpost = bd->laststicky->next;
                        }
                        if(lastpost && lastpost->Key != 0 && lastpost->Key->lastpost != 0)
                        {
                            Forum_Post * fp = lastpost->Key->lastpost->Key;
                            lp[0] = (Json::UInt64)fp->id;
                            if(fp->poster != NULL)
                            {
                                lp[1] = (Json::UInt64)fp->poster->id;

                                lp[2] =fp->poster->username;
                            }
                            else
                            {
                                lp[1] = 1;
                                lp[2] = "Error";
                            }
                            lp[3] = fp->subject;
                            lp[4] = (Json::UInt64)fp->posted;

                        }
                        else
                            lp[0] = 0; //This is the lastpost thing.
                        vec2.append(lp);
                        sub.append(vec2);
                    }
                    resp.append(sub);
                }
            }
            else if(req[0u].asString() == "hme2")
            {

                if(req.size() == 2)
                {
                    resp[0] = "hme2";
                    //Json::Value subvec;
                    if(!(req[1].isArray())) //TODO: Configurable array acceptence or not.
                    {
                        int bid = 0;
                        if(req[1].isInt())
                            bid = req[1].asInt();
                        else if(req[1].isString())
                            bid = (toInt(req[1].asString()));

                        req[1] = Json::Value();
                        req[1][0] = bid;
                        if(req[1].isArray())
                            printf("It is an array now\n");
                        else
                            printf("%s\n",req.toStyledString().c_str());
                    }
                    for(int e=0; e<req[1].size(); e++)
                    {
                        int bid = req[1][e].asInt();

                        Forum_Board * bod = forum.getBoardById(bid);
                        Json::Value vec;
                        vec[0] = bid;
                        for(int i=0; i<bod->children.size(); i++)
                        {
                            Json::Value vec2;
                            Forum_Board * bd = bod->children[i];
                            vec2[0] = bd->id;
                            vec2[1] = bd->name;
                            vec2[2] = bd->description;
                            vec2[3] = bd->topics;
                            vec2[4] = bd->posts;
                            Json::Value lp;
                            Thread_Node * lastpost = NULL;
                            if(bd->laststicky == NULL)
                            {
                                lastpost = bd->threads;

                            }
                            else
                            {
                                lastpost = bd->laststicky->next;
                            }
                            if(lastpost && lastpost->Key != 0 && lastpost->Key->lastpost != 0)
                            {
                                Forum_Post * fp = lastpost->Key->lastpost->Key;
                                lp[0] = (Json::UInt64)fp->id;
                                if(fp->poster != NULL)
                                {
                                    lp[1] = (Json::UInt64)fp->poster->id;

                                    lp[2] =fp->poster->username;
                                }
                                else
                                {
                                    lp[1] = 1;
                                    lp[2] = "Error";
                                }
                                lp[3] = fp->subject;
                                lp[4] = (Json::UInt64)fp->posted;

                            }
                            else
                                lp[0] = 0; //This is the lastpost thing.

                            vec2.append(lp);
                            vec.append(vec2);
                        }
                        resp.append(vec);
                        //subvec.append(vec);
                    }


                }
            }
            else if(req[0].asString() == "bord") //TODO: Send board name in response
                //TODO: Add way to avoid sending board name :(
            {

                int TOPPERPAGE = 20;
                printf("%s\n",req.toStyledString().c_str());
                resp[0] = "bord";
                int bord_id = req[1].asInt();
                int pagec = 1;
                if(req.size() > 2)
                {
                    printf("We got a page number request\n");
                    pagec = req[2].asInt();
                    if(req.size() > 3)
                    {

                        TOPPERPAGE = (int)fmin(100,req[3].asInt());
                    }
                }
                printf("Displaying %d per page\n",TOPPERPAGE);
                Forum_Board * bord = forum.getBoardById(bord_id);
                if(bord == NULL)
                {

                    resp[0] = "error";
                    goto send_resp;
                }
                /*TODO: Switch it up so a bord requrest also sends the
                child boards if they exist.
                resp[1] = bord_id;
                resp[2] = [
                [bord_id,name,desc,etc],
                etc
                ]
                Or it will be an empty array
                resp[3] = totalpages;
                resp[4] = [
                [post array],
                [postarray],
                etc];
                I'll have to rewrite the client to parse this new form.
                I want to reduce the number of back n forth requests that have to be made.
                */


                resp[1] = bord_id;
                resp[2] = pagec;
                resp[3] = (int)(ceil((bord->topics / (float)TOPPERPAGE)));
                resp[4] = Json::Value();
                resp[5] = Json::Value();
                Thread_Node *curThread = bord->threads;
                /*
                while(page > 0 && curThread != NULL )
                {
                    curThread = curThread->next;
                    page--;
                }
                */
                int skip = TOPPERPAGE * (pagec -1);
                printf("skip %d\n",skip);
                //We want it to stop at the page count * num pages
                for(int i=0; i<TOPPERPAGE && curThread != NULL && curThread->Key != 0;)
                {
                    if(skip > 0)
                    {
                        skip--; //TODO: We have to make this better
                        curThread = curThread->next;
                        printf("Skipping pages\n");

                        continue;
                    }
                    Forum_Thread * thred = curThread->Key;
                    if(thred == 0 || thred->firstpost == 0 ) break;
                    Forum_Acct * postr = thred->creator;
                    Forum_Post * firstp = thred->firstpost->Key;
                    Json::Value subvec;
                    std::string tempstring;
                    tempstring.assign(firstp->subject);
                    if(tempstring.length() > 60)
                    {
                        tempstring = tempstring.substr(0,57);
                        tempstring.append("...");
                    }
                    subvec[0] = tempstring;
                    subvec[1] = postr->username;
                    subvec[2] = (Json::UInt64)postr->id;
                    subvec[3] = thred->replies;
                    subvec[4] = thred->lastpost->Key->poster->username;
                    subvec[5] = (Json::UInt64)thred->lastpost->Key->posted;
                    subvec[6] = (Json::UInt64)thred->lastpost->Key->id;
                    subvec[7] = (int)thred->type;
                    subvec[8] = (Json::UInt64)thred->id;
                    resp[4].append(subvec);
                    i++;
                    if(curThread->next == curThread)
                    {
                        printf("yyyyy\n");
                        curThread->next = NULL;
                        break;
                    }
                    curThread = curThread->next;

                }
                for(int i=0; i<bord->children.size(); i++)
                {
                    Json::Value vec2;
                    Forum_Board * bd = bord->children[i];
                    vec2[0] = bd->id;
                    vec2[1] = bd->name;
                    vec2[2] = bd->description;
                    vec2[3] = bd->topics;
                    vec2[4] = bd->posts;
                    Json::Value lp;
                    Thread_Node * lastpost = NULL;
                    if(bd->laststicky == NULL)
                    {
                        lastpost = bd->threads;

                    }
                    else
                    {
                        lastpost = bd->laststicky->next;
                    }
                    if(lastpost && lastpost->Key != 0 && lastpost->Key->lastpost != 0)
                    {
                        Forum_Post * fp = lastpost->Key->lastpost->Key;
                        lp[0] = (Json::UInt64)fp->id;
                        if(fp->poster != NULL)
                        {
                            lp[1] = (Json::UInt64)fp->poster->id;

                            lp[2] =fp->poster->username;
                        }
                        else
                        {
                            lp[1] = 1;
                            lp[2] = "Error";
                        }
                        lp[3] = fp->subject;
                        lp[4] = (Json::UInt64)fp->posted;

                    }
                    else
                        lp[0] = 0; //This is the lastpost thing.

                    vec2.append(lp);
                    resp[5].append(vec2);
                }
            }
            else if(req[0].asString() == "post")
            {
                resp[0] = "post";
                resp[1] = req[1].asInt();
                int pagenum = 0;
                if(resp.size() > 2)
                    pagenum = resp[2].asInt();

                Forum_Thread_S * srchr = new Forum_Thread_S(req[1].asInt());
                Forum_Thread * thred = (Forum_Thread*)(forum.thread_posts.find(srchr)->key);
                delete srchr;
                if(thred == 0)
                {
                    resp[0] = "error";
                    resp[1] = "Post does not exist";
                    goto send_resp;
                }
                //TODO pages
                Post_Node * curPost = thred->firstpost;

                Forum_Post * pp = curPost->Key;
                while(curPost != NULL)
                {
                    pp = curPost->Key;
                    Json::Value subvec;
                    subvec[0] = (Json::UInt64)pp->id;
                    subvec[1] = (Json::UInt64)pp->poster->id;
                    subvec[2] = (Json::UInt64)pp->posted;
                    subvec[3] = pp->subject;
                    subvec[4] = pp->body;
                    subvec[5] = pp->poster->username;

                    resp.append(subvec);
                    curPost = curPost->next;
                }

            }
            else if(req[0].asString() == "reg")
            {
                printf("Hmmmm %s\n",req.toStyledString().c_str());
                Forum_Acct * srch = new Forum_Acct();
                srch->username = req[1].asString();
                Forum_Acct * rslt = (Forum_Acct*)(forum.users_name.find(srch)->key);

                if(rslt != 0)
                {
                    delete srch;
                    resp[0] = "reg";
                    resp[1] = 0;
                    resp[2] = "Username already tafken";
                    goto send_resp;
                }
                else
                {
                    std::string nms = req[1].asString();
                    if(nms.size() < 6 || nms.size() > 16)
                    {
                        delete srch;
                        resp[0] = "reg";
                        resp[1] = 0;
                        resp[2] = "Username wrong size.";
                        goto send_resp;
                    }
                    bool notSpaces = true;
                    for(int i=0; i<nms.size(); i++)
                    {
                        if(nms[i] != ' ') notSpaces = false;
                    }
                    if(notSpaces)
                    {
                        delete srch;
                        resp[0] = "reg";
                        resp[1] = 0;
                        resp[2] = "Username has wrong characters.";
                        goto send_resp;
                    }
                    printf("Could not find name in tree.\n");
                    //push to database

                    srch->id = forum.usersSize +  1;
                    forum.usersSize++;
                    srch->email = req[3].asString();
                    std::string psalt = std::to_string(srch->id).append(srch->email); //Procedure for changing email will cause password hash to change too.
                    std::string tohash = psalt + srch->username + psalt + req[2].asString();
                    printf("Testing [%s]\n",tohash.c_str());
                    SHA512((unsigned char*)tohash.data(),tohash.length(),(unsigned char*)srch->password.pn);
                    printf("Account: [%s]\n[%d]\n[%s]\n",srch->username.c_str(),srch->id,srch->password.GetHex().c_str());
                    forum.users.insert((Forum_Acct_S*)srch);
                    forum.users_name.insert(srch);
                    MainDBPool->user_create->clearParameters();
                    MainDBPool->user_create->setString(1,srch->username);
                    MainDBPool->user_create->setString(2,srch->email);
                    std::streambuf *pass;
                    std::stringstream blob;
                    pass = blob.rdbuf();
                    pass->sputn((char*)srch->password.pn,64);
                    MainDBPool->user_create->setBlob(3,&blob);
                    MainDBPool->user_create->execute();
                    MainDBPool->user_id_get->clearParameters();
                    MainDBPool->user_id_get->setString(1,srch->username);
                    sql::ResultSet * res = MainDBPool->user_id_get->executeQuery();
                    while(res->next())
                    {

                        srch->id = res->getInt("id");
                        printf("Got id %d\n",srch->id);
                        forum.usersSize = srch->id;
                    }
                    delete res;

                    /* MainDBPool->fuser_create->clearParameters();
                     MainDBPool->fuser_create->setUInt64(1,srch->id);
                     MainDBPool->fuser_create->execute();
                     */
                    resp[0] = "reg";
                    resp[1] = 1;
                    resp[2] = "Successful";
                    goto send_resp;
                }
            }
            else if(req[0].asString() == "cpost1")
            {
                if(subpoint(con->metadata,"id").asInt() == 0)
                {
                    resp[0] = "error";
                    resp[1] = "You must be logged in to post";
                    goto send_resp;
                }
                int forum_id = req[1].asInt();
                Forum_Board * theboard =  forum.getBoardById(forum_id);
                if(theboard == NULL)
                {
                    resp[0] = "error";
                    resp[1] = "That board does not exist";
                    goto send_resp;
                }
                Forum_Thread * newThread = new Forum_Thread();
                newThread->parent = theboard;
                newThread->id = forum.threadCount + 1;
                forum.thread_posts.insert((Forum_Thread_S*) newThread);
                Forum_Post * nPost = new Forum_Post();
                nPost->posted = (unsigned long)time(NULL);
                nPost->id = forum.postCount + 1;
                nPost->body = req[3].asString();
                nPost->subject = req[2].asString();
                nPost->parent = newThread;
                Forum_Acct_S * srch = new Forum_Acct_S(subpoint(con->metadata,"id").asUInt64());

                nPost->poster = (Forum_Acct*)forum.users.find(srch)->key;
                nPost->poster->posts++;

                delete srch;

                theboard->insertThread(newThread);
                newThread->insertPost(nPost);
                {
                    MainDBPool->ftopic_create->clearParameters();
                    MainDBPool->ftopic_create->setInt(1,forum_id);
                    MainDBPool->ftopic_create->setInt64(2,nPost->poster->id);
                    MainDBPool->ftopic_create->setString(3,req[2].asString());
                    MainDBPool->ftopic_create->setString(4,req[3].asString());
                    MainDBPool->ftopic_create->setInt64(5,nPost->posted);
                    MainDBPool->ftopic_create->execute();
                }
                resp[0] = "cpost1";
                resp[1] = (Json::UInt64)newThread->id;
                printf("Cpost1 send\n");
                goto send_resp;
            }
            else if(req[0].asString() == "cpost2")
            {
                if(subpoint(con->metadata,"id").asInt() == 0)
                {
                    resp[0] = "error";
                    resp[1] = "You must be logged in to post";
                    goto send_resp;
                }
                int thread_id = req[1].asInt();
                Forum_Thread_S * srch = new Forum_Thread_S();
                srch->id = thread_id;
                Forum_Thread * the_thread = (Forum_Thread*)(forum.thread_posts.find(srch)->key);
                delete srch;
                if(the_thread == 0)
                {

                    resp[0] = "error";
                    resp[1] = "Thread does not exist (submitted id must be thread\'s id)";
                    goto send_resp;
                }

                Forum_Post * newPost = new Forum_Post();
                newPost->id = ++(forum.postCount);
                newPost->subject = req[2u].asString();
                newPost->body = req[3u].asString();
                newPost->posted = (unsigned long)(time(NULL));
                Forum_Acct_S * srch2 = new Forum_Acct_S(subpoint(con->metadata,"id").asUInt64());

                newPost->poster = (Forum_Acct*)(forum.users.find(srch2)->key);
                newPost->poster->posts++;
                printf("Posted by %s\n",newPost->poster->username.c_str());
                {
                    MainDBPool->fpost_create->clearParameters();
                    MainDBPool->fpost_create->setInt(1,the_thread->parent->id);
                    MainDBPool->fpost_create->setInt(2,newPost->poster->id);
                    MainDBPool->fpost_create->setInt(3,thread_id);
                    MainDBPool->fpost_create->setString(4,newPost->subject);
                    MainDBPool->fpost_create->setString(5,newPost->body);
                    MainDBPool->fpost_create->setInt64(6,newPost->posted);
                    MainDBPool->fpost_create->execute();
                }
                delete srch2;
                the_thread->insertPost(newPost);

                resp[0] = "cpost2";
                resp[1] = (Json::UInt64)the_thread->id;
                goto send_resp;

            }
            else if(req[0].asString() == "prof")   //User requested to view their profile.
            {
                //TODO: Permissions to support "multiple forums on the same server"
                resp[0] = "prof";
                resp[1] = req[1]; //We will have support for both usernames, and id searches
                Forum_Acct * babe = NULL;
                if(req[1].isInt())
                {

                    if(req[1].asUInt64() > 0) {}
                    Forum_Acct_S * srch = new Forum_Acct_S(req[1].asUInt64());
                    babe = (Forum_Acct*)(forum.users.find(srch)->key);
                    delete srch;
                }
                Json::Value subvec;
                if(babe == 0)
                {
                    subvec[0] = "No User Found";
                    subvec[1] = -1;
                }
                else
                {
                    subvec[0] = babe->username;
                    subvec[1] = (Json::UInt64)(babe->id);
                }
                resp[2] = subvec;
                goto send_resp;

            }
            else
            {
                requests--;
            }






send_resp:
            tts =(tts + (double)(time(NULL) - start))/2.0;
            //if(requests %100 == 1)
            printf("Requests: %lu, tts: %.04f\n",requests,tts);
            if(!resp.empty())
            {
                server::connection_ptr con = m_server.get_con_from_hdl(a.hdl);

                con->send(resp.toStyledString().c_str());
            }
        }
        catch(std::exception& e)
        {
            printf("Caught exception\n%s\n",e.what());
            //TODO: Check for certain errors like:
            //Database connection errors, we should be saving everythingto the
            //memory map of the forum, so when the DB is reestablished we can
            //update it with the differences.
        }
    }
}
void ybb_handler::setDB(DatabasePool * newDB)
{
    this->dbpool = newDB;
}
// {"type":"participants","value":[<participant>,...]}


// {"type":"msg","sender":"<sender>","value":"<msg>" }

