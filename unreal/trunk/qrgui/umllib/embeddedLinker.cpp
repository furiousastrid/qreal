#include "embeddedLinker.h"
#include "uml_nodeelement.h"

#include <QtGui/QStyle>
#include <QtGui/QGraphicsItem>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtCore/QDebug>
#include <math.h>

#include "../view/editorviewscene.h"
#include "../mainwindow/mainwindow.h"

using namespace UML;
using namespace qReal;

EmbeddedLinker::EmbeddedLinker()
{
	covered = false;
	master = NULL;
	mEdge = NULL;
	mRectangle = QRectF(-6,-6,12,12);
	mInnerRectangle = QRectF(-3,-3,6,6);
	setAcceptsHoverEvents(true);
	color = Qt::blue;

	QObject::connect(this,SIGNAL(coveredChanged()),this,SLOT(changeShowState()));
}

EmbeddedLinker::~EmbeddedLinker()
{

}

void EmbeddedLinker::setMaster(NodeElement *element)
{
	master = element;
	setParentItem(element);
	QObject::connect(master->scene(),SIGNAL(selectionChanged()),this,SLOT(changeShowState()));
}

void EmbeddedLinker::setCovered(bool arg)
{
	covered = arg;
	emit coveredChanged();
}

void EmbeddedLinker::generateColor()
{
	int result = 0;
	QChar *data = edgeType.data();
	while (!data->isNull()) {
		result += data->unicode();
		++data;
	}
	result *= 666;
	color =QColor(result%192+64,result%128+128,result%64+192).darker(0);
}

void EmbeddedLinker::setColor(QColor arg)
{
		color = arg;
}

void EmbeddedLinker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
	Q_UNUSED(option);
	painter->save();

	QBrush brush;
		brush.setColor(color);
	brush.setStyle(Qt::SolidPattern);
	painter->setBrush(brush);
	painter->setOpacity(0.5);
	painter->setPen(color);

	painter->drawEllipse(mRectangle);
	painter->setOpacity(0.7);
	painter->drawEllipse(mInnerRectangle);

	painter->restore();
}

void EmbeddedLinker::setDirected(bool arg)
{
	directed = arg;
}

void EmbeddedLinker::initTitle()
{
	EditorManager* editorManager = dynamic_cast<EditorViewScene*>(scene())->mainWindow()->manager();
	QString edgeTypeFriendly = editorManager->friendlyName(Id::loadFromString("qrm:/"+master->uuid().editor()+"/"+master->uuid().diagram()+"/"+edgeType));

	float textWidth = edgeTypeFriendly.size()*10;
	float rectWidth = master->boundingRect().right() - master->boundingRect().left();
	float rectHeight = master->boundingRect().bottom() - master->boundingRect().top();

	int x = 0;
	int y = 0;
	if (this->scenePos().y() < master->scenePos().y() + rectHeight/3)
		y = -boundingRect().height() - 10;
	else if (this->scenePos().y() > master->scenePos().y() + 2*rectHeight/3)
		y = +boundingRect().height() - 10;

	if (this->scenePos().x() < master->scenePos().x() + rectWidth/3)
		x = -boundingRect().width() - textWidth + 20;
	else if (this->scenePos().x() > master->scenePos().x() + 2*rectWidth/3)
		x = +boundingRect().width() - 10;

	title = new ElementTitle(x,y,edgeTypeFriendly);
	title->setTextWidth(textWidth);
	title->setParentItem(this);
}

void EmbeddedLinker::setEdgeType(QString arg)
{
	edgeType = arg;
	generateColor();
}

bool EmbeddedLinker::isDirected()
{
	return directed;
}

QString EmbeddedLinker::getEdgeType()
{
	return edgeType;
}

void EmbeddedLinker::takePosition(int index, int maxIndex)
{
	const float Pi = 3.141592;
	QRectF bounding = master->boundingRect();

	float top = bounding.topLeft().y();
	float left = bounding.topLeft().x();
	float right = bounding.bottomRight().x();
	float bottom = bounding.bottomRight().y();
	float height = bottom - top;
	float width = right - left;

	float angle = 2*Pi*index/maxIndex;
	float indent = 5;

	int rW = width;
	int rH = height;
	if (rW < 150)
		rW *= 1.5;
	else
		rW += indent;
	if (rH < 150)
		rH *= 1.5;
	else
		rH += indent;

	float px = left + width/2 + rW*cos(angle - Pi/2)/2;
	float py = bottom - height/2 + rH*sin(angle - Pi/2)/2;

	//if linker covers master node:

	float min = py - top;
	if (min > bottom - py)
		min = bottom - py;
	if (min > px - left)
		min = px - left;
	if (min > right - px)
		min = right - px;

	float fx;
	float fy;

	//obviously, top != left != right != bottom
	if ((bottom - py == min) || (py - top == min))
	{
		fx = px;
		if (bottom - py == min)
			fy = bottom + indent;
		else
			fy = top - indent;
	}
	else
	{
		fy = py;
		if (right - px == min)
			fx = right + indent;
		else
			fx = left - indent;
	}

	this->setPos(fx,fy);

	//useful for debug:
	//scene()->addPolygon(master->mapToScene(master->boundingRect().left(),master->boundingRect().top(),
	//									master->boundingRect().width(),master->boundingRect().height()));
}

QRectF EmbeddedLinker::boundingRect() const
{
	return mRectangle;
}

void EmbeddedLinker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	qDebug() << "mousePress (LINKER)";
	EditorViewScene *scene = dynamic_cast<EditorViewScene*>(master->scene());

	if (scene != NULL)
	{
		const QString type = "qrm:/"+master->uuid().editor()+"/"+master->uuid().diagram()+"/"+edgeType;
		if (scene->mainWindow()->manager()->hasElement(Id::loadFromString(type)))
		{
			Id *edgeId = scene->createElement(type, event->scenePos());
			mEdge = dynamic_cast<EdgeElement*>(scene->getElem(*edgeId));
			mEdge->placeStartTo(master->getPortPos(0));
		}
		if (mEdge != NULL)
		{
			mEdge->setSelected(true);
			master->setSelected(false);
			mEdge->placeEndTo(mEdge->mapFromScene(mapToScene(event->pos())));
		}
	}
}

void EmbeddedLinker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	qDebug() << "mouseMove (LINKER)";
	if (mEdge != NULL)
		mEdge->placeEndTo(mEdge->mapFromScene(mapToScene(event->pos())));
}

void EmbeddedLinker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	qDebug() << "mouseRelease (LINKER)";
	hide();
	EditorViewScene *scene = dynamic_cast<EditorViewScene*>(master->scene());
	if ((scene != NULL) && (mEdge != NULL))
	{
		mEdge->hide();
		NodeElement *under = dynamic_cast<NodeElement*>(scene->itemAt(event->scenePos()));
		mEdge->show();
		if (under == NULL)
			if (scene->launchEdgeMenu(mEdge, master, event->scenePos()))
				mEdge = NULL;
	}
	if (mEdge != NULL)
		mEdge->connectToPort();
}

void EmbeddedLinker::changeShowState()
{
	QList<QGraphicsItem*> graphicsItems;
	if (scene())
		graphicsItems = scene()->selectedItems();
	if ((!master) || (!scene()) || (!covered) ||
		((!graphicsItems.contains(master)) && (!graphicsItems.contains(mEdge))))
	{
		hide();
		return;
	}
	else if ((graphicsItems.contains(master)) && (graphicsItems.size() == 1) && covered)
		show();
}
