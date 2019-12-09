//
// Created by Kanika Prasad Nadkarni on 11/19/19.
//

#include "email.h"

//std::string CONFIG_PATH = "./config.txt";
//int NUMBER_OF_PRIMARIES = 1;
//bool PRINT_LOGS = false;
std::string colName = "METADATA";

class EmailClient {

	//BigTableClient bigTable;

public:
	void initialize() {
		//bigTable.initialize(CONFIG_PATH, NUMBER_OF_PRIMARIES, PRINT_LOGS);
	}

	std::string listEmails(std::string usr, std::vector<email>& listMailbox) {

//	    // std::vector<email>listMailbox;
//	    std::string rowName = usr+"_email";
//	    //cout<<"listEmailsrowName="<<rowName<<"\n";
//	    //cout<<"listEmailscolName="<<colName<<"\n";
//	    std::string metadataContent;
//	    int retval = -1//bigTable.get(rowName,colName, metadataContent);
//
//	    if (retval == -1) {
//	    	// TODO: bigtable is down
//	    	return "BACKEND_DEAD";
//	    }
//
//	    //std::cout<<"listEmailsmetadataContent="<<metadataContent<<"\n";
//	    //cout<<"listEmailsmailboxContent"<<mailboxContent<<"\n";
//	    std::string line;
//
//	    std::istringstream metadataStream(metadataContent);
//	    while (getline(metadataStream,line)) {
//	        std::string currentEmail;
//	        int retval=//bigTable.get(rowName,line,currentEmail);
//	        if (retval == -1) {
//	    	// TODO: bigtable is down
//	    	return "BACKEND_DEAD";
//	    	}
//	        std::istringstream currentMailStream(currentEmail);
//            std::string tempLine;
//            email obj;
//            while (getline(currentMailStream,tempLine))
//            {
//                if (tempLine.substr(0, 6) == "From: ")
//                {
//                    obj.sender=tempLine.substr(6,std::string::npos);
//                    //cout<<"obj.sender="<<obj.sender<<"\n";
//                }
//                else if(tempLine.substr(0, 4) == "To: ")
//                {
//                    obj.recepients.push_back(tempLine.substr(4,std::string::npos));
//                }
//                else if (tempLine.substr(0, 6) == "Date: ")
//                {
//                    obj.timestamp=tempLine.substr(6,std::string::npos);
//                }
//                else if (tempLine.substr(0, 9) == "Subject: ")
//                {
//                    obj.subject=tempLine.substr(9,std::string::npos);
//                }
//                else if (tempLine.substr(0, 17) == "hashValueString: ")
//                {
//                    obj.hashValueString=tempLine.substr(17,std::string::npos);
//                }
//                else
//                {
//                    obj.content=obj.content+tempLine+"\n";
//                }
//            }
//            listMailbox.push_back(obj);
//	    }
//
	    return "SUCCESS";
	}

	std::string deleteEmail(email obj) {
//		/*std::string valueEmail="From: "+obj.sender+"\nTo: "+obj.recepients[0]+"\nDate: "+
//		                obj.timestamp+"\nSubject: "+obj.subject+"\n"+obj.content+"\n";
//        std::cout<<"deleteEmailvalueEmail="<<valueEmail<<"\n";
//        std::hash<std::string> hashFunction;
//		std::size_t hashValue=hashFunction(valueEmail);
//		std::string hashValueString=to_string(hashValue);*/
//		std::string hashValueString=obj.hashValueString;
//		//std::cout<<"hashValueString="<<hashValueString<<"\n";
//
//		std::string temp;
//		//std::cout<<"SenderTimestamp="<<SenderTimestamp<<"\n";
//		//std::cout<<"in delete function\n";
//		std::string rowName = obj.recepients[0].substr(0,obj.recepients[0].find("@"))+"_email";
//	    //std::cout<<"delrowName="<<rowName<<"\n";
//	    //cout<<"delcolName="<<colName<<"\n";
//
//	    //std::string metadataContent = bigTable.get(rowName,colName);
//	    std::string metadataContent;
//			    int retval = bigTable.get(rowName,colName, metadataContent);
//
//			    if (retval == -1) {
//			    	// TODO: bigtable is down
//			    	return "BACKEND_DEAD";
//			    }
//	    //std::cout<<"delmailboxContent"<<mailboxContent<<"\n";
//	    //std::string here="here\n";
//	    //std::cout<<"here="<<here<<"\n";
//	    std::string line;
//		std::istringstream metadataStream(metadataContent);
//	    while (getline(metadataStream,line)) {
//	    	if(hashValueString!=line)
//	    	{
//	    		temp=temp+line+"\n";
//	    	}
//	    	/*else
//	    	{
//	    		//std::cout<<"matched$$$$$$$$$$$$\n";
//	    		//std::cout<<"hashValueString="<<hashValueString<<"\n";
//	    		//std::cout<<"line="<<line<<"\n";
//	    		//std::cout<<"matched$$$$$$$$$$$$\n";
//	    	}*/
//	    }
//	    //std::cout<<"temp="<<temp<<"\n";
//	    bigTable.put(rowName,colName,temp);
//	    //std::string hashValueString=to_string(hashValue);
//	    bigTable.del(rowName,hashValueString);
//	    return "SUCCESS";
 	}

	std::string putEmail(email obj) {
//		//std::string colName=obj.subject+"_"+obj.timestamp;
//	    for(long long int i=0;i<obj.recepients.size();i++){
//	        std::string rowName=obj.recepients[i].substr(0,obj.recepients[i].find("@"))+"_email";
//	        std::string domain   = obj.recepients[i].substr(obj.recepients[i].find("@") + 1,std::string::npos);
//	        //cout<<"domain="<<domain<<"\n";
//	        if(domain=="penncloud"){
//	        	//std::string metadataContent=bigTable.get(rowName,colName);
//	        	std::string metadataContent;
//			    int retval = bigTable.get(rowName,colName, metadataContent);
//
//			    if (retval == -1) {
//			    	// TODO: bigtable is down
//			    	return "BACKEND_DEAD";
//			    }
//		        //cout<<"rowName="<<rowName<<"\n";
//		        time_t current_time = time(NULL);
//		        struct tm * current_time_tm = localtime (&current_time);
//		        std::string valueEmail="From: "+obj.sender+"\nTo: "+obj.recepients[i]+"\nDate: "+
//		                asctime(current_time_tm)+"Subject: "+obj.subject+"\n"+obj.content+"\n";
//		        //std::cout<<"valueEmail="<<valueEmail<<"\n";
//		        std::hash<std::string> hashFunction;
//		        std::size_t hashValue=hashFunction(valueEmail);
//		        std::string hashValueString=std::to_string(hashValue);
//		        metadataContent=metadataContent+hashValueString+"\n";
//	            //std::cout<<"putEmailmetadataContent="<<metadataContent<<"\n";
//		        bigTable.put(rowName,colName,metadataContent);
//		        valueEmail=valueEmail+"hashValueString: "+hashValueString+"\n";
//		        //std::cout << valueEmail << std::endl;
//	            bigTable.put(rowName,hashValueString,valueEmail);
//		    }
//	        //printf("here1\n");
//	    }
	    return "SUCCESS";
 	}

};


