#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <map>
#include <unistd.h>
 
 
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
 // verify cd  output
    bool cmd_cd(vector<string> tokens) {
        if (tokens[1][0] == '$') {
            int ch = chdir(getenv(tokens[1].substr(1).c_str()));
            if (ch != 0) {
                printf("chdir change of directory not successful\n");
                return false;
            }
            else {
                return true;
            }
        }
        else {
            int ch = chdir(tokens[1].substr(0).c_str());
            if (ch !=0) {
                printf("chdir change of directory not successful\n");
                return false;
            }
            else {
                return true;
            }
        }
        return true;
    }
 
    bool cmd_pwd(vector<string> tokens) {

        puts("Path info by use environment variable PWD:");
        printf("\tWorkdir: %s\n", getenv("PWD"));
        printf("\tFilepath: %s/%s\n", getenv("PWD"), __FILE__);
        return true;
    }
 
    bool cmd_echo(vector<string> tokens) {
        for( auto it = tokens.begin()+1; it!= tokens.end(); it++){
            if ((*it)[0] == '$') {
                printf("\t var: %s\n", getenv((*it).substr(1).c_str()));
            }
            else {
                printf("\t var: %s\n",(*it).c_str());
            }
        }
        return true;
    }
 
    bool cmd_env(vector<string> tokens) {
    for (int i=0; environ[i]!=NULL; i++) {
        printf("%d: %s\n", i, environ[i]);
    }

        return true;
    }
 
    bool cmd_setenv(vector<string> tokens) {
        setenv(tokens[1].c_str(), tokens[2].c_str(), 1);
        printf("\tvariablename: %s\n", getenv(tokens[1].c_str()));
        printf("\tvariablevsal: %s\n", getenv(tokens[2].c_str()));

        return true;
    }
};

class CommandManager {
private:
    Commands command;
    vector<string> tokens;
public:
    CommandManager() {
    }
 
    bool executeCommand() {
        if (tryExecuteBuiltInCommand(tokens[0])) {
            return true;
        }
        return false;
    }
 
    bool tryExecuteBuiltInCommand(string cmdName) {
        if (cmdName == "cd") {
            return command.cmd_cd(tokens);
        }
        else if (cmdName == "pwd") {
            return command.cmd_pwd(tokens);
        }
        else if (cmdName == "echo") {
            return command.cmd_echo(tokens);
        }
        else if (cmdName == "env") {
            return command.cmd_env(tokens);
        }
        else if (cmdName == "setenv") {
            return command.cmd_setenv(tokens);
        }
        else {
            return false;
        }
    }
 
    void createToken(string rawCommand, char delim = ' ') {
        cout<<"RAW: "<<rawCommand<<endl;
        string token;
        stringstream ss(rawCommand);
        tokens.clear();
        while (getline(ss, token, delim)) {
            tokens.push_back(token);
        }
    }
 
    void print() {
        for (auto s : tokens) {
            cout << s << endl;
        }
    }
};
 
 
// int main(int argc, char** argv, char** envp)
// {
//     CommandManager cmd;
//     string raw_command;
//     while (true) {
//         cout << "\n> ";
//         getline(cin, raw_command);
//         cmd.createToken(raw_command);
//         bool status = cmd.executeCommand();
//         if (!status) {
//             break;
//         }
//         //cmd.print();
//     }
// }

// PART 2///////////////////////////////////////////////////////////////////
int main(int argc, char** argv, char** envp)
{
	CommandManager cmd;
	if (argc > 1) {
        string commandlist;
        for( int i = 1; i< argc; i++)
        {
            commandlist+= argv[i];
            if(i<argc)
                commandlist+= " ";
        }
        cout<<"Commandlist: "<<commandlist<<endl;
        cout<<"1"<<endl;
		pid_t pid = fork();
        cout<<"//////////////////////////////////////////////////////////////////////////";
        cout<<"2"<<endl;
		cmd.createToken(commandlist);
        cout<<"3"<<endl;
		bool status = cmd.executeCommand();

        // cmd.createToken("env");
        // cout<<"3"<<endl;
	    // status = cmd.executeCommand();

        if (pid == 0) {          
            if (execvp("./temp", argv) < 0) {     
                printf("*** ERROR: exec failed\n");
                exit(1);
            }
        }
        
          cout<<"4"<<endl;
	}
}
