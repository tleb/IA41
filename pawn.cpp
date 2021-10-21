#include "pawn.h"
#include <QBrush>
#include "board.h"
#include "game.h"

extern Board* board;
extern Game* game;

int Pawn::getOwner() {
    return owner;
}

Pawn::Pawn(QGraphicsItem* parent,int owner) : owner(owner) {
    (void)parent;
    setAcceptHoverEvents(true);
    setRect(0, 0, 100, 100);
    if (owner != 0) {
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        if(owner == 1)
            brush.setColor(Qt::darkCyan);
        else
            brush.setColor(Qt::darkRed);
        setBrush(brush);
    }
}

void Pawn::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    (void)event;
    emit clicked();
}

void Pawn::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    (void)event;
    emit hoverEnter();
}

void Pawn::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    (void)event;
    emit hoverLeave();
}
