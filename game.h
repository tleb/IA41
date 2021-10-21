#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <board.h>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QFutureWatcher>

class Game: public QGraphicsView
{
    Q_OBJECT
protected:
    bool realplayer;
    int whosTurn;
    QGraphicsTextItem* whosTurnText;
    Board* board;
    PrologMinMax* ia;
    QFuture<struct State> iaFuture;
    QFutureWatcher<struct State> iaFutureWatcher;
    bool iaRunning;

public:
    QGraphicsScene* scene;

    Game(PrologMinMax* ia, QWidget* parent=NULL);
    void start();
    void displayMainMenu();
    void launchIA();
    void launchRealPlayer();
    int getWhosTurn();
    void setWhosTurn(int playernumber);
    void drawGUI();
    void nextPlayersTurn();
    void getIATurn();
    bool getIaRunning();

    bool getRealplayer() const;

public slots:
    void onFinishedIa(void);
    void closing(void);

/*
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
*/
    void gameFinished(int winnernumber);
};

#endif // GAME_H
