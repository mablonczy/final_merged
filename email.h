//
// Created by Kanika Prasad Nadkarni on 11/19/19.
//

#ifndef FINAL_EMAIL_H
#define FINAL_EMAIL_H

class Email {
public:
    std::string sender;
    std::string timestamp;
    std::vector <std::string>recepients;
    std::string content;
    std::string subject;

    Email()= default;

    Email(const std::string &sender, const std::string &timestamp, const std::vector<std::string> &recepients,
          const std::string &content, const std::string &subject) : sender(sender), timestamp(timestamp),
                                                                    recepients(recepients), content(content),
                                                                    subject(subject) {}
    Email& operator=(const Email& rhs) {};
    std::string hashValueString;
    //std::string username;
};

#endif