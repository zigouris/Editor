#include <ncurses.h>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <map>
#include <functional>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <cstdio>

using namespace std;
namespace fs = std::filesystem;

enum class EditorMode {
    Normal,
    Insert,
    Command,
};

void cursor_set_color_string(const char *color) {
    printf("\e]12;%s\a", color);
    fflush(stdout);
}

void scrollIfNeeded(int &scrollOffset, int currentLine, int totalLines) {
    if (currentLine - scrollOffset < 5) {
        scrollOffset = max(0, currentLine - 5);
    } else if (LINES - currentLine + scrollOffset < 5) {
        scrollOffset = max(0, currentLine + 5 - LINES);
    }
}

int calculatePercentage(int currentLine, int totalLines) {
    if (totalLines == 0) return 100;
    return static_cast<int>(round((static_cast<double>(currentLine) / (totalLines - 1)) * 100));
}

bool saved = false;
bool error = false;
bool show_warn = false;
bool changed = false;
bool dont_go_in = false;
string buff;
bool show_warning_temp = false;
bool QUIT = false;
bool is_file = false;
bool is_dir = false;
void quit_FUNC() {
    if (!saved && changed && !QUIT) {
        show_warn = true;
    } else {
        clear();
        endwin();
        exit(0);
    }
}

void write_FUNC(const char *filename, vector<string> &lines) {
    ofstream outFile(filename);
    for (const string &line : lines) {
        outFile << line << endl;
    }
    outFile.close();
    saved = true;
    changed = false;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(2);
    cursor_set_color_string("yellow");
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);     // Warn
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Warn
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // ~
    init_pair(4, COLOR_BLACK, COLOR_BLACK);   // Dark gray for line numbers
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);  // Yellow for current line number
    string FILENAME = "";
    start:
    is_dir = false;
    is_file = false;

    vector<string> lines;
    ifstream file(argv[1]);
    
    if (argv[1] == ".")
	    is_dir = true;
    
    fs::path fullPath = fs::current_path() / argv[1];
    if (fs::exists(fullPath) && is_dir != true && is_file != true) {
    	if (fs::is_regular_file(fullPath)) {
	       is_file = true;
	   }
	
        else if (fs::is_directory(fullPath)) {
	       is_dir = true;
	    }

	    else {
 	       is_file = true;
	    } 
    }
    
    if (is_dir) {
        while (true) {
            is_dir = false;
            fs::current_path(fullPath);
            FILE* pipa = popen("ls", "r");
            vector<string> linesls;
            char buffer2[128];
            while (fgets(buffer2, sizeof(buffer2), pipa) != nullptr) {
                string line = buffer2;
                line.erase(line.find_last_not_of("\n") + 1);
                linesls.push_back(line);
            }
            pclose(pipa);
            FILE* pipe = popen("ls -l", "r");
            vector<string> linesFromLS;
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                string line = buffer;
                line.erase(line.find_last_not_of("\n") + 1);
                linesFromLS.push_back(line);
            }
            pclose(pipe);

            int selectedLine = 0;
            int scrollOffset = 0;

            while (true) {
                nigger:
                clear();
                refresh();

                int firstVisibleLine = scrollOffset;
                int lastVisibleLine = scrollOffset + LINES - 1;
                bool dont_go_in2 = false;
                for (int i = firstVisibleLine; i <= lastVisibleLine - 4; ++i) {
                    if (i == firstVisibleLine && selectedLine < 10) {
                        printw("");
                    }
                    int lineNumber = i - selectedLine;

                    if (i >= 0 && i < linesFromLS.size()) {
                        if (lineNumber == 0) {
                            attron(COLOR_PAIR(5) | A_BOLD);
                        } else {
                            attron(COLOR_PAIR(4) | A_BOLD);
                        }

                        int spaces = max(0, 2 - int(log10(abs(lineNumber + 1))));
                        for (int j = 0; j < spaces / 2; ++j) {
                            
                        }
                        if (lineNumber == 0 && selectedLine < 100) printw("%d", selectedLine+1);
                        else if (lineNumber == 0 && selectedLine + 1 > 9){ printw("%d",lineNumber); dont_go_in2 = true;}
                        else {
                            printw("%d", abs(lineNumber + 0));
                        }
                        if(abs(lineNumber > 9)) {
                            if (!dont_go_in2) {
                                attroff(COLOR_PAIR(4));
                                printw(" %s\n", linesFromLS[i].c_str());
                                attron(COLOR_PAIR(4));   
                            }
                            else {
                                dont_go_in2 = false;
                                attroff(COLOR_PAIR(4));
                                printw(" %s\n", linesFromLS[i].c_str());
                                attron(COLOR_PAIR(4));
                            }                        
                        }
                        else {
                            if (!dont_go_in2) {
                                attroff(COLOR_PAIR(4));
                                printw(" %s\n", linesFromLS[i].c_str());
                                attron(COLOR_PAIR(4));
                            }
                            else {
                                dont_go_in2 = false;
                                attroff(COLOR_PAIR(4));
                                printw(" %s\n", linesFromLS[i].c_str());
                                attron(COLOR_PAIR(4));
                            }
                            
                        }

                        if (lineNumber == 0) {
                            attroff(COLOR_PAIR(5) | A_BOLD);
                        } else {
                            attroff(COLOR_PAIR(4) | A_BOLD);
                        }
                    }
                }

                int ch = getch();

                if (ch == KEY_DOWN) {
                    if (selectedLine < linesFromLS.size() - 1) {
                        selectedLine++;
                        scrollIfNeeded(scrollOffset, selectedLine - 2, linesFromLS.size());
                    }
                } else if (ch == KEY_UP) {
                    if (selectedLine > 0) {
                        selectedLine--;
                        scrollIfNeeded(scrollOffset, selectedLine, linesFromLS.size());
                    }
                } else if (ch == '\n') {
                    fs::path newPath = fullPath / linesls[selectedLine-1];
                    if (is_directory(newPath)) {
                        is_dir = true;
                        fs::current_path(newPath);
                        clear();
                        refresh();
                        clrtoeol();
                        refresh();
                        goto start;
                    }
                    else if(is_regular_file(newPath)) {
                        is_file = true;
                        FILENAME = linesls[selectedLine - 1];
                        ifstream file(FILENAME);
                        goto startfile;
                    }
                }
                 else if (ch == 27) {
                    break;
                }

                refresh();
            }
        }
    }


    startfile:
    if (FILENAME == "") {
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        } else {
            lines.push_back("");
        }
    }
    else {
        ifstream tempFile(FILENAME);
        if (tempFile.is_open()) {
            string line;
            while (getline(tempFile, line)) {
                lines.push_back(line);
            }
            tempFile.close();
        } else {
            lines.push_back("");
        }
    }

    int currentLine = 0;
    int cursorX = 0;
    EditorMode mode = EditorMode::Normal;
    int scrollOffset = 0;
    string commandBuffer;
    map<string, function<void()>> cmds;

    cmds["q"] = quit_FUNC;
    cmds["w"] = [&]() {
        write_FUNC(argv[1], lines);
    };


    while (true) {
        clear();

        int firstVisibleLine = scrollOffset;
        int lastVisibleLine = scrollOffset + LINES - 1;

        for (int i = firstVisibleLine; i <= lastVisibleLine - 4; ++i) {
            int lineNumber = i - currentLine;
            if (i >= 0 && i < lines.size()) {
                if (lineNumber == 0) {
                    attron(COLOR_PAIR(5) | A_BOLD);  // yellow for current line number
                } else {
                    attron(COLOR_PAIR(4) | A_BOLD);  // dark gray for other line numbers
                }
                int spaces = max(0, 2 - int(log10(abs(lineNumber))));
                for (int j = 0; j < spaces / 2; ++j) {
                    if (abs(lineNumber) < 10 && lineNumber != 0) printw(" ");
                }
                if (lineNumber == 0 && currentLine + 1 < 10) printw(" %d", currentLine + 1);
                else if (lineNumber == 0 && currentLine + 1 >= 10) printw("%d", currentLine + 1);
                else {
                    printw("%d", abs(lineNumber));
                }
                if (lineNumber == 0) {
                    attroff(COLOR_PAIR(5) | A_BOLD);
                } else {
                    attroff(COLOR_PAIR(4) | A_BOLD);
                }
                printw(" %s\n", lines[i].c_str());
            } else {
                attron(COLOR_PAIR(3));
                printw("~\n");
                attroff(COLOR_PAIR(3));
            }
        }

        int totalLines = lines.size();
        int totalChars = (currentLine < totalLines) ? lines[currentLine].size() : 0;
        int rightX = COLS - 1;
        int percentage = calculatePercentage(currentLine, totalLines);

        mvprintw(LINES - 1, rightX - 17, "%4d,%d", currentLine + 1, cursorX + 1);
        mvprintw(LINES - 1, rightX - 2, "%d%%", percentage);

        move(LINES - 1, 0);

        if (mode == EditorMode::Normal) {
            int Nlines = lines.size();
            ifstream filee(FILENAME, ios::binary | ios::ate);
            streampos fileSize = filee.tellg();
            printw("\"%s\" %dL, %dB", FILENAME, Nlines, fileSize);
        } else if (mode == EditorMode::Insert) {
            attron(A_BOLD);
            printw("-- INSERT --");
            attroff(A_BOLD);
        } else if (mode == EditorMode::Command) {
            printw(":%s", commandBuffer.c_str());
        }

        if (saved) {
            printw(" written");
        }

        if (error) {
            move(LINES - 1, 0);
            clrtoeol();
            attron(COLOR_PAIR(1));
            printw("Not an editor command: %s", buff.c_str());
            attroff(COLOR_PAIR(1));
            refresh();
            error = false;
            mode = EditorMode::Normal;
        }

        if (show_warn) {
            clear();
            move(0, 0);

            attron(COLOR_PAIR(3));
            for (int i = lines.size(); i < LINES - 1; i++) {
                printw("~\n");
            }
            attroff(COLOR_PAIR(3));

            move(LINES - 2, 0);
            attron(COLOR_PAIR(1));
            printw("No write since last change (add ! to override)");
            attroff(COLOR_PAIR(1));

            move(LINES - 1, 0);
            attron(COLOR_PAIR(5) | A_BOLD);
            printw("Press ENTER or type command to continue");
            attroff(COLOR_PAIR(5) | A_BOLD);
            curs_set(0);
            refresh();
            show_warn = false;
            mode = EditorMode::Normal;
        }

        move(currentLine - scrollOffset, cursorX + 3);

        int ch = getch();
        curs_set(2);

        if (show_warning_temp) {
            if (ch == '\n') {
                show_warning_temp = false;
            }
            continue;
        }

        if (mode == EditorMode::Normal) {
            if (ch == ':') {
                mode = EditorMode::Command;
                commandBuffer.clear();
                saved = false;
                error = false;
            } else if (ch == 'I' || ch == 'i') {
                mode = EditorMode::Insert;
                saved = false;
                error = false;
            } else if (ch == KEY_DOWN) {
                if (currentLine < totalLines - 1) {
                    currentLine++;
                    cursorX = min(cursorX, static_cast<int>(lines[currentLine].size()));
                    scrollIfNeeded(scrollOffset, currentLine - 2, totalLines);
                }
            } else if (ch == KEY_UP) {
                if (currentLine > 0) {
                    currentLine--;
                    cursorX = min(cursorX, static_cast<int>(lines[currentLine].size()));
                    scrollIfNeeded(scrollOffset, currentLine, totalLines);
                }
            } else if (ch == KEY_LEFT && cursorX > 0) {
                cursorX--;
            } else if (ch == KEY_RIGHT && cursorX < totalChars) {
                cursorX++;
            } else if (ch == 'G') {
                currentLine = totalLines - 1;
                cursorX = 0;
                scrollOffset = max(0, currentLine - (LINES - 1));
            } else if (ch == 'g') {
                int nextCh = getch();
                if (nextCh == 'g') {
                    currentLine = 0;
                    cursorX = 0;
                    scrollOffset = 0;
                } else {
                    buff = commandBuffer;
                }
            } else {
                buff = commandBuffer;
            }
        } else if (mode == EditorMode::Insert) {
            if (ch == 27) {
                mode = EditorMode::Normal;
            } else if (isprint(ch)) {
                if (currentLine < totalLines) {
                    lines[currentLine].insert(cursorX, 1, ch);
                    cursorX++;
                    changed = true;
                }
            } else if (ch == KEY_DOWN) {
                if (currentLine < totalLines - 1) {
                    currentLine++;
                    cursorX = min(cursorX, static_cast<int>(lines[currentLine].size()));
                    scrollIfNeeded(scrollOffset, currentLine - 2, totalLines);
                }
            } else if (ch == KEY_UP) {
                if (currentLine > 0) {
                    currentLine--;
                    cursorX = min(cursorX, static_cast<int>(lines[currentLine].size()));
                    scrollIfNeeded(scrollOffset, currentLine, totalLines);
                }
            } else if (ch == KEY_LEFT && cursorX > 0) {
                cursorX--;
            } else if (ch == KEY_RIGHT && cursorX < totalChars) {
                cursorX++;
            } else if (ch == '\n') {
                if (currentLine < totalLines) {
                    if (cursorX == lines[currentLine].size()) {
                        lines.insert(lines.begin() + currentLine + 1, "");
                        currentLine++;
                        cursorX = 0;
                    } else {
                        string &currentLineText = lines[currentLine];
                        string newLineText = currentLineText.substr(cursorX);
                        currentLineText.erase(cursorX);
                        lines.insert(lines.begin() + currentLine + 1, newLineText);
                        currentLine++;
                        cursorX = 0;
                    }
                    changed = true;
                    scrollIfNeeded(scrollOffset, currentLine, totalLines);
                }
            } else if (ch == KEY_DC) {
                if (mode == EditorMode::Insert) {
                    if (currentLine < totalLines) {
                        if (cursorX < lines[currentLine].size()) {
                            lines[currentLine].erase(cursorX, 1);
                            changed = true;
                        } else if (currentLine < totalLines - 1) {
                            lines[currentLine] += lines[currentLine + 1];
                            lines.erase(lines.begin() + currentLine + 1);
                            totalLines--;
                            changed = true;
                        }
                    }
                }
            } else if (ch == KEY_BACKSPACE || ch == 8) {
                if (mode == EditorMode::Insert) {
                    if (currentLine < totalLines) {
                        if (cursorX > 0) {
                            lines[currentLine].erase(cursorX - 1, 1);
                            cursorX--;
                            changed = true;
                        } else if (currentLine > 0) {
                            cursorX = lines[currentLine - 1].length();
                            lines[currentLine - 1] += lines[currentLine];
                            lines.erase(lines.begin() + currentLine);
                            currentLine--;
                            changed = true;
                        }
                    }
                }
            }
        } 
        if (mode == EditorMode::Command) {
            if (ch == 27) {
                mode = EditorMode::Normal;
            } else if (ch == '\n') {
                if (!commandBuffer.empty()) {
                    bool allCommandsValid = true;
                    for (int i = 0; i < commandBuffer.length(); ++i) {
                        string singleCommand = commandBuffer.substr(i, 1);
                        auto commandB = cmds.find(singleCommand);
                        if (commandB == cmds.end() && singleCommand != "!") {
                            allCommandsValid = false;
                            buff = commandBuffer;
                            error = true;
                            break;
                        }
                    }

                    if (allCommandsValid) {
                        for (int i = 0; i < commandBuffer.length(); ++i) {
                            string singleCommand = commandBuffer.substr(i, 1);
                            auto commandB = cmds.find(singleCommand);
                            if (commandB != cmds.end()) {
                                if (commandBuffer.substr(i + 1, 1) == "!") {
                                    QUIT = true;
                                }
                                commandB->second(); // calling function associated with the command in the map
                            }
                        }
                    }
                }
                mode = EditorMode::Normal;
            } else if (isprint(ch) && ch != ':') {
                commandBuffer += ch;
            } else if (ch == KEY_BACKSPACE || ch == 8) {
                if (!commandBuffer.empty()) {
                    commandBuffer.pop_back();
                }
            }
        } else {
            if (ch == ':') {
                if (commandBuffer.empty() || commandBuffer.back() != ':') {
                    commandBuffer += ch;
                }
            } else if (isprint(ch)) {
                commandBuffer += ch;
            }
        }

        scrollIfNeeded(scrollOffset, currentLine, totalLines);
    }

    endwin();
    
    return 0;
}
