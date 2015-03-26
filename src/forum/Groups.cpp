#include "Groups.h"
#include "Forum.h"
//Creating and parsing the rules is relatively expensive
//But I think it's ok, since it's only going to happen
//During teh initial startup. And when a new rule is created.
//Which may or may not require the entire process to be redone.
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
    for(auto i=board->children.begin(); i != board->children.end(); i++)
    {
        newRule.f_id = (*i)->id;
        //Set the new ID, then recursively call the function.
        this->AddRule(newRule);
    }
}
void UserGroup::CompileRules()
{
    //Until we have no more rules to parse.
    while(this->prules.size() > 0)
    {
        std::list<PermRule> workingList;
        workingList.push_front(this->prules.front());
        this->prules.pop_front();
        unsigned int f_id;
        for(auto i=this->prules.begin(); i!= this->prules.end(); i++)
        {
            if((*i).f_id == f_id)
            {
                workingList.push_front((*i));
                this->prules.erase(i);
            }
        }
        unsigned char rule;
        int aS;
        int dS;
        for(int p = 0; p< 8; p++)
        {
            aS = dS = 1000; //Hmm, perhaps this is a good number, will there ever be a 1k deep forum chain?
            for(auto i=workingList.begin(); i != workingList.end(); i++)
            {
                PermRule tt = (*i);
                if(tt.allow)
                {
                    //Some bitwise flags based on if the scope is less than the opposite flag's scope.
                    if(tt.perm & (1<< p) && tt.scope < dS)
                    {
                        aS = tt.scope;
                        rule = rule | (1 << p);
                    }
                }
                else
                {

                    if(tt.perm & (1 << p) && tt.scope < aS)
                    {
                        dS = tt.scope;
                        rule = rule | (0 << p);
                    }
                }
            }
        }
        CompRule nRule;
        nRule.f_id = f_id;
        nRule.perm = rule;
        if(rule != this->defaultPerm) //If it is the default we dont need to save
        {
            this->perms.push_back(nRule);
        }
    }
}
unsigned char UserGroup::GetPerm(unsigned int f_id)
{
    for(auto i=this->prules.begin(); i!=this->prules.end(); i++)
    {
        if((*i).f_id == f_id)
        {
            return (*i).perm;
        }
    }
    return this->defaultPerm;

}
