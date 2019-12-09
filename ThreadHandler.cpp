//
// Created by cis505 on 11/17/19.
//

#include <iostream>
#include <zconf.h>
#include "ThreadHandler.h"
#include "GetHandler.h"
#include "PostHandler.h"
#include "Writer.h"

void* ThreadHandler::worker(void *args) {
    pthread_detach(pthread_self());
    int fd = *(int*) args;
    //10 MB buffer
    char* buf = new char[10000000];
    int rcvd = do_read(fd, buf, 10000000);
    std::string request(buf, rcvd);
    delete[] buf;
    std::cerr << request << std::endl;
    std::string request_type = request.substr(0, request.find_first_of(' '));
    if (request_type == "GET") {
        //get response string
        //send response string to fd
        GetHandler getHandler;
        std::string response = getHandler.handleRequest(request);
        Writer::do_write(fd, response.c_str(), response.length());
    } else if (request_type == "POST") {
        PostHandler postHandler;
        std::string response = postHandler.handleRequest(request);
        Writer::do_write(fd, response.c_str(), response.length());
    } else if (request_type == "HEAD") {
        // do I even need this?
    } else {
        //invalid - request not implemented
    }
    close(fd);
    std::cerr << "[" << fd << "] Connection closed" << std::endl;
    return nullptr;
}

int ThreadHandler::do_read(int fd, char* buf, int len) {
    int rcvd = 0;
    //will loop until there is nothing left to read
    while (true) {
        int n = read(fd, &buf[rcvd], len - rcvd);
        rcvd += n;
        if (n <= 0 || find_end(buf, len) != -1) {
            return rcvd;
        }
    }
}

int ThreadHandler::find_end(char* buf, int len) {
    for (int i = 0; i < len - 1; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return i + 1;
        }
    }
    return -1;
}
