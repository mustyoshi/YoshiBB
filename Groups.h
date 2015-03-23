#ifndef YoshiBB_Groups
#define YoshiBB_Groups

/*
I was thinking of some sort of initial rule parsing
Where all the rules for the group are fed into it...
And higher level rules are overridden by more specific rules.
Rules would affect every child board of the board they are on.
Given the structure:
A
-x
-y
--c
--d
-z

A rule allowing you to moderate A would let you moderate x,y,c,d,z
But a rule allowing you to see/post in y would mean you could not moderate y.
Rules with the same "scope" will always be combined to give you the most permissions(Unless it is a rule for denying permissions)
*/
struct PermRule
{
    unsigned int f_id;
    char scope; //Distance from the board it was for (starts at 0)
    bool allow; //If this is a negation rule or not
    unsigned char perm; //Which permission this is.

};
namespace Perms
{
const unsigned char VIEW = 1 << 1;
const unsigned char POST = 1 << 2;
const unsigned char MODERATE = 1 << 3;
const unsigned char ADMIN = 1 << 7;
};

class UserGroup
{
private:
    //The binary tree will return defaultPerm if it does not find an extry in the tree. For the majority of users it will return the
    //default perm.
    std::vector<unsigned char> perms; //TODO: Make a binary tree
public:
    std::vector<PermRule> prules; //This will be filled up then compiled into an unsigned char.
    unsigned int id;
    unsigned char defaultPerm;
    UserGroup(int id,unsigned char def);
    void AddRule(PermRule newRule);
    void CompileRules(); //It will be an expensive operation that goes through and checks all the scopes.
    unsigned char GetPerm(unsigned int f_id);
};
#endif // YoshiBB_Groups
