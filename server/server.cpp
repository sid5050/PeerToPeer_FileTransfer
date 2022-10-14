#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <string.h>
#include <pthread.h>
#include <fstream>
#define PORT 8885
using namespace std;
string userInfo = "userinfo.txt";


struct UserData {
	char uname[50];
	char password[50];
};

struct grpData {
	char admin[50];
	char grpID[50];
};

struct grpMembers {
	char grpID[50];
	char member[50];
};

struct grpRequest {
	char grpID[50];
	char usrname[50];
};

struct grpMemberFileData {
	char grpID[50];
	char usrname[50];
	char fileName[50];
	char filePath[300];
};

//App users userid, passwd
map<string, string> appUsers;
//loggedIn users userid, passwd
map<string, string> loggedUsers;
//loggedIn users userid, portNo
map<string, string> clientPorts;
//loggedIn users userid, IP address
map<string, string> clientIPs;
//List of grps a user is owner/admin of
map<string, vector<string>> grpOwner;
//Set of available groups
set<string> availableGroups;
//List of members of a group
map<string, vector<string>> grpMemberList;
//List of pending request of members to join group
map<string, vector<string>> grpRequestList;
//List of files a grp member will have
map<pair<string, string>, map<string, string>> grpMemberFileList;
//List of members of a file in a group
map<pair<string, string>, set<string>> grpFileMemberList;

void *newAccount(void *userCred) {
	string signUpData = (char *)userCred;
	int pos = signUpData.find(" ");
	int i;
	int sockfd = stoi(signUpData.substr(0, pos));
	string uname, pass, tmp;
	for(i = pos+1;i < signUpData.size();i++) {
		if(signUpData[i] == ' ') {
			i++;
			uname = tmp;
			break;
		}
		tmp += signUpData[i];
	}
	
	uname = tmp;
	tmp = "";
	for(;i < signUpData.size();i++) {
		tmp += signUpData[i];
	}
	pass = tmp;

	cout << "Tracker in newAccount Function and has uname and passwd as: " << uname << " " << pass << endl;


	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	if(appUsers.find(uname) != appUsers.end()) {
		string pswd = appUsers[uname];
		if(pass == pswd) {
			string msg = "User Already Exists!!\n";
			strcpy(reply, msg.c_str());
			send(sockfd, reply, strlen(reply), 0);
			pthread_exit(NULL);
			return NULL;
		}
	}

	appUsers[uname] = pass;

	struct UserData ud;
	// ud.uname = uname.c_str();
	strcpy(ud.uname, uname.c_str());
	strcpy(ud.password, pass.c_str());
	// ud.password = pass.c_str();
	printf("UD-->%s --- %s\n",ud.uname, ud.password);

	FILE *datafile;
	datafile = fopen("userdata.dat", "a");
	if(!datafile) {
		cout << "Invalid UserData file" << endl;
		exit(1);
	}
	fwrite(&ud, sizeof(struct UserData), 1, datafile);
	if(fwrite != 0) {
		string msg = "User Created Successfully!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
	} else {
		string msg = "File Handling Error!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
	}
	fclose(datafile);
	pthread_exit(NULL);
	return NULL;
}



