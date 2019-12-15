cmake_minimum_required(VERSION 3.14)
project(final_merged)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(final_merged
        resources/admin.html
        resources/dne.html
        resources/drive.html
        resources/home.html
        resources/login.html
        resources/mail.html
        resources/serversdown.html
        AdminClient.h
        BigTableClient.h
        Checkpoint.h
        CMakeLists.txt
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
