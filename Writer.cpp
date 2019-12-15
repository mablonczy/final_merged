//
// Created by cis505 on 11/17/19.
//


#include <zconf.h>
#include <csignal>
#include "Writer.h"

bool Writer::do_write(int fd, const char *buf, int len) {
    signal(SIGPIPE, SIG_IGN);
    int sent = 0;
    while (sent < len) {
        int n = write(fd, &buf[sent], len-sent);
        if (n <= 0) {
            return false;
        }
        sent += n;
    }
    return true;
}
