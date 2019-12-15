//
// Created by Kanika Prasad Nadkarni on 11/19/19.
//
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <functional>
#include <queue>

#include "Email.h"
#include "BigTableClient.h"

#include <netinet/in.h>
#include <resolv.h>
#include <netdb.h>

#include<iostream>
using namespace std;

#define N 4096
int MAX_MESSAGE_LENGTH  = 2000;
//std::string CONFIG_PATH = "./config.txt";
//int NUMBER_OF_PRIMARIES = 1;
//bool PRINT_LOGS = false;
inline bool charcomparek(char a, char b)
{
    return(toupper(a) == toupper(b));
}

bool comparek(const string& s1, const string& s2)
{
    return((s1.size() == s2.size()) && equal(s1.begin(), s1.end(), s2.begin(), charcomparek));
}


class EmailClient {

    BigTableClient bigTable;
    std::string ROW_ID = "_email";
    std::string PENNCLOUD_DOMAIN = "penncloud";
    std::string METADATA_COL = "METADATA";
    std::string NEW_LINE = "\n";
    std::queue <Email> queue_emails_for_externalservers;


public:
    void initialize() {
        bigTable.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
    }

    std::string listEmails(std::string usr, std::vector<Email>& listMailbox) {
        int separator_index = usr.find("@");
        std::string recepient_username = usr.substr(0, separator_index);
        std::string rowName = recepient_username + ROW_ID;
        std::cout << "Listing emails for : " <<  rowName << std::endl;

        std::string metadataContent;
        int metadata_return = bigTable.get(rowName, METADATA_COL, metadataContent);
        if (metadata_return == FAILURE || metadataContent.find(EMPTY_DATA) != std::string::npos) metadataContent = "";

        std::cout << "Listing Metadata: " << metadataContent << std::endl;

        std::string line;
        std::istringstream metadataStream(metadataContent);
        while (getline(metadataStream,line)) {
            std::string currentEmail;
            int retval = bigTable.get(rowName,line,currentEmail);
            if (retval == -1) {
                return "BACKEND_DEAD";
            }
            std::istringstream currentMailStream(currentEmail);
            std::string tempLine;
            Email obj;
            while (getline(currentMailStream,tempLine))
            {
                std::cout << "Mailbox line: " << tempLine << std::endl;

                if (tempLine.substr(0, 6) == "From: ") {
                    obj.sender=tempLine.substr(6,std::string::npos);
                }
                else if(tempLine.substr(0, 4) == "To: ") {
                    obj.recepients.push_back(tempLine.substr(4,std::string::npos));
                }
                else if (tempLine.substr(0, 6) == "Date: ") {
                    obj.timestamp=tempLine.substr(6,std::string::npos);
                }
                else if (tempLine.substr(0, 9) == "Subject: ") {
                    obj.subject=tempLine.substr(9,std::string::npos);
                }
                else if (tempLine.substr(0, 17) == "hashValueString: ") {
                    obj.hashValueString=tempLine.substr(17,std::string::npos);
                    std::cout << "DID GET HASH: " << obj.hashValueString << std::endl;
                }
                else {
                    obj.content=obj.content+tempLine+"\n";
                }
            }
            listMailbox.push_back(obj);
        }
        return "SUCCESS";
    }

    std::string deleteEmail(Email email) {

        std::string email_column_id=email.hashValueString;
        std::cout <<  "Hash value to delete " << email_column_id << std::endl;
        std::string temp;
        std::string recepient_email = email.recepients[0];
        int separator_index = recepient_email.find("@");
        std::string recepient_username = recepient_email.substr(0, separator_index);
        std::string recepient_row_id = recepient_username + ROW_ID;
        std::cout <<  "Row of email to delete " << recepient_row_id << std::endl;

        std::string metadataContent;
        int metadata_return = bigTable.get(recepient_row_id, METADATA_COL, metadataContent);
        if (!metadata_return) metadataContent = "";

        std::string line;
        std::istringstream metadataStream(metadataContent);
        while (getline(metadataStream,line)) {
            if(email_column_id != line) {
                temp += line + NEW_LINE;
            }
        }
        std::cout <<  "Metadata after delete " << temp << std::endl;

        bigTable.put(recepient_row_id,METADATA_COL,temp);

        bigTable.del(recepient_row_id,email_column_id);
        return "SUCCESS";
    }

