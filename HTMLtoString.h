//
// Created by cis505 on 11/17/19.
//

#ifndef FINAL_HTMLTOSTRING_H
#define FINAL_HTMLTOSTRING_H

#include <map>

class HTMLtoString {
    std::map<std::string, std::string> pages = {
            {"/web","resources/login.html"},
            {"/web/home","resources/home.html"},
            {"/web/mail","resources/mail.html"},
            {"/web/drive","resources/drive.html"},
            {"/web/admin", "resources/admin.html"}
    };
    static std::string readHTML(const std::string& path);
public:
    std::string goodRequest(const std::string& page, bool allocateCookie);
    std::string badRequest();
};


#endif //FINAL_HTMLTOSTRING_H
