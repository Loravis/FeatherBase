#include "statuscodes.h"

/**Converts a status code integer value into a string. */
std::string getStatusCodeString(int &statuscode) {
    if (statuscode == SUCCESS) {
        return "SUCCESS";
    } else if (statuscode == NOTFOUND) {
        return "NOTFOUND";
    } else if (statuscode == EXISTS) {
        return "EXISTS";
    } else if (statuscode == INVALID) {
        return "INVALID";
    } else if (statuscode == FAILURE) {
        return "FAILURE";
    } else if (statuscode == SYNTAXERROR) {
        return "SYNTAXERROR";
    }
    else {
        throw("Invalid status code!");
    }
}