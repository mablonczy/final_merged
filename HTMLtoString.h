//
// Created by cis505 on 11/17/19.
//

#ifndef FINAL_HTMLTOSTRING_H
#define FINAL_HTMLTOSTRING_H

#include <map>

class HTMLtoString {
    std::map<std::string, std::string> pages = {
            {"/web","/Users/murri/Desktop/FrontEnd/final_merged/resources/login.html"},
            {"/web/home","/Users/murri/Desktop/FrontEnd/final_merged/resources/home.html"},
            {"/web/mail","/Users/murri/Desktop/FrontEnd/final_merged/resources/mail.html"},
            {"/web/drive","/Users/murri/Desktop/FrontEnd/final_merged/resources/drive.html"},
            {"/web/admin", "/Users/murri/Desktop/FrontEnd/final_merged/resources/admin.html"}
    };
    static std::string readHTML(const std::string& path);
public:
    std::string goodRequest(const std::string& page, bool allocateCookie);
    static std::string badRequest();
    static std::string serversDown();
};


#endif //FINAL_HTMLTOSTRING_H
