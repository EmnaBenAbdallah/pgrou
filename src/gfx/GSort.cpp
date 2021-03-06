#include <iostream>
#include <QPen>
#include <QColor>
#include <QBrush>
#include <QTextDocument>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QDebug>
#include <QApplication>
#include <QMenu>
#include <QPoint>
#include <QSize>
#include <cmath>
#include "GSort.h"


const int GSort::marginDefault = 10;
const int GSort::defaultDistance = 15;

GSort::GSort(SortPtr s, GVNode n, qreal width, qreal height) : QGraphicsRectItem(n.centerPos.x()-width/2, n.centerPos.y()-height/2, width, height),sort(s), node(n) {

    // graphic items set and Actions color
    color = makeColor();
    sizeRect = new QSize(width, height);

    leftTopCorner = new QPoint(n.centerPos.x()-sizeRect->width()/2,n.centerPos.y()-sizeRect->height()/2);

    // rectangle
    _rect = new QGraphicsRectItem(QRectF(*leftTopCorner, *sizeRect),this);
    _rect->setPen(QPen(QColor(7,54,66)));
    _rect->setBrush(QBrush(QColor(7,54,66)));

    // label
    text = new QGraphicsTextItem (QString(), this);
    text->setHtml(QString::fromStdString("<u>sort " + sort->getName() + "</u>"));
    text->setDefaultTextColor(*color);
    text->setPos(leftTopCorner->x()+sizeRect->width()/2, leftTopCorner->y());
    QSizeF textSize = text->document()->size();
    text->setPos(text->x() - textSize.width()/2, text->y() - textSize.height());

    setCursor(QCursor(Qt::OpenHandCursor));
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

    // set related GProcesses as children (so they move with this GSort)
    vector<ProcessPtr> processes = sort->getProcesses();
    int currPosYProcess = marginDefault+GProcess::sizeDefault/2;

    for(ProcessPtr &p : processes){
	gProcesses.push_back(make_shared<GProcess>(p, leftTopCorner->x() + GProcess::sizeDefault/2+ marginDefault, leftTopCorner->y()+ currPosYProcess));
	currPosYProcess+= 2*marginDefault + GProcess::sizeDefault;
    }
    for(GProcessPtr &gp: gProcesses){
	gp->getDisplayItem()->setParentItem(this);
	ProcessPtr* p = gp->getProcess();
	(*p)->setGProcess(gp);
    }

}

GSort::~GSort() {
    gProcesses.clear();
    delete _rect;
    delete leftTopCorner;
    delete sizeRect;
    delete text;   
}

// mouse press event handler: start "drag"
void GSort::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    // ignore right click
    if (event->button() == Qt::RightButton) {
        event->ignore();
        return;
    }

    setCursor(QCursor(Qt::ClosedHandCursor));


    // record coordinates for drawing item when mouse is moved/released
    initPosPoint.setX(pos().x());
    initPosPoint.setY(pos().y());
    eventPressPoint.setX(event->scenePos().x());
    eventPressPoint.setY(event->scenePos().y());

    event->accept();
}


// mouse move event handler: porcess "drag"
void GSort::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {

    // update item position
    setX(initPosPoint.x() + event->scenePos().x() - eventPressPoint.x());
    setY(initPosPoint.y() + event->scenePos().y() - eventPressPoint.y());

    event->accept();
}

void GSort::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {

    setCursor(QCursor(Qt::OpenHandCursor));

    //save new position of inital point
    int x1=initPosPoint.x() + event->scenePos().x() - eventPressPoint.x();
    int y1=initPosPoint.y() + event->scenePos().y() - eventPressPoint.y();

    int a=node.centerPos.x()+ event->scenePos().x() - eventPressPoint.x();
    int b=node.centerPos.y()+ event->scenePos().y() - eventPressPoint.y();

    int x2,y2;
    int distanceHeightMin=0;

    map<string, GSortPtr> listGSorts = dynamic_cast<PHScene*>(scene())->getGSorts();

    bool resetPosition = false;

    for(auto &s : listGSorts){

        x2=s.second.get()->getNode().centerPos.x();
        y2=s.second.get()->getNode().centerPos.y();

       if(s.second.get()->getSort()->getName()!=sort->getName()){

           distanceHeightMin=getSizeRect()->height()/2 + s.second.get()->getSizeRect()->height()/2;

           if( abs(a-x2)<getSizeRect()->width()+defaultDistance && abs(b-y2)<distanceHeightMin+defaultDistance ){
               resetPosition = true;
               break;
           }
        }
    }

        //coordinates have the initial value
    if(resetPosition){
        setX(initPosPoint.x());
        setY(initPosPoint.y());
        leftTopCorner->setX(x());
        leftTopCorner->setY(y());
    }
    else{
        setX(x1);
        setY(y1);
        leftTopCorner->setX(x());
        leftTopCorner->setY(y());
        for(GProcessPtr &p: gProcesses){
            p->getCenterPoint()->setX(p->getCenterPoint()->x() + event->scenePos().x() - eventPressPoint.x());
            p->getCenterPoint()->setY(p->getCenterPoint()->y() + event->scenePos().y() - eventPressPoint.y());
        }

        node.centerPos = QPoint(node.centerPos.x()+ event->scenePos().x() - eventPressPoint.x(),node.centerPos.y()+ event->scenePos().y() - eventPressPoint.y());

    }

    dynamic_cast<PHScene*>(scene())->updateActions();

    event->accept();
}
// context menu event handler
void GSort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {

    // if other mouse buttons are pressed, do nothing
    if (QApplication::mouseButtons() != Qt::RightButton) {
        event->ignore();
        return;
    }
}

// palette managementsizeRect

int GSort::paletteIndex = 0;

// TODO see QPalette class?
std::vector<QColor> GSort::palette = 	{	QColor(181,137,0)
										, 	QColor(220,50,47)
										, 	QColor(211,54,130)
										, 	QColor(108,113,196)
										, 	QColor(38,139,210)
										, 	QColor(42,161,152)
										, 	QColor(133,153,0)
										};

QColor* GSort::makeColor () {
	paletteIndex = (paletteIndex + 1) % palette.size();
	return &(palette[paletteIndex]);
}


// getters

QGraphicsRectItem* GSort::getRect() { return this->_rect; }

SortPtr GSort::getSort() { return this->sort; }

GVNode GSort::getNode() { return this->node; }

QGraphicsTextItem* GSort::getText() { return this->text; }

QPoint GSort::geteventPressPoint() { return this->eventPressPoint; }

QPoint* GSort::getLeftTopCornerPoint() {return this->leftTopCorner;}

QSize* GSort::getSizeRect() { return this->sizeRect;}

void GSort::updatePosition() {

}


void GSort::hide() {
    this->setOpacity(0);
}

void GSort::show() {
    this->setOpacity(1);
}

bool GSort::isVisible() {
    return (this->opacity() == 1);
}
