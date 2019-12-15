//
// Created by cis505 on 11/17/19.
//

#ifndef FINAL_HTMLTOSTRING_H
#define FINAL_HTMLTOSTRING_H

#include <map>

class HTMLtoString {
    std::map<std::string, std::string> pages = {
            {"/web","/home/cis505/final/resources/login.html"},
            {"/web/home","/home/cis505/final/resources/home.html"},
            {"/web/mail","/home/cis505/final/resources/mail.html"},
            {"/web/drive","/home/cis505/final/resources/drive.html"},
            {"/web/admin", "/home/cis505/final/resources/admin.html"}
    };
    static std::string readHTML(const std::string& path);
public:
    std::string goodRequest(const std::string& page, bool allocateCookie);
    static std::string badRequest();
    static std::string serversDown();
};


#endif //FINAL_HTMLTOSTRING_H
