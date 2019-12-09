//
// Created by cis505 on 11/17/19.
//


#include <string>
#include "GetHandler.h"
#include "HTMLtoString.h"

std::string GetHandler::handleRequest(std::string& request) {
    //eliminate GET
    request = request.substr(request.find_first_of('/'));
    path = request.substr(0,request.find_first_of(' '));
    HTMLtoString htmlToString;
    if (path == loginPath) {
        bool allocateCookie = false;
        if (!hasCookie(request)) {
            allocateCookie = true;
        }
        return htmlToString.goodRequest(loginPath, allocateCookie);
    } else if (path == homePath) {
        return htmlToString.goodRequest(homePath, false);
    } else if (path == emailPath) {
        return htmlToString.goodRequest(emailPath, false);
    } else if (path == drivePath) {
        return htmlToString.goodRequest(drivePath, false);
    } else if (path == adminPath) {
        return  htmlToString.goodRequest(adminPath, false);
    } else {
        return htmlToString.badRequest();
    }
}

bool GetHandler::hasCookie(std::string& request) {
    std::string lineEnd = "\r\n";
    int pos = 0;
    std::string headerLine;
    while ((pos = request.find(lineEnd)) != std::string::npos) {
        headerLine = request.substr(0, pos);
        if (headerLine.substr(0, headerLine.find_first_of(':')) == "Cookie") {
            return true;
        }
        request.erase(0, pos + lineEnd.length());
    }
    return false;
}
