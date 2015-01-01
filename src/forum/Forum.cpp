#include "Forum.h"
template <class T>
void DLL_Node<T>::insertBefore(DLL_Node<T> & pop)
{
    this->next = &pop;
    this->prev = pop.prev;
    pop.prev->next = this;
    pop.prev = this;
}
template <class T>
void DLL_Node<T>::remove()
{
    this->next->prev = this->prev;
    this->prev->next = this->next;
}
Forum_Acct::Forum_Acct()
{
    username = "Guest";
    email = "No Email";
}
void Forum_Thread::insertPost(Forum_Post * reply)
{
    bool first = true;
    forum.postCount++;
    printf("Forum count is %d\n",forum.postCount);
    Post_Node * newReply = new Post_Node();
    newReply->Key = reply;
    this->parent->posts++;
    if(firstpost == NULL)
    {
        firstpost = newReply;
        creator = reply->poster;
        this->parent->topics++;
        forum.threadCount++;
    }
    else //If it's the first post we don't want to increment the reply counter.
    {
        first = false;
        replies++;


    }
    if(lastpost != NULL)
        lastpost->next = newReply;
    lastpost = newReply;
    if(replies%20) //1%20 = 1 == true, so inserting a new post will create the first page link automatically.
    {
        pages.push_back(newReply);
    }
    if(!first) parent->bumpThread(myself);
}

void Forum_Board::bumpThread(Thread_Node * forum_thread)
{
    if(forum_thread == NULL)
    {
        printf("What happenedn\n");
        return;
    }
    if(forum_thread->prev != NULL)
    {
        printf("The previous fixed\n");
        forum_thread->prev->next = forum_thread->next;

    }
    if(forum_thread->next != NULL)
    {
        printf("The next is fixed\n");
        forum_thread->next->prev = forum_thread->prev;

    }
    if(laststicky == NULL)
    {
        printf("We inserted\n");
        forum_thread->next = threads;
        threads->prev  = forum_thread;
        threads = forum_thread;
        forum_thread->prev = NULL;
    }
    else
    {
        if(laststicky->next != NULL)
            laststicky->next->prev = forum_thread;
        laststicky->next = forum_thread;
        forum_thread->prev = laststicky;
    }

}
void Forum_Board::insertThread(Forum_Thread * newThread)
{


    Thread_Node * newNode = new Thread_Node();
    newThread->myself = newNode;
    newThread->parent = this;
    newNode->Key = newThread;
    if(threads == NULL)
    {
        printf("First thread\n");
        threads = newNode;
        newNode->next = NULL;
        newNode->prev = NULL;
    }
    else if(threads != newNode)
    {
        printf("Hmm?\n");
        threads->prev = newNode;
        newNode->next = threads;
        threads = newNode;
    }
    else
    {
        printf("Herp derp\n");
    }
}
void Forum::addBoard(Forum_Board * newBoard)
{
    boards.push_back(newBoard);
}
void Forum::addTLBoard(Forum_Board * newBoard)
{
    topboards.push_back(newBoard);
}
Forum_Board * Forum::getBoardById(unsigned int check_id)
{
    Forum_Board * toRet = NULL;
    for(int i=0; i<this->boards.size() && (toRet == NULL); i++)
    {
        if(this->boards[i]->id == check_id)
            toRet = this->boards[i];

    }
    for(int i=0; i<this->topboards.size() && (toRet == NULL); i++)
    {
        if(this->topboards[i]->id == check_id)
            toRet = this->topboards[i];

    }
    return toRet;
}

template<>
bool splay_tree<Forum_Acct_S*>::comp(Forum_Acct_S *a,Forum_Acct_S *b)
{
    return a->id < b->id;

}
template<>
bool splay_tree<Forum_Thread_S*>::comp(Forum_Thread_S *a,Forum_Thread_S *b)
{
    return a->id < b->id;

}
using namespace std;

template<>
bool splay_tree<Forum_Acct*>::comp(Forum_Acct *a,Forum_Acct *b)
{
    std::string an;
    std::string bn;
    for(int i=0; i<a->username.length(); i++)
    {
        char t = a->username.at(i);
        if(t >= 'A' && t <= 'Z')
            t = (t+32);

        an.push_back(t);
    }
    for(int i=0; i<b->username.length(); i++)
    {
        char t = b->username.at(i);
        if(t >= 'A' && t <= 'Z')
            t = (t+32);

        bn.push_back(t);
    }
    int d = strcmp(an.c_str(),bn.c_str());
    printf("Comparing %s to %s = %d\n",an.c_str(),bn.c_str(),d);
    /* if(al < bl) return false;
     if(al > bl) return true;

     for(int i=0; i<al; i++)
     {
         if(
             ((an[i] - 65) % 32) + 65
             <=
             ((bn[i] - 65) % 32) + 65
         ) return false;

     }
     */
    return std::lexicographical_compare(an.begin(),an.end(),bn.begin(),bn.end());
    //return a->username.compare((b->username)) < 0;


}
bool fa_ses_comp(Forum_Acct* a, Forum_Acct * b)
{

    return (a->session.compare((b->session)))>=0;
}
