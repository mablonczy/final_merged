//
// Created by cis505 on 11/17/19.
//

#ifndef FINAL_THREADHANDLER_H
#define FINAL_THREADHANDLER_H

class ThreadHandler {
public:
    static void* worker(void* args);
    static int do_read(int fd, char* buf, int len);
    static int find_end(char* buf, int len);
};


#endif //FINAL_THREADHANDLER_H
