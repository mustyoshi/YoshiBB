#include "Groups.h"

UserGroup::UserGroup(int id,unsigned char def)
{
    this->id = id;
    this->defaultPerm = def;
}
void UserGroup::AddRule(PermRule newRule)
{
    Forum_Board * board = forum.getBoardById(newRule.f_id);
    //Add the rule to our list.
    this->prules.push_back(newRule);
    //Increment the scope
    newRule.scope++;
    //Loop through the board's children and add it with the updaeted scope.
    for(auto i=board->children.begin();i != board->children.end();i++){
        newRule.f_id = (*i)->id;
        //Set the new ID, then recursively call the function.
        this->AddRule(newRule);
    }
}
