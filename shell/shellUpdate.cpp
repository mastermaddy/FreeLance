#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <map>
#include <unistd.h>
#include<vector>
#include<algorithm>

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
            return getenv(str.substr(1).c_str());
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
            currentIndex++;
            return true;
        }
        return false;
	}


	static bool cmd_echo(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,-1)){
            for (auto it = tokens.begin() + currentIndex; it != tokens.end(); it++) {
                printf("\t var: %s\n", getEnvironmentVariableValue((*it)));
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
            currentIndex++;
            return true;
        }
		return false;
	}


	static bool cmd_setenv(int &currentIndex, vector<string> tokens) {
        if(isNumberOfArgumentsCorrect(tokens.size(),currentIndex,1)){
            const char* var_name = getEnvironmentVariableValue(tokens[currentIndex].c_str());
            const char* var_value = getEnvironmentVariableValue(tokens[currentIndex+1].c_str());
            //setenv(var_name, var_value, 1);
            currentIndex+=2;
            printf("%s: %s\n", var_name, var_value);
            _putenv_s(var_name, var_value);
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
        for(i=0;i<tokens.size();++i){
            if (tryExecuteBuiltInCommand(i)) {
                return true;
            }
            if(tryExecuteForkCommand(i)){
                return true;
            }
        }
        return false;
	}


	bool tryExecuteBuiltInCommand(int &currentIndex) {
		for (auto cmnd : commandMap) {
			if (cmnd.first == tokens[currentIndex]) {
                currentIndex += 1;
				return commandMap[cmnd.first](currentIndex, tokens);
			}
		}
        return true;
	}

    char** createProgramArguments(){
        vector<char*> vec;
        transform(begin(tokens), end(tokens),
                back_inserter(vec),
                [](string& s){ s.push_back(0); return &s[0]; });
        vec.push_back(nullptr);
        char** carray = vec.data();
        return carray;
    }

    bool tryExecuteForkCommand(int &currentIndex){
        char** newARGV = createProgramArguments();
        pid_t pid;// = fork();
        if (pid == 0) {          
            if (execvp(tokens[currentIndex].c_str(), newARGV) < 0) {     
                printf("*** ERROR: exec failed\n");
                exit(1);
            }
        }
        return true;
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

int main(int argc, char** argv, char** envp) {
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