void *loginUser(void *userCred) {
	string signUpData = (char *)userCred;
	int pos = signUpData.find(" ");
	int i;
	int sockfd = stoi(signUpData.substr(0, pos));
	string uname, pass, tmp;
	for(i = pos+1;i < signUpData.size();i++) {
		if(signUpData[i] == ' ') {
			uname = tmp;
			i++;
			break;
		}
		tmp += signUpData[i];
	}
	
	uname = tmp;
	tmp = "";
	for(;i < signUpData.size();i++) {
		tmp += signUpData[i];
	}
	pass = tmp;

	cout << "Tracker in LoginUser Function and has uname and passwd as: " << uname << " " << pass << endl;
	for(auto itr = appUsers.begin();itr != appUsers.end();itr++)
		cout << itr->first << "---" << itr->second << endl;

	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	int exists = 0;
	if(appUsers.find(uname) != appUsers.end()) {
		string pswd = appUsers[uname];
		if(pass == pswd) {
			exists = 1;
		}
	}
	if(exists == 0) {
		string msg = "Invalid User Credentials!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}
	
	
	loggedUsers[uname] = pass;

	
	string msg = "User Logged In Successfully!!";
	strcpy(reply, msg.c_str());
	send(sockfd, reply, strlen(reply), 0);

	cout << "Msg sent" << endl;
	memset(&reply[0], 0, sizeof(reply));
	recv(sockfd, reply, 1024, 0);

	string cltData = reply;
	pos = cltData.find(" ");
	int cport = stoi(cltData.substr(0, pos));
	string clientIP = cltData.substr(pos+1);
	clientPorts[uname] = cport;
	clientIPs[uname] = clientIP;
	cout << "Client " << uname << "Port is " << cport << " IP is " << clientIP << endl;

	pthread_exit(NULL);
	return NULL;
}


void *createGrp(void *grpData) {
	string Data = (char *)grpData;
	int pos = Data.find(" ");
	int i;
	int sockfd = stoi(Data.substr(0, pos));


	string grpID, admin, tmp;
	for(i = pos+1;i < Data.size();i++) {
		if(Data[i] == ' ') {
			i++;
			grpID = tmp;
			break;
		}
		tmp += Data[i];
	}
	
	// uname = tmp;
	tmp = "";
	for(;i < Data.size();i++) {
		tmp += Data[i];
	}
	admin = tmp;

	cout << "Tracker in CreateGrp Function and has admin and grpid as: " << admin << " " << grpID << endl;
	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	// auto itr = find(availableGroups.begin(), availableGroups.end(), grpID);
	if(availableGroups.find(grpID) != availableGroups.end()) {
		string msg = "GrpID already exists!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}

	if(grpOwner.find(admin) != grpOwner.end()) {
		vector<string> usrgrps = grpOwner[admin];
		for(int i = 0;i < usrgrps.size();i++) {
			if(usrgrps[i] == grpID) {
				string msg = "GrpID already exists!!\n";
				strcpy(reply, msg.c_str());
				send(sockfd, reply, strlen(reply), 0);
				pthread_exit(NULL);
				return NULL;
			}
		}
	}

	grpOwner[admin].push_back(grpID);


	struct grpData gd;
	// ud.uname = uname.c_str();
	strcpy(gd.grpID, grpID.c_str());
	strcpy(gd.admin, admin.c_str());
	// ud.password = pass.c_str();
	printf("GD-->%s --- %s\n",gd.grpID, gd.admin);

	FILE *datafile;
	datafile = fopen("groupdata.dat", "a");
	if(!datafile) {
		cout << "Invalid GroupData file" << endl;
		exit(1);
	}
	fwrite(&gd, sizeof(struct grpData), 1, datafile);
	if(fwrite != 0) {
		string msg = "Group Created Successfully!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		availableGroups.insert(grpID);
	} else {
		string msg = "Group File Handling Error!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
	}
	fclose(datafile);



	struct grpMembers gm;
	strcpy(gm.grpID, grpID.c_str());
	strcpy(gm.member, admin.c_str());
	printf("GM-->%s --- %s\n",gm.grpID, gm.member);

	// FILE *datafile;
	datafile = fopen("groupmember.dat", "a");
	if(!datafile) {
		cout << "Invalid groupmember file" << endl;
		exit(1);
	}
	fwrite(&gm, sizeof(struct grpMembers), 1, datafile);
	if(fwrite == 0) {
		string msg = "groupmember File Handling Error!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);	
	}
	fclose(datafile);


	pthread_exit(NULL);
	return NULL;

}


void *joinGrp(void *grpData) {
	string Data = (char *)grpData;
	int pos = Data.find(" ");
	int i;
	int sockfd = stoi(Data.substr(0, pos));


	string grpID, user, tmp;
	for(i = pos+1;i < Data.size();i++) {
		if(Data[i] == ' ') {
			i++;
			grpID = tmp;
			break;
		}
		tmp += Data[i];
	}
	
	// uname = tmp;
	tmp = "";
	for(;i < Data.size();i++) {
		tmp += Data[i];
	}
	user = tmp;

	cout << "Tracker in joinGrp Function and has username and grpid as: " << user << " " << grpID << endl;
	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	// auto itr = find(availableGroups.begin(), availableGroups.end(), grpID);
	if(availableGroups.find(grpID) == availableGroups.end()) {
		string msg = "GrpID does not exist!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}

	if(grpOwner.find(user) != grpOwner.end()) {
		vector<string> usrgrps = grpOwner[user];
		for(int i = 0;i < usrgrps.size();i++) {
			if(usrgrps[i] == grpID) {
				string msg = "You are a group member(admin)!!\n";
				strcpy(reply, msg.c_str());
				send(sockfd, reply, strlen(reply), 0);
				pthread_exit(NULL);
				return NULL;
			}
		}
	}


	// struct grpRequest {
	// char grpID[50];
	// char usrname[50];
	// };

	struct grpRequest gr;
	strcpy(gr.grpID, grpID.c_str());
	strcpy(gr.usrname, user.c_str());
	printf("GR-->%s --- %s\n",gr.grpID, gr.usrname);

	FILE *datafile;
	datafile = fopen("grouprequest.dat", "a");
	if(!datafile) {
		cout << "Invalid grouprequest file" << endl;
		exit(1);
	}
	fwrite(&gr, sizeof(struct grpRequest), 1, datafile);
	if(fwrite != 0) {
		string msg = "Group Join Request Sent Successfully!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		// availableGroups.insert(grpID);
		grpRequestList[grpID].push_back(user);
	} else {
		string msg = "grouprequest File Handling Error!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
	}
	fclose(datafile);
	pthread_exit(NULL);
	return NULL;

}



void *listGrpRequest(void *grpData) {
	string Data = (char *)grpData;
	int pos = Data.find(" ");
	int i;
	int sockfd = stoi(Data.substr(0, pos));


	string grpID, user, tmp;
	for(i = pos+1;i < Data.size();i++) {
		if(Data[i] == ' ') {
			i++;
			grpID = tmp;
			break;
		}
		tmp += Data[i];
	}
	
	// uname = tmp;
	tmp = "";
	for(;i < Data.size();i++) {
		tmp += Data[i];
	}
	user = tmp;

	cout << "Tracker in listGrpRequest Function and has username and grpid as: " << user << " " << grpID << endl;
	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	// auto itr = find(availableGroups.begin(), availableGroups.end(), grpID);
	if(availableGroups.find(grpID) == availableGroups.end()) {
		string msg = "GrpID does not exist!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}

	if(grpOwner.find(user) != grpOwner.end()) {
		vector<string> usrgrps = grpOwner[user];
		for(int i = 0;i < usrgrps.size();i++) {
			if(usrgrps[i] == grpID) {

				vector<string> list = grpRequestList[grpID];

				string msg = "Group Join Requests are from users:\n";
				for(string ele : list) {
					msg += ele + "\n";
				}
				strcpy(reply, msg.c_str());
				send(sockfd, reply, strlen(reply), 0);
				pthread_exit(NULL);
				return NULL;
			}
		}
	}


	string msg = user + " is not an admin of group " + grpID + "\n";
	strcpy(reply, msg.c_str());
	send(sockfd, reply, strlen(reply), 0);
	availableGroups.insert(grpID);
	pthread_exit(NULL);
	return NULL;

}



void *uploadFile(void *grpData) {
	string Data = (char *)grpData;
	int pos = Data.find(" ");
	int i;
	int sockfd = stoi(Data.substr(0, pos));


	string filePath, grpID, tmp, usrID;
	for(i = pos+1;i < Data.size();i++) {
		if(Data[i] == ' ') {
			i++;
			filePath = tmp;
			break;
		}
		tmp += Data[i];
	}
	
	tmp = "";
	for(;i < Data.size();i++) {
		if(Data[i] == ' ') {
			i++;
			grpID = tmp;
			break;
		}
		tmp += Data[i];
	}

	tmp = "";
	for(;i < Data.size();i++) {
		tmp += Data[i];
	}
	usrID = tmp;


	string fileName = "";
	tmp = "";
	for(int i = filePath.size()-1;i >= 0;i--) {
		if(filePath[i] == '/') {
			fileName = tmp;
			break;
		}
		tmp += filePath[i];
	}
	reverse(fileName.begin(), fileName.end());
	cout << "Tracker in uploadFile Function and has filePath and fileName and grpid and usrID as: " << filePath << " " 
	<< fileName << grpID << " " << usrID << endl;

	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	if(availableGroups.find(grpID) == availableGroups.end()) {
		string msg = "GrpID does not exist!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}

	vector<string> members = grpMemberList[grpID];
	int flag = 0;
	for(int i = 0;i < members.size();i++) {
		if(members[i] == usrID) {
			flag = 1;
			break;	
		}
	}

	if(!flag) {
		string msg = usrID + " is not an part of group " + grpID + "\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		availableGroups.insert(grpID);
		pthread_exit(NULL);
		return NULL;
	}

	// pair<string, string> grpMember = {grpID, usrID};
	// grpMemberFileList[grpMember][fileName] = filePath;
	// pair<string, string> grpFile = {grpID, fileName};
	// grpFileMemberList[grpFile].insert(usrID);

	struct grpMemberFileData gmf;
	strcpy(gmf.grpID, grpID.c_str());
	strcpy(gmf.usrname, usrID.c_str());
	strcpy(gmf.fileName, fileName.c_str());
	strcpy(gmf.filePath, filePath.c_str());
	printf("GMF-->%s --- %s --- %s --- %s\n",gmf.grpID, gmf.usrname, gmf.fileName, gmf.filePath);

	FILE *datafile;
	datafile = fopen("groupmemberfile.dat", "a");
	if(!datafile) {
		cout << "Invalid grouprequest file" << endl;
		exit(1);
	}
	fwrite(&gmf, sizeof(struct grpMemberFileData), 1, datafile);
	
	if(fwrite == 0) {
		string msg = "groupmember File Handling Error!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		fclose(datafile);
		pthread_exit(NULL);
		return NULL;
	}
	fclose(datafile);
	string msg = fileName + " uploaded successfully to group " + grpID + "\n";
	pair<string, string> grpMember = {grpID, usrID};
	grpMemberFileList[grpMember][fileName] = filePath;
	pair<string, string> grpFile = {grpID, fileName};
	grpFileMemberList[grpFile].insert(usrID);
	strcpy(reply, msg.c_str());
	send(sockfd, reply, strlen(reply), 0);
	pthread_exit(NULL);
	return NULL;
}







void *acceptGrpRequest(void *grpData) {
	string Data = (char *)grpData;
	int pos = Data.find(" ");
	int i;
	int sockfd = stoi(Data.substr(0, pos));


	string grpID, user, admin, tmp;
	for(i = pos+1;i < Data.size();i++) {
		if(Data[i] == ' ') {
			grpID = tmp;
			i++;
			break;
		}
		tmp += Data[i];
	}
	
	// uname = tmp;
	tmp = "";
	for(;i < Data.size();i++) {
		if(Data[i] == ' ') {
			user = tmp;
			i++;
			break;
		}
		tmp += Data[i];
	}
	admin = Data.substr(i);

	cout << "Tracker in acceptGrpRequest Function and has admin, grpid and user as: " << admin << " " << grpID << " "<< user << endl;
	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	// auto itr = find(availableGroups.begin(), availableGroups.end(), grpID);
	if(availableGroups.find(grpID) == availableGroups.end()) {
		string msg = "GrpID does not exist!!\n";
		strcpy(reply, msg.c_str());
		send(sockfd, reply, strlen(reply), 0);
		pthread_exit(NULL);
		return NULL;
	}

	if(grpOwner.find(admin) != grpOwner.end()) {
		cout << "Inside" << endl;
		vector<string> usrgrps = grpOwner[admin];
		for(int i = 0;i < usrgrps.size();i++) {
			cout << usrgrps[i] << endl;
			if(usrgrps[i] == grpID) {
				vector<string> list = grpRequestList[grpID];
				bool flag = 0;
				for(string ele : list) {
					if(ele == user) {
						flag = 1;
						break;
					}
				}
				string msg;
				if(!flag)
					msg = user + " is already a member of Group: " + grpID + "\n";
				else {
					msg = user + " added to Group: " + grpID + "\n";

					for(int i = 0;i < grpRequestList[grpID].size();i++) {
						if(grpRequestList[grpID][i] == user) {
							grpRequestList[grpID].erase(grpRequestList[grpID].begin()+i);
							break;
						}
					}

					if (remove("grouprequest.dat") == 0){
				    	printf("Deleted successfully\n");

				    
				    	FILE *datafile;
				  		datafile = fopen("grouprequest.dat", "a");
				  		for(int i = 0;i < grpRequestList[grpID].size();i++) {
				  			struct grpRequest gr;
							strcpy(gr.grpID, grpID.c_str());
							strcpy(gr.usrname, grpRequestList[grpID][i].c_str());
							// printf("GR-->%s --- %s\n",gr.grpID, gr.usrname);

					  		if(!datafile) {
								cout << "Invalid grouprequest file" << endl;
								exit(1);
					   		}
							fwrite(&gr, sizeof(struct grpRequest), 1, datafile);
							if(fwrite == 0) {
								string msg = "grouprequest File Handling Error!!\n";
								// strcpy(reply, msg.c_str());
								// send(sockfd, reply, strlen(reply), 0);
								exit(1);
							}
						}
						fclose(datafile);
					}
				   else{
				      printf("Unable to delete grpRequest file");
				      exit(1);
				   }

					grpMemberList[grpID].push_back(user);
					struct grpMembers gm;
					strcpy(gm.grpID, grpID.c_str());
					strcpy(gm.member, user.c_str());
					printf("GM-->%s --- %s\n",gm.grpID, gm.member);

					FILE *datafile;
					datafile = fopen("groupmember.dat", "a");
					if(!datafile) {
						cout << "Invalid groupmember file" << endl;
						exit(1);
					}
					fwrite(&gm, sizeof(struct grpMembers), 1, datafile);
					if(fwrite == 0) {
						string msg = "groupmember File Handling Error!!\n";
						strcpy(reply, msg.c_str());
						send(sockfd, reply, strlen(reply), 0);	
					}
					fclose(datafile);
				}
				strcpy(reply, msg.c_str());
				send(sockfd, reply, strlen(reply), 0);
				pthread_exit(NULL);
				return NULL;
			}
		}
	}


	string msg = admin + " is not an admin of group " + grpID + "\n";
	strcpy(reply, msg.c_str());
	send(sockfd, reply, strlen(reply), 0);
	availableGroups.insert(grpID);
	pthread_exit(NULL);
	return NULL;

}



void *listAllGrps(void *sockData) {
	string Data = (char *)sockData;
	int sockfd = stoi(Data);


	cout << "Tracker in listAllGrps Function " << endl;
	char reply[1024];
	memset(&reply[0], 0, sizeof(reply));

	// auto itr = find(availableGroups.begin(), availableGroups.end(), grpID);
	string msg = "Following Groups exists:\n";
	for(auto itr = availableGroups.begin(); itr != availableGroups.end(); itr++) {
		msg += (*itr) + "\n";
	}
	strcpy(reply, msg.c_str());
	send(sockfd, reply, strlen(reply), 0);
	pthread_exit(NULL);
	return NULL;
}





void getData() {
	
    FILE *inpFile;
    inpFile = fopen("userdata.dat", "r");
    if(!inpFile) {
    	cout << "File does not Exist!" << endl;
    	return;
    }
    struct UserData ud;
    while (fread(&ud, sizeof(struct UserData), 1, inpFile)) {
        appUsers[ud.uname] = ud.password;
    }
    cout << "User Data is " << endl;
    for(auto itr = appUsers.begin(); itr != appUsers.end(); itr++) {
    	cout << itr->first << " " << itr->second << endl;
    }
    fclose(inpFile);



    // FILE *inpFile;
    inpFile = fopen("groupdata.dat", "r");
    if(!inpFile) {
    	cout << "File does not Exist!" << endl;
    	return;
    }
    struct grpData gd;
    while (fread(&gd, sizeof(struct grpData), 1, inpFile)) {
        grpOwner[gd.admin].push_back(gd.grpID);
        availableGroups.insert(gd.grpID);
    }
    cout << "Grp Data is " << endl;
    for(auto itr = grpOwner.begin(); itr != grpOwner.end(); itr++) {
    	// cout << itr->first << " " << itr->second << endl;
    	// if(grpOwner.find(" ved") != grpOwner.end())
    	// 	cout << "YO" << endl;
    	cout << "\nGroup Admin" << itr->first << endl;
    	vector<string> list = itr->second;
    	for(int i = 0;i < list.size();i++) {
    		cout << list[i] << " ";
    	}
    	cout << endl;
    }
    fclose(inpFile);
    


    inpFile = fopen("groupmember.dat", "r");
    if(!inpFile) {
    	cout << "File does not Exist!" << endl;
    	return;
    }
    struct grpMembers gm;
    while (fread(&gm, sizeof(struct grpMembers), 1, inpFile)) {
        grpMemberList[gm.grpID].push_back(gm.member);
    }

    cout << "\nGrp Members are " << endl;
    for(auto itr = grpMemberList.begin(); itr != grpMemberList.end(); itr++) {
    	// cout << itr->first << " " << itr->second << endl;
    	cout << "Group Name " << itr->first << endl;
    	vector<string> list = itr->second;
    	for(int i = 0;i < list.size();i++) {
    		cout << list[i] << " ";
    	}
    	cout << endl;
    }
    fclose(inpFile);



    // map<string, vector<string>> grpRequestList;
    inpFile = fopen("grouprequest.dat", "r");
    if(!inpFile) {
    	cout << "File does not Exist!" << endl;
    	return;
    }
    struct grpRequest gr;
    while (fread(&gr, sizeof(struct grpRequest), 1, inpFile)) {
        grpRequestList[gr.grpID].push_back(gr.usrname);
        // availableGroups.insert(gd.grpID);
    }
    cout << "\nGrp Requests are " << endl;
    for(auto itr = grpRequestList.begin(); itr != grpRequestList.end(); itr++) {
    	// cout << itr->first << " " << itr->second << endl;
    	cout << "Group Name " << itr->first << endl;
    	vector<string> list = itr->second;
    	for(int i = 0;i < list.size();i++) {
    		cout << list[i] << " ";
    	}
    	cout << endl;
    }
    fclose(inpFile);



    inpFile = fopen("groupmemberfile.dat", "r");
    if(!inpFile) {
    	cout << "grpMemberFile does not Exist!" << endl;
    	return;
    }
    struct grpMemberFileData gmf;
    while (fread(&gmf, sizeof(struct grpMemberFileData), 1, inpFile)) {
    	string grpID = gmf.grpID;
    	string usrID = gmf.usrname;
    	string fileName = gmf.fileName;
    	string filePath = gmf.filePath;
    	pair<string, string> grpMember = {grpID, usrID};
		grpMemberFileList[grpMember][fileName] = filePath;

		pair<string, string> grpFile = {grpID, fileName};
		grpFileMemberList[grpFile].insert(usrID);
    }
    cout << "\nGrp Member Files are " << endl;
    for(auto itr = grpMemberFileList.begin(); itr != grpMemberFileList.end(); itr++) {
    	// cout << itr->first << " " << itr->second << endl;
    	cout << "\nGroup Name and Member Name" << itr->first.first << " " << itr->first.second << endl;
    	map<string, string> mp = itr->second;
    	// 
    	for(auto itr1 : mp) {
    		cout << itr1.first << " " << itr1.second << endl;
    	}
    	cout << endl;
    }

    cout << "\nGrp File Owners are " << endl;
    for(auto itr = grpFileMemberList.begin(); itr != grpFileMemberList.end(); itr++) {
    	// cout << itr->first << " " << itr->second << endl;
    	cout << "\nGroup Name and File Name" << itr->first.first << " " << itr->first.second << endl;
    	set<string> st = itr->second;
    	// 
    	for(auto itr1 = st.begin();itr1 != st.end();itr1++) {
    		cout << *itr1 << endl;
    	}
    	cout << endl;
    }
    fclose(inpFile);
    cout << endl;
}


int main(){
	getData();
	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Tracker Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", PORT);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

	
		memset(&buffer[0], 0, sizeof(buffer));
		recv(newSocket, buffer, 1024, 0);

		// cout << "Enter" << endl;
		pthread_t createAccountThread, loginUserThread, createGrpThread, joinGrpThread, uploadFileThread, 
		listRequestThread, listAllGrpsThread, acceptRequestThread;

		int createUser = 0, loginCheck = 0, createGrpCheck = 0, joinGrpCheck = 0, listRequestCheck = 0, listAllCheck = 0, 
		acceptRequestCheck = 0, uploadFileCheck = 0;
		string usrcmd = buffer;
		cout << "\nNew Request from client, Here: "<<usrcmd << endl;
		int pos = usrcmd.find(" ");
		string cmdtype = usrcmd.substr(0, pos);
		char buff[1024];
		
		if(strcmp(buffer, ":exit") == 0){
			printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
			break;
		} else if(cmdtype == "new_account") {
			string credData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + credData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&createAccountThread, NULL, newAccount, buff);
			usleep(1000);
			createUser = 1;
		} else if(cmdtype == "login") {
			string credData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + credData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&loginUserThread, NULL, loginUser, buff);
			usleep(1000);
			loginCheck = 1;
		} else if(cmdtype == "newgrp") {
			string grpData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + grpData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&createGrpThread, NULL, createGrp, buff);
			usleep(1000);
			createGrpCheck = 1;
		} else if(cmdtype == "joingrp") {
			string grpData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + grpData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&joinGrpThread, NULL, joinGrp, buff);
			usleep(1000);
			joinGrpCheck = 1;
		} else if(cmdtype == "listrequest") {
			string grpData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + grpData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&listRequestThread, NULL, listGrpRequest, buff);
			usleep(1000);
			listRequestCheck = 1;
		} else if(usrcmd == "listallgrps") {
			string socket = to_string(newSocket);
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, socket.c_str());
			pthread_create(&listAllGrpsThread, NULL, listAllGrps, buff);
			usleep(1000);
			listAllCheck = 1;
		} else if(cmdtype == "acceptrequest") {
			string grpData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + grpData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&acceptRequestThread, NULL, acceptGrpRequest, buff);
			usleep(1000);
			acceptRequestCheck = 1;
		} else if(cmdtype == "uploadfile") {
			string grpData = usrcmd.substr(pos);
			string socket = to_string(newSocket);
			string sendData = socket + grpData;
			memset(&buff[0], 0, sizeof(buff));
			strcpy(buff, sendData.c_str());
			pthread_create(&uploadFileThread, NULL, uploadFile, buff);
			usleep(1000);
			uploadFileCheck = 1;
		}

		if(createUser) {
			pthread_join(createAccountThread, NULL);
		}
		if(loginCheck) {
			pthread_join(loginUserThread, NULL);
		}
		if(createGrpCheck) {
			pthread_join(createGrpThread, NULL);
		}
		if(joinGrpCheck) {
			pthread_join(joinGrpThread, NULL);
		}
		if(listRequestCheck) {
			pthread_join(listRequestThread, NULL);
		}
		if(listAllCheck) {
			pthread_join(listAllGrpsThread, NULL);
		}
		if(acceptRequestCheck) {
			pthread_join(acceptRequestThread, NULL);
		}
		if(uploadFileCheck) {
			pthread_join(uploadFileThread, NULL);	
		}

	}
	cout << "Exit" << endl;
	close(newSocket);
	close(sockfd);


	return 0;
}
