#include <QApplication>

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <QObject>

#include "game.h"
#include "prolog.h"

// Assume Windows and assume SWI-Prolog is in the path...
#define SWI_PROLOG_PATH "swipl.exe"

Game* game;
Board* board;

int main(int argc, char *argv[]) {
    srand(time(NULL));
    QApplication a(argc, argv);

    PrologMinMax *ia = new PrologMinMax("swipl.exe");

    game = new Game(ia);
    game->show();
    game->displayMainMenu();

    QObject::connect(&a, SIGNAL(aboutToQuit()), game, SLOT(closing()));

    return a.exec();
}
