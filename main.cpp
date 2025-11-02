using namespace std;
#include <iostream>
#include <string>
#include <cstring> // For strtok
#include <sstream>
#include <vector>
#include <termios.h> // for raw mode(tcgetattr, tcsetattr, cfmakeraw)
#include <unistd.h>  // access to POSIX opsys API (read, write, fork, exec...)
#include <sys/wait.h>
#include <fstream> // for ofstream

struct termios orig_termios;
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
    string s;

    // opening file for storing
    ofstream myfile;
    myfile.open("store.txt");

    for (;;)
    {
        cout << "\nshreyashell> ";

        std::getline(cin, s);

        // storing
        myfile << s << endl;

        if (s == "exit")
            break;
        else if (s == "history")
        {
            string cmd;
            ifstream myread;
            myread.open("store.txt");

            while (getline(myread, cmd))
                cout << cmd << "\n";
        }
        else
        {
            // Tokenize the input
            istringstream iss(s);
            vector<string> tokens;
            string token;
            while (iss >> token)
            {
                tokens.push_back(token);
            }

            // Convert vector<string> to char* array
            vector<char *> args;
            for (auto &t : tokens)
            {
                args.push_back(&t[0]); // safe because t is stored in tokens
            }
            args.push_back(NULL); // execvp needs NULL termination

            pid_t pid = fork();
            if (pid == 0)
            {
                execvp(args[0], args.data());
                cout << "invalid command" << endl; // If exec fails
                exit(1);
            }
            else
            {
                wait(NULL);
            }
        }
    }
    myfile.close();
    // cout << s << endl;
    return 0;
}