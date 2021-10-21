#include "game.h"
#include "board.h"
#include "button.h"

#include <stdlib.h>
#include <windows.h>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QFutureWatcher>

bool Game::getRealplayer() const {
    return realplayer;
}

Game::Game(PrologMinMax* ia, QWidget *parent): ia(ia), iaRunning(false) {
    (void)parent;
    // set up the screen
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(1024, 768);

    connect(&(this->iaFutureWatcher), SIGNAL(finished()), this, SLOT(onFinishedIa()));

    // set up the scene
    scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 1024, 768);
    setScene(scene);
}

void Game::start() {
    board = new Board();
    board->initialiseBoard();
    drawGUI();

    if((getWhosTurn() == 2 && realplayer == 1) || realplayer == 0) {
        getIATurn();
    }
}

void Game::launchRealPlayer() {
    scene->clear();

    realplayer = 1;
    start();
}

void Game::launchIA() {
    scene->clear();

    realplayer = 0;
    Button* nextButton = new Button(QString("Next Turn"));
    nextButton->setPos(560, 705);
    connect(nextButton, &Button::clicked, this, &Game::nextPlayersTurn);
    scene->addItem(nextButton);
    start();
}

void Game::displayMainMenu() {
    scene->clear();

    // create the title text
    QGraphicsTextItem* titleText = new QGraphicsTextItem(QString("POGO"));
    QFont titleFont("comic sans",50);
    titleText->setFont(titleFont);
    int txPos = this->width()/2 - titleText->boundingRect().width()/2;
    int tyPos = 150;
    titleText->setPos(txPos,tyPos);
    scene->addItem(titleText);

    // create the play button
    Button* playButton = new Button(QString("Play Against IA"));
    int bxPos = this->width()/2 - playButton->boundingRect().width()/2;
    int byPos = 275;
    playButton->setPos(bxPos, byPos);
    connect(playButton, &Button::clicked, this, &Game::launchRealPlayer);
    scene->addItem(playButton);

    // create the IAvsIA button
    Button* IaButton = new Button(QString("IA vs IA"));
    int bx2Pos = this->width()/2 - IaButton->boundingRect().width()/2;
    int by2Pos = 350;
    IaButton->setPos(bx2Pos, by2Pos);
    connect(IaButton, &Button::clicked, this, &Game::launchIA);
    scene->addItem(IaButton);

    // create the quit button
    Button* quitButton = new Button(QString("Quit"));
    int qxPos = this->width()/2 - quitButton->boundingRect().width()/2;
    int qyPos = 425;
    quitButton->setPos(qxPos, qyPos);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    scene->addItem(quitButton);
}

void Game::drawGUI() {
    // draw the right panel
    QGraphicsRectItem* panel = new QGraphicsRectItem(774, 0, 250, 768);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::lightGray);
    panel->setBrush(brush);
    panel->setOpacity(1);
    scene->addItem(panel);

    // place player 1 text
    QGraphicsTextItem* panelText = new QGraphicsTextItem("Pawns stack (click to see - from top to bottom)");
    panelText->setPos(774+10, 0);
    scene->addItem(panelText);

    // place whosTurnText
    whosTurnText = new QGraphicsTextItem();

    QFont textFont("comic sans", 30);
    whosTurnText->setFont(textFont);
    whosTurnText->setPos(270, 10);

    setWhosTurn(rand()%2 + 1);

    scene->addItem(whosTurnText);
}

void Game::setWhosTurn(int playernumber) {
    whosTurn = playernumber;
    if(playernumber == 1) {
        whosTurnText->setPlainText(QString("Player 1 Turn"));
        whosTurnText->setDefaultTextColor(Qt::darkCyan);
    } else {
        whosTurnText->setPlainText(QString("Player 2 Turn"));
        whosTurnText->setDefaultTextColor(Qt::darkRed);
    }
}

int Game::getWhosTurn() {
    return whosTurn;
}

void Game::nextPlayersTurn() {
    if (this->getIaRunning()) {
        return;
    }

    setWhosTurn(getWhosTurn() == 1 ? 2 : 1);

    if (getWhosTurn() == 2 || realplayer == 0) {
        getIATurn();
    }
}

struct State runIa(struct State state, PrologMinMax *ia) {
    return ia->run(state);
}

void Game::getIATurn() {
    this->iaRunning = true;
    this->iaFuture = QtConcurrent::run(runIa, this->board->pawnsToState(), this->ia);
    this->iaFutureWatcher.setFuture(this->iaFuture);
}

void Game::onFinishedIa(void) {
    this->board->stateToPawns(this->iaFuture.result());
    this->iaRunning = false;

    if (board->isGameFinished() != 1 && realplayer != 0) {
        nextPlayersTurn();
    }
}

void Game::closing(void) {
    delete this->ia;
}

void Game::gameFinished(int winnernumber) {
    scene->clear();

    QGraphicsTextItem* winText = new QGraphicsTextItem(QString("Game Finished : Player %1 Won !").arg(winnernumber));
    if (winnernumber == 1) {
        winText->setDefaultTextColor(Qt::darkCyan);
    } else {
        winText->setDefaultTextColor(Qt::darkRed);
    }
    QFont titleFont("comic sans", 50);
    winText->setFont(titleFont);
    int txPos = this->width()/2 - winText->boundingRect().width()/2;
    int tyPos = 200;
    winText->setPos(txPos, tyPos);
    scene->addItem(winText);

    Button* playAgainButton = new Button(QString("Play Again"));
    int bxPos = this->width()/2 - playAgainButton->boundingRect().width()/2;
    int byPos = 325;
    playAgainButton->setPos(bxPos, byPos);
    connect(playAgainButton, &Button::clicked, this, &Game::displayMainMenu);
    scene->addItem(playAgainButton);

    Button* quitButton = new Button(QString("Quit"));
    int qxPos = this->width()/2 - quitButton->boundingRect().width()/2;
    int qyPos = 400;
    quitButton->setPos(qxPos, qyPos);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    scene->addItem(quitButton);

    delete board;
}

bool Game::getIaRunning() {
    return this->iaRunning;
}
