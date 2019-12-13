//
// Created by cis505 on 11/19/19.
//

#ifndef FINAL_POSTHANDLER_H
#define FINAL_POSTHANDLER_H

#include <string>
#include <vector>

class PostHandler {

    std::string path;
    std::string registerPath = "/web/register";
    std::string loginPath = "/web/login";
    std::string logOutPath = "/web/logout";
    std::string passwordPath = "/web/password";
    std::string mailPath = "/web/mail";
    std::string mailDeletePath = "/web/mail/delete";
    std::string mailSendPath = "/web/mail/send";
    std::string drivePath = "/web/drive";
    std::string driveDeletePath = "/web/drive/delete";
    std::string driveRenamePath = "/web/drive/rename";
    std::string driveMovePath = "/web/drive/move";
    std::string driveNewFolderPath = "/web/drive/newfolder";
    std::string driveUploadPath = "/web/drive/upload";
    std::string driveDownloadPath = "/web/drive/download";
    std::string adminPath = "/web/admin";
    std::string adminTogglePath = "/web/admin/toggle";
    std::string adminContentPath = "/web/admin/content";

public:
    std::string handleRequest(std::string& request);

    static std::string findData(std::string &basicString);

    static std::string findCookie(std::string request);

    static std::vector<std::string> parseRecipients(std::string basicString);

    static std::string sendOk(const std::string& basicString);

    std::string sendFail();
};


#endif //FINAL_POSTHANDLER_H
