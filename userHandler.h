// user and session operations live here

#include <string>
//#include "BigTableClient.h"

std::string accountPref = "acc_" ;
std::string sessionPref = "cookie_" ;

bool usr_valid(BigTableClient big_table, std::string user, std::string pass){
    user = accountPref + user ;

    std::string cur_pass;
	int retGet = big_table.get(user, user, cur_pass);

    if (retGet < 0) return false ; // no such user
    if (cur_pass != pass) return false ; // bad password

    return true;
}

bool store_usr_pass(BigTableClient big_table, std::string user, std::string pass, bool change){
    user = accountPref + user ;

    if (!change) {
        // check that user doesn't exist first
        std::string old_pass;
        int retGet = big_table.get(user, user, old_pass);
        if (retGet > 0) return false ; // such user exists
    } else {
        // check that user exists first
        std::string old_pass;
        int retGet = big_table.get(user, user, old_pass);
        if (retGet < 0) return false ; // such user does not exists
    }

    // check for password being none-empty
    if (pass.length() == 0) return false ;

    int retPut = big_table.put(user, user, pass);
    if (retPut < 0) return false;
    return true ;
}

bool store_usr_cookie(BigTableClient big_table, std::string user, std::string cookie){
    cookie = sessionPref + cookie ;
    int retPut = big_table.put(cookie, cookie, user);
    if (retPut < 0) return false;
    return true ;
}

std::string get_usr_from_cookie(BigTableClient big_table, std::string cookie){
    cookie = sessionPref + cookie ;
    std::string user;
	int retGet = big_table.get(cookie, cookie, user);

    return user; // = EMPTY_DATA
}

bool del_usr_cookie(BigTableClient big_table, std::string cookie){
    cookie = sessionPref + cookie ;
    int retDel = big_table.del(cookie, cookie);
    if (retDel < 0) return false;

    return true ;
}
