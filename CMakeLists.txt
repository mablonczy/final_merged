cmake_minimum_required(VERSION 3.15)
project(final)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS -pthread)

include_directories(.)
include_directories(resources)

add_executable(final
        resources/admin.html
        resources/drive.html
        resources/home.html
        resources/login.html
        resources/mail.html
        AdminClient.h
        BigTableClient.h
        Checkpoint.h
        communication.h
        config.txt
        DriveClient.h
        email.h
        EmailClient.h
        GetHandler.cpp
        GetHandler.h
        HTMLtoString.cpp
        HTMLtoString.h
        json.hpp
        main.cpp
        Payload.h
        PostHandler.cpp
        PostHandler.h
        Server.cpp
        Server.h
        ThreadHandler.cpp
        ThreadHandler.h
        userHandler.h
        Writer.cpp
        Writer.h)
