#ifndef STATUSCODES
#define STATUSCODES
#include <string>

const int SUCCESS = 0;
const int NOTFOUND = 1;
const int EXISTS = 2;
const int INVALID = 3;
const int FAILURE = 4;
const int SYNTAXERROR = 5;

std::string getStatusCodeString(int &statuscode);

#endif