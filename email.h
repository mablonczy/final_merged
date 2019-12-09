//
// Created by Kanika Prasad Nadkarni on 11/19/19.
//

#ifndef FINAL_EMAIL_H
#define FINAL_EMAIL_H

class email {
public:
    std::string sender;
    std::string timestamp;
    std::vector <std::string>recepients;
    std::string content;
    std::string subject;

    email()= default;

    email(const std::string &sender, const std::string &timestamp, const std::vector<std::string> &recepients,
          const std::string &content, const std::string &subject) : sender(sender), timestamp(timestamp),
                                                                    recepients(recepients), content(content),
                                                                    subject(subject) {}

    std::string hashValueString;
    //std::string username;
};

#endif