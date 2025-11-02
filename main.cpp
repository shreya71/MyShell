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
#include <ctype.h> // for iscntrl

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

int main()
{
    enableRawMode();
    char c;

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