    std::string putEmail(Email email) {
        for(int i=0; i < email.recepients.size(); i++){

            std::string recepient_email = email.recepients[i];
            int separator_index = recepient_email.find("@");

            std::string recepient_username = recepient_email.substr(0, separator_index);
            std::string recepient_domain = recepient_email.substr(separator_index + 1, std::string::npos);

            std::string recepient_row_id = recepient_username + ROW_ID;
            std::cout << "**** Sending email to: " << recepient_username  << " " << recepient_row_id <<  std::endl;
            if (recepient_domain == PENNCLOUD_DOMAIN){
                std::cout << "****  This is a penncloud email " << std::endl;
                int ret = putEmail_in_Penncloud(recepient_row_id, recepient_email, email);
                if (ret != SUCCESS) {
                    return "BACKEND_DEAD";
                }
            }
            else
            {
                std::vector <std::string>recepient_new;
                recepient_new.push_back(email.recepients[i]);
                Email email_with_external_recepient_email(email.sender,email.timestamp,recepient_new, email.content,email.subject);

                queue_emails_for_externalservers.push(email_with_external_recepient_email);
            }
        }
        return "SUCCESS";
    }

    void send_email_to_external_server()
    {
        if(queue_emails_for_externalservers.empty())
            return;
        Email email=queue_emails_for_externalservers.front();

        u_char nsbuf[N];
        char dispbuf[N];
        ns_msg msg;
        ns_rr rr;
        int i, l,msg_len;

        std::string recepient_email = email.recepients[0];
        int separator_index = recepient_email.find("@");

        std::string recepient_username = recepient_email.substr(0, separator_index);
        std::string recepient_domain = recepient_email.substr(separator_index + 1, std::string::npos);
        printf("Domain : %s\n", recepient_domain.c_str());

        // MX RECORD
        int minpriority=1000;
        std::string serverNameTest;

        char serverName[NS_MAXDNAME];
        printf("MX records : \n");//ns_c_any
        l = res_query(recepient_domain.c_str(), ns_c_any, ns_t_mx, nsbuf, sizeof(nsbuf));
        msg_len=l;
        if (l < 0)
        {
            perror(recepient_domain.c_str());
            queue_emails_for_externalservers.pop(); //pop since no mX records of domain found
            return;
        }
        else
        {
#ifdef USE_PQUERY
            /* this will give lots of detailed info on the request and reply */
      res_pquery(&_res, nsbuf, l, stdout);

#else
            /* just grab the MX answer info */
            ns_initparse(nsbuf, l, &msg);
            l = ns_msg_count(msg, ns_s_an);
            for (i = 0; i < l; i++)
            {
                ns_parserr(&msg, ns_s_an, i, &rr);
                ns_sprintrr(&msg, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
                printf ("\t%s\n", dispbuf);
                char exchange[NS_MAXDNAME];
                //char exchange[N];
                const u_char *rdata = ns_rr_rdata(rr);

                const uint16_t pri = ns_get16(rdata);
                int len = dn_expand(nsbuf, nsbuf + msg_len, rdata + 2, exchange, sizeof(exchange));

                printf("Pri->%d\n", pri);
                if(pri<minpriority)
                {
                    minpriority=pri;

                    std::cout << "---> NEW EXCHANGE = " << exchange << endl;

                    std::string new_exchange = std::string(exchange);
                    serverNameTest = new_exchange.c_str();


                    strncpy(serverName,exchange,strlen(exchange));

                    std::cout << "---> NEW SEVER NAME = " << serverName << endl;

                    printf("serverName->%s\n", serverName);
                    printf("CurrentExchange->%s\n", exchange);
                }
                printf("Exchange->%s\n", exchange);
            }
#endif
        }

        std::cout << "----> Final server name: " << serverNameTest << std::endl;


        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            fprintf(stderr, "Error: Could not open SOCKET\n");
            exit(EXIT_FAILURE);
        }


        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(25);
        inet_pton(AF_INET, serverNameTest.c_str(), &(address.sin_addr));
        struct hostent *hp =gethostbyname(serverNameTest.c_str());
        memcpy((char*)&(address.sin_addr),hp->h_addr,hp->h_length);



        int connection_ret= connect(sockfd, (struct sockaddr*)&address, sizeof(address));
        if (connection_ret < 0) {
            std::cout << "Error connecting..." << std::endl;
            return;
        }
        else{
            std::cout << "Connected" << std::endl;
        }

        int stage=1;
        while(true)
        {
            string buff;
            string aks;
            char ch;

            while (read(sockfd, &ch, sizeof(ch)) > 0 )				//read all characters received from client and store into buffer
            {
                if(ch=='\n')
                {
                    if(aks[aks.length()-1]=='\r')
                    {
                        aks.push_back(ch);
                        buff=aks;
                        aks.clear();
                        break;
                    }
                }
                else
                {
                    aks.push_back(ch);
                }

            }
            cout<<"buff="<<buff<<"\n";
            sleep(1);

            string substrk=buff.substr(0,4);
            if(comparek(substrk,"220 ")&&stage==1)
            {
                string hellostr="HELO "+recepient_domain+"\r\n";
                cout<<"hellostr="<<hellostr<<"\n";
                write(sockfd, hellostr.c_str(), hellostr.length());
                stage=2;
            }
            else if(comparek(substrk,"250 ")&&stage==2)
            {
                string mailfromstr="MAIL FROM:<"+email.sender+">\r\n";
                cout<<"mailfromstr="<<mailfromstr<<"\n";
                write(sockfd, mailfromstr.c_str(), mailfromstr.length());
                stage=3;
            }
            else if(comparek(substrk,"250 ")&&stage==3)
            {
                string rcptstr="RCPT TO:<"+email.recepients[0]+">\r\n";
                cout<<"rcptstr="<<rcptstr<<"\n";
                write(sockfd, rcptstr.c_str(), rcptstr.length());
                stage=4;
            }
            else if(comparek(substrk,"250 ")&&stage==4)
            {
                string datastr="DATA\r\n";
                write(sockfd, datastr.c_str(), datastr.length());


                stage=5;
            }
            else if (comparek(substrk,"354 ")&&stage==5)
            {
                string datastr=serialize_email(email, email.recepients[0]);
                cout<<"datastr="<<datastr<<"\n";
                write(sockfd, datastr.c_str(), datastr.length());

                datastr=".\r\n";
                cout<<"datastr="<<datastr<<"\n";
                write(sockfd, datastr.c_str(), datastr.length());

                stage=6;
            }
            else if(comparek(substrk,"250 ")&&stage==6)
            {
                string quitstr="QUIT\r\n";
                cout<<"quitstr="<<quitstr<<"\n";
                write(sockfd, quitstr.c_str(), quitstr.length());
                stage=7;
                break;
            }
        }

        close(sockfd);
        if(stage==7)
            queue_emails_for_externalservers.pop();

    }
private:

