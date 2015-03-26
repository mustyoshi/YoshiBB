#include <functional>

#ifndef SPLAY_TREE
#define SPLAY_TREE
#include <vector>
#include <string>
#include "uint256.h"
#include <strings.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <functional>
#include "Groups.h"
class Forum;
extern Forum forum;

template< class T>
class splay_tree
{
private:

    bool comp(T a, T b);
    unsigned long p_size;

    struct node
    {
        node *left, *right;
        node *parent;
        T key;
        node( const T& init = T( ) ) : left( 0 ), right( 0 ), parent( 0 ), key( init ) { }
    } *root,*nullNode = new node((T)0);

    void left_rotate( node *x )
    {
        node *y = x->right;
        x->right = y->left;
        if( y->left ) y->left->parent = x;
        y->parent = x->parent;
        if( !x->parent ) root = y;
        else if( x == x->parent->left ) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void right_rotate( node *x )
    {
        node *y = x->left;
        x->left = y->right;
        if( y->right ) y->right->parent = x;
        y->parent = x->parent;
        if( !x->parent ) root = y;
        else if( x == x->parent->left ) x->parent->left = y;
        else x->parent->right = y;
        y->right = x;
        x->parent = y;
    }

    void splay( node *x )
    {
        while( x->parent )
        {
            if( !x->parent->parent )
            {
                if( x->parent->left == x ) right_rotate( x->parent );
                else left_rotate( x->parent );
            }
            else if( x->parent->left == x && x->parent->parent->left == x->parent )
            {
                right_rotate( x->parent->parent );
                right_rotate( x->parent );
            }
            else if( x->parent->right == x && x->parent->parent->right == x->parent )
            {
                left_rotate( x->parent->parent );
                left_rotate( x->parent );
            }
            else if( x->parent->left == x && x->parent->parent->right == x->parent )
            {
                right_rotate( x->parent );
                left_rotate( x->parent );
            }
            else
            {
                left_rotate( x->parent );
                right_rotate( x->parent );
            }
        }
    }

    void replace( node *u, node *v )
    {
        if( !u->parent ) root = v;
        else if( u == u->parent->left ) u->parent->left = v;
        else u->parent->right = v;
        if( v ) v->parent = u->parent;
    }

    node* subtree_minimum( node *u )
    {
        while( u->left ) u = u->left;
        return u;
    }

    node* subtree_maximum( node *u )
    {
        while( u->right ) u = u->right;
        return u;
    }
public:
    splay_tree( ) : root( 0 ), p_size( 0 ) { }

    void insert( const T &key )
    {
        node *z = root;
        node *p = 0;

        while( z )
        {
            p = z;
            if( comp( z->key, key ) ) z = z->right;
            else z = z->left;
        }

        z = new node( key );
        z->parent = p;

        if( !p ) root = z;
        else if( comp( p->key, z->key ) ) p->right = z;
        else p->left = z;

        splay( z );
        p_size++;
    }

    node* find( const T &key )
    {
        node *z = root;
        while( z )
        {
            if( comp( z->key, key ) ) z = z->right;
            else if( comp( key, z->key ) ) z = z->left;
            else
            {
                splay (z); //I do want it to splay
                return z;
            }
        }
        return nullNode;
    }

    void erase( const T &key )
    {
        node *z = find( key );
        if( !z ) return;

        splay( z );

        if( !z->left ) replace( z, z->right );
        else if( !z->right ) replace( z, z->left );
        else
        {
            node *y = subtree_minimum( z->right );
            if( y->parent != z )
            {
                replace( y, y->right );
                y->right = z->right;
                y->right->parent = y;
            }
            replace( z, y );
            y->left = z->left;
            y->left->parent = y;
        }

        delete z;
        p_size--;
    }

    const T& minimum( )
    {
        return subtree_minimum( root )->key;
    }
    const T& maximum( )
    {
        return subtree_maximum( root )->key;
    }

    bool empty( ) const
    {
        return root == 0;
    }
    unsigned long size( ) const
    {
        return p_size;
    }
};


class Forum_Post;
class Forum_Board;
class Forum_Thread;

//Ay o
class Forum_Acct_S
{

public:
    unsigned long id;
    Forum_Acct_S() {id=0;};
    Forum_Acct_S(unsigned long d)
    {
        id = d;
    };

};
class Forum_Acct : public Forum_Acct_S
{
public:
    unsigned long posts;
    Forum_Post * lastpost;
    std::string signature;
    std::string username;
    std::string session;
    unsigned long lastactive;
    std::list<unsigned char> boardPerms; //TODO: Better data structure
    std::string email;
    uint512 password;
    unsigned long last_attempt;
    Forum_Acct();
};

class Forum_Post
{
public:
    unsigned long id;
    Forum_Thread * parent;
    Forum_Acct * poster;
    std::string subject;
    std::string body;
    unsigned long posted;

};
template <class T>
class SLL_Node
{
public:
    T Key;
    SLL_Node<T> * next;
};
typedef SLL_Node<Forum_Post*> Post_Node;


template <class T>
class DLL_Node
{
public:
    T Key;
    DLL_Node<T> * prev;
    DLL_Node<T> * next;
    void insertBefore(DLL_Node<T> & pop);

    void remove();

};
typedef DLL_Node<Forum_Thread*> Thread_Node;

class Forum_Thread_S{
public:
    unsigned long id;
    Forum_Thread_S(){};
    Forum_Thread_S(unsigned long nid){id = nid;};
};
class Forum_Thread : public Forum_Thread_S
{

public:
    Forum_Board * parent;
    Thread_Node * myself; //Keep a pointer to myself.
    Forum_Acct * creator;
    Post_Node *firstpost;
    Post_Node *lastpost;
    std::vector<Post_Node*> pages;
    void insertPost(Forum_Post * reply);
    unsigned int replies;
    unsigned int views;
    unsigned char type;
};
class Forum_Board
{
public:
    std::string name;
    std::string description;
    Forum_Board * parent;
    std::vector<Forum_Board*> children;
    unsigned int id;
    Thread_Node * threads;
    Thread_Node * laststicky;
    unsigned int topics;
    unsigned int posts;
    void bumpThread(Thread_Node * forum_thread);
    void insertThread(Forum_Thread* newThread);

};
class Forum
{
public:
    splay_tree<Forum_Acct_S*> users;
    splay_tree<Forum_Acct*> users_name;
    splay_tree<Forum_Thread_S*> thread_posts;
    std::vector<Forum_Board*> boards;
    std::vector<Forum_Board*> topboards; //This one holds the board groups
    void addBoard(Forum_Board * newBoard);
    void addTLBoard(Forum_Board * newBoard);
    Forum_Board*  getBoardById(unsigned int check_id);
    unsigned long postCount;
    unsigned long threadCount;
    unsigned long usersSize;
};

#endif
