#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <map>
#include <unistd.h>
#include<algorithm>
#include <sys/wait.h>
#include<signal.h>

using namespace std;

extern char** environ;
/*
1. cd
2. pwd
3. echo
4. env
5. setenv
*/
class Commands {
public:
	Commands() {
	}

    static bool isNumberOfArgumentsCorrect(int a, int b, int c){
        return a-b>c;
    }

	static const char* getEnvironmentVariableValue(string str) {
        if (str[0] == '$') {
            char* val = getenv(str.substr(1).c_str());
            if(val == NULL){
                cout<<"Environment variable "+str+" not fount!"<<endl;
                exit(1);
            }
            return val;
		}
		return str.c_str();
	}

	// verify cd output
	static bool cmd_cd(int &currentIndex, vector<string> tokens) {
		if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            const char* path = getEnvironmentVariableValue(tokens[currentIndex]);
            int ch = chdir(path);
            currentIndex++;
            if (ch != 0) {
                printf("chdir change of directory not successful\n");
                return false;
            }
            else {
                return true;
            }
        }
        return false;
	}

	static bool cmd_pwd(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            printf("\tWorkdir: %s\n", getenv("PWD"));
            printf("\tFilepath: %s/%s\n", getenv("PWD"), __FILE__);
            return true;
        }
        return false;
	}

	static bool cmd_echo(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            for (auto it = tokens.begin() + currentIndex; it != tokens.end(); it++) {
                printf("%s ", getEnvironmentVariableValue((*it)));
            }
            currentIndex++;
            return true;
        }
		return false;
	}

	static bool cmd_env(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            for (int i = 0; environ[i] != NULL; i++) {
                printf("%d: %s\n", i, environ[i]);
            }
            return true;
        }
		return false;
	}

	static bool cmd_exit(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            exit(0);
        }
		return false;
	}
	
	static bool cmd_setenv(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,1)){
            const char* var_name = getEnvironmentVariableValue(tokens[currentIndex].c_str());
            const char* var_value = getEnvironmentVariableValue(tokens[currentIndex+1].c_str());
            setenv(tokens[currentIndex].c_str(), var_value, 1);
            
            printf("%s: %s\n", tokens[currentIndex].c_str(), var_value);
            currentIndex+=2;
            return true;
        }
		return false;
	}

	map<string, bool(*)(int&,vector<string>)> GetCommandMap() {
		map<string, bool(*)(int&,vector<string>)> commandMap;
		commandMap["cd"] = cmd_cd;
		commandMap["pwd"] = cmd_pwd;
		commandMap["echo"] = cmd_echo;
		commandMap["env"] = cmd_env;
		commandMap["setenv"] = cmd_setenv;
		commandMap["exit"] = cmd_exit;
		return commandMap;
	}
};


class CommandManager {
private:
	Commands command;
	vector<string> tokens;
	map<string, bool(*)(int&,vector<string>)> commandMap;
public:
	CommandManager() {
		commandMap = command.GetCommandMap();
	}

	bool executeCommand() {
        int i;
        int status;
        for(i=0;i<tokens.size();){
        	status = tryExecuteBuiltInCommand(i);
        	if(status > 0){
        		continue;
        	}
		    else if(status<0){
		    	return false;	
		    }
            status = tryExecuteForkCommand(i);
            if(status > 0){
        		continue;
        	}
		    else if(status<0){
		    	return false;	
		    }
        }
        return false;
	}


	int tryExecuteBuiltInCommand(int &currentIndex) {
		for (auto cmnd : commandMap) {
			if (cmnd.first == tokens[currentIndex]){
                currentIndex += 1;
				bool status = commandMap[cmnd.first](currentIndex, tokens);
				if(status == false) return -1;
				return 1;
			}
		}
        return 0;
	}

    vector<char *> createProgramArguments(int &currentIndex, bool &isBackground){
        vector<char*> vec;
        int offset = 0;
        if(tokens.size()>currentIndex+1){
            if(tokens[currentIndex + 1]=="&"){
            transform(begin(tokens)+currentIndex, begin(tokens)+currentIndex+1,
            back_inserter(vec),
            [](string& s){ s.push_back(0); return &s[0]; });
                isBackground = true;
                currentIndex++;
            }
        }
        if(currentIndex < tokens.size()&& !isBackground) {
            transform(begin(tokens)+currentIndex, end(tokens),
            back_inserter(vec),
            [](string& s){ s.push_back(0); return &s[0]; });
        }

        vec.push_back(nullptr);
        char** carray = vec.data();
        currentIndex = tokens.size();
        return vec;
    }

    int tryExecuteForkCommand(int &currentIndex){
        bool isBackground = false;
        int status;

	vector<char *> newvec = createProgramArguments(currentIndex,isBackground);
        char** newARGV = newvec.data();
        pid_t pid = fork();
        if(pid < 0){
            printf("Fork Failed\n");
            exit(1);
        }
        if(pid == 0){
        cout<<"child running"<<endl;
            if (execvp(newARGV[0], newARGV) < 0){
            //if (execvp(tokens[currentIndex].c_str(), newARGV) < 0) {     
                printf("Child: ERROR: exec failed\n");
                exit(1);
            }
            cout<<"parent exiting "<<endl;
            exit(0);
        }
        else{
            if(!isBackground){
                waitpid(pid, &status, 0);       
                cout<<"parent running"<<endl;
            }
            else{
                cout<<" Child is running in the background";
            }
        }
        return 0;
    }

	void createToken(string rawCommand, char delim = ' ') {
		string token;
		stringstream ss(rawCommand);
		tokens.clear();
		while (getline(ss, token, delim)) {
			tokens.push_back(token);
            
		}
	}

    string createRawCommand(int argc, char** argv){
        string rawCommand = "";
        for(int i=1;i<argc;++i){
            rawCommand += argv[i];
            rawCommand += " ";
        }
        return rawCommand;
    }

	void print() {
		for (auto s : tokens) {
			cout << s << endl;
		}
	}
};

void sig_handler(int signum){
  signal(SIGINT,sig_handler);   // Re Register signal handler for default action
}

int main(int argc, char** argv, char** envp) {
//    signal(SIGINT,SIG_IGN);
    signal(SIGINT,sig_handler);
	CommandManager cmd;
	string raw_command;
    if(argc > 1){
        raw_command = cmd.createRawCommand(argc, argv);
        cmd.createToken(raw_command);
        bool status = cmd.executeCommand();
    }
    
    while (true) {
        cout << "\n> ";
        getline(cin, raw_command);
        cmd.createToken(raw_command);
        bool status = cmd.executeCommand();
    }
}