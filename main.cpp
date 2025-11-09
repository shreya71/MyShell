using namespace std;
#include <iostream>
#include <cstdlib> // for getencv
#include <string>
#include <cstring> // For strtok, strcmp
#include <sstream>
#include <vector>
#include <termios.h> // for raw mode(tcgetattr, tcsetattr, cfmakeraw)
#include <unistd.h>  // access to POSIX opsys API (read, write, fork, exec, chdir, getcwd...)
#include <sys/wait.h>
#include <fstream> // for ofstream
#include <ctype.h> // for iscntrl
#include <filesystem> // for path manipulation and validation
#include <limits.h> //for PATH_MAX

struct termios orig_termios;
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;// to turn of printing... typing on terminal
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void changeDirectory(const string &path) {
    const char *targetPath;

    if(path.empty() || path == "~") {
        targetPath = getenv("HOME");
    }
    else {
        targetPath = path.c_str(); // pointer to array of characters
    }

    if (chdir(targetPath) != 0)
    {
        perror("cd failed");
        return;
    }

    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != nullptr){
        cout << "Changed directory to: " << cwd << endl;
    } else{
        perror("getcwd() error");
    }
}

int main()
{
    enableRawMode();
    char c;
    string path;

    // opening file for storing
    ofstream myfile;
    myfile.open("store.txt");

    for (;;)
    {
        string s = "";
        cout << "shreyashell> " << flush;

        while (read(STDIN_FILENO, &c, 1))
        { // Read one character at a time, exit on 'q'
            if (c == '\n')
            {
                cout << '\n';
                break;
            }
            if (iscntrl(c))
            {
                char s[10];
                if (read(STDIN_FILENO, s, 3))
                {
                    if (s[1] == 'A')
                        cout << "Up Arrow Pressed\n" ;
                    else if (s[1] == 'B')
                        cout << "Down Arrow Pressed\n";
                    else if (s[1] == 'C')
                        cout << "Right Arrow Pressed\n";
                    else if (s[1] == 'D')
                        cout << "Left Arrow Pressed\n";
                    else if (s[0] == 'b')
                        cout << s[0] << s[1] << "\n";
                    break;
                    // else
                    //     cout << s[1] << "\n";
                }
                else
                    cout << "Read failed\n";
            }
            else
            {
                //raw.c_lflag |= ECHO;
                s += c;
                cout << c << flush;
            }
            // printf("%c", c);
            // fflush();
        }

        // storing
        myfile << s << endl;

        if (s == "exit")
            {
            atexit(disableRawMode);
            break;
        }
        else if (s == "history")
        {
            string cmd;
            ifstream myread;
            myread.open("store.txt");

            while (getline(myread, cmd))
                cout << cmd << "\n";
        }
        else if (s.rfind("cd",0) == 0)
        {
            path = s.substr(2);
            path.erase(0, path.find_first_not_of(" \t"));

            changeDirectory(path);
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
                cout << "invalid command\n"; // If exec fails
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