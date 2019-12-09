//
// Created by cis505 on 11/17/19.
//

#ifndef FINAL_GETHANDLER_H
#define FINAL_GETHANDLER_H


#include <string>

class GetHandler {

    std::string path;
    std::string loginPath = "/web";
    std::string homePath = "/web/home";
    std::string emailPath = "/web/mail";
    std::string drivePath = "/web/drive";
    std::string adminPath = "/web/admin";
public:
    std::string handleRequest(std::string& request);
    static bool hasCookie(std::string& request);
};


#endif //FINAL_GETHANDLER_H
