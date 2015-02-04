#ifndef msty_strtools_h
#define msty_strtools_h
#include <string>
namespace msty
{
std::vector<std::string> explode(std::string input,std::string delimiter);
std::string join(std::vector<std::string> &input,std::string delimiter);
};
inline std::vector<std::string> msty::explode(std::string input,std::string delimiter)
{
    std::vector<std::string> returnvec = std::vector<std::string>(); /* The vector that holds the returned strings */
    int pos = input.find(delimiter); /* Find the first delimiter, std::string.find returns -1 when it isn't found. */
    while(pos != std::string::npos) /* Loop until the delimiter isn't found. */
    {
        returnvec.push_back(input.substr(0,pos)); /* When we find something, push it into the return vector */
        input = input.substr(pos+delimiter.size(),std::string::npos); /* discard the portion of the string we've passed through */
        pos = input.find(delimiter); /* Find the next delimiter */
    };
    returnvec.push_back(input); /* push back the remaining string */
    return returnvec; /* return the vector */
}
inline std::string msty::join(std::vector<std::string> & input,std::string delimiter)
{
    std::string returnstr = std::string(); /* This is the string object we return */
    for(int i=0,e=input.size(); i<e; i++) /* Iterate through the vector */
    {
        for(int c=0; c<input[i].size(); c++) /* For each character, push it back (or else we would lose \x00(null terminator) */
        {
            returnstr.push_back(input[i][c]);
        }
        if(i+1 != e) /* if this isn't the last thing in the vector, push the delimiter, character by character */
            for(int c=0; c<delimiter.size(); c++)
            {
                returnstr.push_back(delimiter[c]);
            }
    }
    return returnstr;
}
#endif
