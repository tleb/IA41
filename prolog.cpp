#include <stdio.h>
#include <string>
#include <Windows.h>
#include <cstdio>
#include <stdexcept>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include "prolog.h"

#define PROLOG_FILE_NAME "main.pl"
#define STDOUT_BUFFER_SIZE 256
#define DEPTH 4

HANDLE childStdoutRead = NULL;
HANDLE childStdoutWrite = NULL;

/**
 * PrologMinMax constructor
 * @param swiplCommand path to the SWI-Prolog executable used to compile the Prolog file
 */
PrologMinMax::PrologMinMax(const char *swiplCommand) {
    char tmpFile[MAX_PATH];
    char buildCommand[MAX_PATH];

    unsigned int ret = GetTempFileNameA(".", "tmp", 0, tmpFile);
    if (ret == 0) {
        throw std::runtime_error("failed creating a temporary file");
    }
    this->execPath.assign(tmpFile);
    std::string prologPath = std::string(__FILE__).substr(0, std::string(__FILE__).rfind("\\"));
    prologPath.append("\\");
    prologPath.append(PROLOG_FILE_NAME);
    snprintf(buildCommand, MAX_PATH*3, "%s --stand_alone=true -g main -o %s -c %s", swiplCommand, tmpFile, prologPath.c_str());
    int status = system(buildCommand);

    if (status != 0) {
        throw std::runtime_error("Prolog compilation failed");
    }
}

PrologMinMax::~PrologMinMax(void) {
    std::remove(this->execPath.c_str());
}

/**
 * Run the MinMax IA for the given state.
 * It takes state.nextPlayer as the player who should win.
 * Blocking function.
 * See DEPTH.
 */
struct State PrologMinMax::run(struct State initial) {
    char cmd[1024];
    snprintf(cmd, 1024, "%s %d \"%s\"", this->execPath.c_str(), DEPTH, stateToString(initial).c_str());

    return stringToState(runChildProcess(cmd));
}

/**
 * Runs the given command and blocks until something can be read from its stdout. It returns what was read.
 */
std::string runChildProcess(char *command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    BOOL ret;

    // === Create the stdout pipe ===

    // make pipe handles be inherited
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = FALSE;

    if (!CreatePipe(&childStdoutRead, &childStdoutWrite, &sa, 0)) {
        throw std::runtime_error("CreatePipe() failed on child stdout");
    }

    if (!SetHandleInformation(childStdoutRead, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("SetHandleInformation() failed on child stdout");
    }

    // === Spawn the child process ===

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = childStdoutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    ret = ::CreateProcessA(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (!ret) {
        throw std::runtime_error("CreateProcess() failed");
    }

    // === Close unused handles in the parent process ===

    CloseHandle(childStdoutWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // === Read stdout from child ===

    char buffer[STDOUT_BUFFER_SIZE];
    unsigned long read;
    ZeroMemory(buffer, STDOUT_BUFFER_SIZE);

    ret = ReadFile(childStdoutRead, buffer, STDOUT_BUFFER_SIZE-1, &read, NULL);
    if (!ret) {
        throw std::runtime_error("ReadFile() failed");
    }

    return std::string(buffer);
}

struct State stringToState(std::string s) {
    struct State res = {};
    res.nextPlayer = none;
    int pos = 0;

    // === Parse the next player ===

    if (s[pos++] != '[') {
        std::cerr << "state array not opened" << std::endl;
        return res;
    }

    res.nextPlayer = stringToPlayer(s, &pos);
    if (res.nextPlayer == none) {
        std::cerr << "invalid next player" << std::endl;
        return res;
    }

    if (s[pos++] != ',') {
        std::cerr << "missing first comma" << std::endl;
        res.nextPlayer = none;
        return res;
    }

    // === Parse the tower array ===

    if (s[pos++] != '[') {
        std::cerr << "missing opening towers array bracket" << std::endl;
        res.nextPlayer = none;
        return res;
    }

    int blackPawns = 0;
    int whitePawns = 0;

    for (int i = 0; i < 9; i++) {
        // skip commas between towers
        if (i != 0 && s[pos++] != ',') {
            std::cerr << "missing comma between two towers" << std::endl;
            res.nextPlayer = none;
            return res;
        }

        // parse the current tower
        if (!parseTower(s, &pos, res.towers[i])) {
            std::cerr << "invalid tower, parsing failed" << std::endl;
            res.nextPlayer = none;
            return res;
        }

        // count the number of pawns in the current tower
        // and check once we have one none pawn, we don't get any black or white pawn
        bool seenNonePawn = false;
        for (int j = 0; j < 12; j++) {
            if (seenNonePawn && res.towers[i][j] != none) {
                std::cerr << "black or white pawn after a none pawn" << std::endl;
                res.nextPlayer = none;
                return res;
            }

            if (res.towers[i][j] == black) {
                blackPawns++;
            } else if (res.towers[i][j] == white) {
                whitePawns++;
            } else {
                seenNonePawn = true;
            }
        }
    }

    // check the count is right
    if (blackPawns != 6 || whitePawns != 6) {
        std::cerr << "invalid pawn count" << std::endl;
        res.nextPlayer = none;
        return res;
    }

    if (s[pos++] != ']') {
        std::cerr << "towers array not closed" << std::endl;
        res.nextPlayer = none;
        return res;
    }

    if (s[pos++] != ']') {
        std::cerr << "state array not closed" << std::endl;
        res.nextPlayer = none;
        return res;
    }

    return res;
}

/**
 * Used internally in stringToState.
 * It parses the tower at position pos in s and puts it into tower.
 * It returns whether it was successful.
 */
bool parseTower(std::string s, int *pos, enum Player *tower) {
    if (s[*pos] != '[') {
        std::cerr << "tower array not opened" << std::endl;
        return false;
    }
    (*pos)++;

    int playerCount = 0;
    while (1) {
        enum Player p = stringToPlayer(s, pos);
        if (p == none) {
            // we are done if the next character is a closing bracket
            return s[(*pos)++] == ']';
        }

        if (playerCount == 12) {
            std::cerr << "tower is too big" << std::endl;
            return false;
        }
        tower[playerCount++] = p;

        // skip commas
        if (s[*pos] == ',') {
            (*pos)++;
        }
    }
}

std::string stateToString(struct State state) {
    std::string res;
    res.reserve(256);

    res += '[';
    res += playerToString(state.nextPlayer);
    res += ",[";

    for (int i = 0; i < 9; i++) {
        if (i != 0) {
            res += ',';
        }
        res += '[';
        for (int j = 0; j < 12 && state.towers[i][j] != none; j++) {
            if (j != 0) {
                res += ',';
            }
            res += playerToString(state.towers[i][j]);
        }
        res += ']';
    }

    res += "]]";

    return res;
}

/**
 * Used internally in stateToString.
 * Turns a Player into a corresponding string, throwing an exeception if p is none.
 */
const char* playerToString(enum Player p) {
    if (p == white) {
        return "pw";
    } else if (p == black) {
        return "pb";
    } else {
        throw std::runtime_error("Trying to convert invalid player into string");
    }
}

/**
 * Used internally in stringToState.
 * It takes a string s and a position pos and returns what is written at this position.
 * It moves pos forward.
 */
enum Player stringToPlayer(std::string s, int *pos) {
    if (s[*pos] != 'p') {
        return none;
    }
    (*pos)++;

    if (s[*pos] == 'w') {
        (*pos)++;
        return white;
    } else if (s[*pos] == 'b') {
        (*pos)++;
        return black;
    } else {
        return none;
    }
}
