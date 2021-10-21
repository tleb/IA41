#ifndef BOARD_H
#define BOARD_H

#include <QList>
#include <QObject>
#include "pawn.h"
#include "button.h"
#include "prolog.h"
#include <QGraphicsTextItem>

class Board : public QObject
{
    Q_OBJECT
protected:
    QList<Pawn*> pawns[9];
    int pos[3] = {170,330,490};
    int posnumber;
    int postomovenumber;
    int numbertomove;
    QList<Pawn*> stackedpawns;
    QList<Pawn*> ListToMove;
    QList<QGraphicsTextItem*> numberToMoveText;
    Button* errorButton;
public:
    Board();
    QList<Pawn*> * getPawns();
    void createBoard();
    void initialiseBoard();
    void showPawnStackOrMove(Pawn* );
    int findposnumber(QPointF coordinates);
    void hidePawnStack();
    int findnumbertomove(QPointF coordinates);
    void movePawnStack(Pawn* );
    bool isMovePossible();
    QPointF findposnumber(int positionnumber);
    void clearShownStack();
    void showError(QString text);
    void hideError();
    void stateToPawns(struct State);
    struct State pawnsToState();
    bool isGameFinished(void);
};

#endif // BOARD_H
