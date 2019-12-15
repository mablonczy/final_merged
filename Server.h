//
// Created by cis505 on 11/19/19.
//

#ifndef FINAL_SERVER_H
#define FINAL_SERVER_H


#include "BigTableClient.h"

class Server {

public:
    static BigTableClient bigTableClient;
    static bool run;
    void startServer(char* portNumber);
};


#endif //FINAL_SERVER_H