    std::string get_email_unique_id(std::string serialized_email){
        std::hash<std::string> hashFunction;
        std::size_t hashValue = hashFunction(serialized_email);
        return std::to_string(hashValue);
    }

    std::string serialize_email(Email email, std::string recepient_email){
        std::string email_content;
        email_content += "From: " + email.sender + NEW_LINE;
        email_content += "To: " + recepient_email + NEW_LINE;
        email_content += "Date: " +  get_current_time() + NEW_LINE;
        email_content += "Subject: " + email.subject + NEW_LINE;
        email_content += email.content + NEW_LINE;
        return email_content;
    }
    std::string get_current_time(){
        time_t current_time = time(NULL);
        struct tm * current_time_tm = localtime (&current_time);
        return asctime(current_time_tm);
    }
    /*int putEmail_in_Penncloud(std::string recepient_row_id, std::string recepient_email, Email email)
    {
        std::string metadata_content;
        bigTable.get(recepient_row_id, METADATA_COL, metadata_content);

        std::string serialized_email = serialize_email(email, recepient_email);
        std::string email_column_id = get_email_unique_id(serialized_email);

        // Append new email column ID to metadata
        metadata_content += email_column_id + NEW_LINE;
        bigTable.put(recepient_row_id, METADATA_COL, metadata_content);

        //Put new email in bigtable with its column ID appended
        serialized_email += "hashValueString: " + email_column_id + NEW_LINE;

        return bigTable.put(recepient_row_id, email_column_id, serialized_email);
    }*/
    int putEmail_in_Penncloud(std::string recepient_row_id, std::string recepient_email, Email email)
    {
        std::string metadata_content;
        int metadata_return = bigTable.get(recepient_row_id, METADATA_COL, metadata_content);
        if (metadata_return == FAILURE || metadata_content.find(EMPTY_DATA) != std::string::npos) metadata_content = "";

        std::cout << "Metadata: " << metadata_content << std::endl;

        /* Append new email column ID to metadata */
        std::string serialized_email = serialize_email(email, recepient_email);
        std::string email_column_id = get_email_unique_id(serialized_email);
        std::string new_metadata_content = metadata_content+email_column_id + NEW_LINE;

        std::cout << "New metadata " << new_metadata_content <<  std::endl;

        /* Edit metadata or try putting email again */
        if (bigTable.cput(recepient_row_id, METADATA_COL, metadata_content,new_metadata_content) != SUCCESS) {
            putEmail_in_Penncloud(recepient_row_id, recepient_email, email);
        }

        /* Put new email in bigtable with its column ID appended */
        serialized_email += "hashValueString: " + email_column_id + NEW_LINE;

        return bigTable.put(recepient_row_id, email_column_id, serialized_email);
    }
};


