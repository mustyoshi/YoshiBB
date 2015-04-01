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
void UserGroup::AddRule(PermRule * newRule)
{
    Forum_Board * board = forum.getBoardById(newRule->f_id);
    printf("New board (%d,%d @ %d)\n",newRule->f_id,newRule->perm,newRule->scope);;
    //Add the rule to our list.
    this->prules.push_back((newRule));
    //Increment the scope

    //Loop through the board's children and add it with the updaeted scope.
    for(auto i=board->children.begin(); i != board->children.end(); i++)
    {
        PermRule * nRule = new PermRule();
        nRule->f_id =(*i)->id;
        printf("Adding to babies\n");
        nRule->perm = newRule->perm;
        nRule->scope = newRule->scope+1;
        //Set the new ID, then recursively call the function.
        this->AddRule(nRule);
    }
}
void UserGroup::CompileRules()
{
    //Until we have no more rules to parse.
    while(this->prules.size() > 0)
    {
        std::list<PermRule*> workingList;
        unsigned int f_id = this->prules.front()->f_id;
        pof:
        for(std::list<PermRule*>::iterator i = this->prules.begin(); i != this->prules.end(); ++i)
        {
            if((*i)->f_id == f_id)
            {
                workingList.push_front((*i));
                this->prules.erase(i);
                goto pof;
            }
        }
        unsigned char rule = 0;
        int *dS = new int[4];
        int *aS =   new  int[4];


        for(int i=0; i<4; i++)
        {
            dS[i] = aS[i] = 1000;

        }
        for(auto i=workingList.begin(); i != workingList.end(); ++i)
        {

            PermRule * tt = (*i);
            for(int p = 0; p<= 6; p+=2)
            {
                //Some bitwise flags based on if the scope is less than the opposite flag's scope.
                if((tt->perm >> (p+1)) & 1)
                {

                    if((tt->perm & ( 1<<p)) >0 && tt->scope <= dS[p/4])
                    {
                        aS[p/4] = tt->scope;
                        rule = rule | (3 << p);
                    }
                }
                else
                {

                    if((tt->perm & (1 << p)) >0  && tt->scope <= aS[p/4])
                    {
                        dS[p/4] = tt->scope;
                        rule = rule ^ !(1 << p);
                        if((rule & (1 << (p+1))) == 0)
                        {
                            rule = rule  ^ !(1 << (p+1));

                        }

                    }
                }

            }
        }
        CompRule *  nRule = new CompRule();
        nRule->f_id = f_id;
        nRule->perm = rule;
        if(rule != this->defaultPerm) //If it is the default we dont need to save
        {
            this->perms.push_back(nRule);
        }
    }
}
unsigned char UserGroup::GetPerm(unsigned int f_id)
{
    for(auto i=this->perms.begin(); i!=this->perms.end(); ++i)
    {
        if((*i)->f_id == f_id)
        {
            return (*i)->perm;
        }
    }
    return this->defaultPerm;

}
