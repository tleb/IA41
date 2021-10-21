#ifndef PAWN_H
#define PAWN_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QObject>

class Pawn : public QObject, public QGraphicsEllipseItem{
    Q_OBJECT
private:
    int owner;

public:
    Pawn(QGraphicsItem* parent=NULL,int owner = 0);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    int getOwner();

signals:
    void hoverEnter();
    void hoverLeave();
    void clicked();
};

#endif // PAWN_H
