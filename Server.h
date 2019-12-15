//
// Created by cis505 on 11/19/19.
//

#ifndef FINAL_SERVER_H
#define FINAL_SERVER_H

#include "BigTableClient.h"

class Server {

public:
    static std::vector<int> frontendServers;
    static BigTableClient bigTableClient;
    static bool run;
    static int portNum;
    void startServer(char* portNumber);
    Server() {
        frontendServers = {5051, 5052, 5053};
    }
    static std::vector<std::string> pingFrontendServers();
};


#endif //FINAL_SERVER_H
