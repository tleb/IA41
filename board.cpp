#include "board.h"
#include "game.h"
#include <cmath>
#include <cstring>
#include "prolog.h"

extern Game* game;

Board::Board() {
    numbertomove = -1;
    posnumber = -1;
    postomovenumber = -1;
}

QList<Pawn *> * Board::getPawns() {
    return pawns;
}

void Board::createBoard() {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            Pawn* pawn = new Pawn();
            pawn->setPos(pos[i], pos[j]);
            connect(pawn, &Pawn::clicked, this, [=](){ this->showPawnStackOrMove(pawn); });
            game->scene->addItem(pawn);
        }
    }
}

void Board::initialiseBoard() {
    createBoard();

    //Add Player 2 Pawns
    for(int n = 0; n < 2; n++) {
        for(int i = 0; i < 3; i++) {
            Pawn* pawn = new Pawn(NULL, 2);
            pawns[i].append(pawn);
            pawn->setPos(pos[i], pos[0]);
            connect(pawn, &Pawn::clicked, this, [=](){ this->showPawnStackOrMove(pawn); });
            game->scene->addItem(pawn);
        }
    }

    //Add Player 1 Pawns
    for(int n = 0; n < 2; n++) {
        for(int i = 0; i < 3; i++) {
            Pawn* pawn = new Pawn(NULL, 1);
            pawns[6+i].append(pawn);
            pawn->setPos(pos[i], pos[2]);
            connect(pawn, &Pawn::clicked, this, [=](){ this->showPawnStackOrMove(pawn); });
            game->scene->addItem(pawn);
        }
    }
}

void Board::showPawnStackOrMove(Pawn* selectedpawn) {
    //Either we first select a pawn to see the pawn stack
    if (numbertomove < 0) {
        //Clear already shown pawns
        clearShownStack();

        posnumber = findposnumber(selectedpawn->pos());
        for(int i = 0; i < pawns[posnumber].size(); i++) {
            //add pawns
            Pawn* pawn = new Pawn(NULL, pawns[posnumber].at(i)->getOwner());
            connect(pawn, &Pawn::clicked, this, [=](){ this->movePawnStack(pawn); });
            stackedpawns.append(pawn);
            pawn->setPos(870, i*120+30);
            game->scene->addItem(pawn);
        }

        int i = 0;
        for (int i = 0; i < pawns[posnumber].size() && i < 3; i++) {
            //add move numbers next to them
            QGraphicsTextItem* text = new QGraphicsTextItem(QString("Move %1").arg(i+1));
            text->setPos(800, i*120+70);
            numberToMoveText.append(text);
            game->scene->addItem(text);
        }
    } else {
        //or we already have selected the pawns we want to move and we click on pawns to select the place we want to move them
        postomovenumber = findposnumber(selectedpawn->pos());
        if(isMovePossible() && !game->getIaRunning()) {
            for(int i = 0; i < numbertomove ; i++) {
                pawns[posnumber][numbertomove-1-i]->setPos(selectedpawn->pos().x(), selectedpawn->pos().y());
                pawns[postomovenumber].prepend(pawns[posnumber][numbertomove-1-i]);
                pawns[posnumber].removeAt(numbertomove-1-i);
            }
            clearShownStack();
            if (!isGameFinished()) {
                game->nextPlayersTurn();
            }
        } else {
            clearShownStack();
        }
    }
}

bool Board::isMovePossible() {
    if (pawns[posnumber][0]->getOwner() != game->getWhosTurn() && game->getRealplayer() != 0) {
        showError(QString("You can't move this Pawn Stack"));
        return 0;
    } else {
        int posnumberx = posnumber%3;
        int posnumbery = posnumber/3;
        int postomovenumberx = postomovenumber%3;
        int postomovenumbery = postomovenumber/3;
        int xmove = abs(posnumberx - postomovenumberx);
        int ymove = abs(posnumbery - postomovenumbery);
        if((xmove + ymove) != numbertomove) {
            showError( QString("You should move %1 times").arg(numbertomove) );
            return 0;
        } else {
            return 1;
        }
    }
}

void Board::clearShownStack() {
    for(int i = 0; i < stackedpawns.size(); i++) {
        game->scene->removeItem(stackedpawns[i]);
    }
    for (int i = 0; i < stackedpawns.size() && i < 3; i++) {
        game->scene->removeItem(numberToMoveText[i]);
    }
    stackedpawns.clear();
    numberToMoveText.clear();
    posnumber=-1;
    postomovenumber=-1;
    numbertomove=-1;
}

int Board::findposnumber(QPointF coordinates) {
    int positionnumber = 0;

    int i = 0;
    while(coordinates.x() != pos[i]) {
        i++;
    }
    positionnumber = i;

    i=0;
    while(coordinates.y() != pos[i]) {
        i++;
    }
    positionnumber+=3*i;

    return positionnumber;
}

void Board::movePawnStack(Pawn* selectedpawn) {
    numbertomove = findnumbertomove(selectedpawn->pos());
}

int Board::findnumbertomove(QPointF coordinates) {
    int i = 0;
    while(coordinates.y() != 30+(120*i) && i < 2) {
        i++;
    }
    return i+1;
}

void Board::showError(QString text) {
    clearShownStack();
    errorButton = new Button(text);
    errorButton->setPos(480, 350);
    connect(errorButton, &Button::clicked, this, &Board::hideError);
    game->scene->addItem(errorButton);
}

void Board::hideError() {
    game->scene->removeItem(errorButton);
    delete errorButton;
}

struct State Board::pawnsToState() {
    struct State state;
    memset(&state, 0, sizeof(state));

    state.nextPlayer = game->getWhosTurn() == 1 ? white : black;

    for(int i = 0; i < 9 ; i++) {
        for (int j = 0; j < pawns[i].size(); j++) {
            state.towers[i][j] = pawns[i][j]->getOwner() == 1 ? white : black;
        }
    }

    return state;
}

void Board::stateToPawns(struct State state) {
    for(int i = 0; i < 9; i++) {
        for(int j = 0; j < pawns[i].size(); j++) {
            game->scene->removeItem(pawns[i][j]);
        }
        pawns[i].clear();
    }

    for(int i = 0; i < 9; i++) {
        for (int j = 8; j >= 0; j--) {
            if (state.towers[i][j] == none) {
                continue;
            }

            int owner = state.towers[i][j] == white ? 1 : 2;
            Pawn *pawn = new Pawn(NULL, owner);

            pawns[i].prepend(pawn);
            pawn->setPos(pos[i%3], pos[i/3]);
            connect(pawn, &Pawn::clicked, this, [=](){ this->showPawnStackOrMove(pawn); });
            game->scene->addItem(pawn);
        }
    }
}

bool Board::isGameFinished() {
    int nbOccupiedPositions = 0;
    int nbPlayerOnePositions = 0;
    int nbPlayerTwoPositions = 0;

    for (int i = 0; i < 9; i++) {
        if (!pawns[i].isEmpty()) {
            if (pawns[i][0]->getOwner() == 1) {
                nbPlayerOnePositions++;
                nbOccupiedPositions++;
            } else {
                nbPlayerTwoPositions++;
                nbOccupiedPositions++;
            }
        }
    }

    if(nbOccupiedPositions == nbPlayerOnePositions) {
        game->gameFinished(1);
        return true;
    }

    if(nbOccupiedPositions == nbPlayerTwoPositions) {
        game->gameFinished(2);
        return true;
    }

    return false;
}
