#ifndef PROLOG_H
#define PROLOG_H

#include <string>
#include <windows.h>
#include <QThread>
#include <QObject>

// to compile:
// swipl.exe --stand_alone=true -g main -o exec -c main.pl
// to run:
// ./exec DEPTH "STATE"

enum Player { none, white, black };

struct State {
    enum Player nextPlayer;
    enum Player towers[9][12];
};

class PrologMinMax {
private:
    std::string execPath;
public:
    PrologMinMax(const char *swiplCommand);
    ~PrologMinMax(void);
    struct State run(struct State initial);
    void deleteExec(void);
};

std::string runChildProcess(char *command);
struct State stringToState(std::string s);
bool parseTower(std::string s, int *pos, enum Player *tower);
std::string stateToString(struct State state);
const char* playerToString(enum Player p);
enum Player stringToPlayer(std::string s, int *pos);

#endif // PROLOG_H
