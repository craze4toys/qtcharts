#include "chartanimator_p.h"
#include "axisanimation_p.h"
#include "xyanimation_p.h"
#include "splineanimation_p.h"
#include "xychartitem_p.h"
#include "pieanimation_p.h"
#include "areachartitem_p.h"
#include "splinechartitem_p.h"
#include "scatterchartitem_p.h"
#include <QTimer>

Q_DECLARE_METATYPE(QVector<QPointF>)
Q_DECLARE_METATYPE(QVector<qreal>)

QTCOMMERCIALCHART_BEGIN_NAMESPACE

const static int duration = 1000;

ChartAnimator::ChartAnimator(QObject *parent):QObject(parent)
{
}

ChartAnimator::~ChartAnimator()
{
}

void ChartAnimator::addAnimation(AxisItem* item)
{
    ChartAnimation* animation = m_animations.value(item);

    if(!animation) {
        animation = new AxisAnimation(item);
        m_animations.insert(item,animation);
    }

    item->setAnimator(this);
}

void ChartAnimator::addAnimation(SplineChartItem*  item)
{
    ChartAnimation* animation = m_animations.value(item);

    if(!animation) {
        animation = new SplineAnimation(item);
        m_animations.insert(item,animation);
    }

    item->setAnimator(this);
}

void ChartAnimator::addAnimation(ScatterChartItem* item)
{
    ChartAnimation* animation = m_animations.value(item);

    if(!animation) {
        animation = new XYAnimation(item);
        m_animations.insert(item,animation);
    }

    item->setAnimator(this);
}

void ChartAnimator::addAnimation(LineChartItem*  item)
{
    ChartAnimation* animation = m_animations.value(item);

    if(!animation) {
        animation = new XYAnimation(item);
        m_animations.insert(item,animation);
    }

    item->setAnimator(this);
}

void ChartAnimator::addAnimation(PieChartItem* item)
{
    ChartAnimation* animation = m_animations.value(item);

    if(!animation) {
        animation = new PieAnimation(item);
        m_animations.insert(item,animation);
    }

    item->setAnimator(this);
}

void ChartAnimator::removeAnimation(ChartItem* item)
{
    item->setAnimator(0);
    m_animations.remove(item);
}

void ChartAnimator::applyLayout(AxisItem* item , QVector<qreal>& newLayout)
{
    AxisAnimation* animation = static_cast<AxisAnimation*>(m_animations.value(item));

    Q_ASSERT(animation);

    QVector<qreal> oldLayout = item->layout();

    if(newLayout.count()==0) return;

    switch(m_state)
	{
    	case ZoomOutState: {
    			QRectF rect = item->geometry();
    			oldLayout.resize(newLayout.count());

    			for(int i=0,j=oldLayout.count()-1;i<(oldLayout.count()+1)/2;i++,j--)
    			{
    				oldLayout[i]= item->axisType()==AxisItem::X_AXIS?rect.left():rect.bottom();
    				oldLayout[j]= item->axisType()==AxisItem::X_AXIS?rect.right():rect.top();
    			}
    		}
    	break;
		case ZoomInState: {
			int index = qMin(oldLayout.count()*(item->axisType()==AxisItem::X_AXIS?m_point.x():(1 -m_point.y())),newLayout.count()-1.0);
			oldLayout.resize(newLayout.count());

			for(int i=0;i<oldLayout.count();i++)
			{
				oldLayout[i]= oldLayout[index];
			}
		}
		break;
		case ScrollDownState:
		case ScrollRightState: {
			oldLayout.resize(newLayout.count());

			for(int i=0, j=i+1;i<oldLayout.count()-1;i++,j++)
			{
				oldLayout[i]= oldLayout[j];
			}
		}
		break;
		case ScrollUpState:
		case ScrollLeftState: {
			oldLayout.resize(newLayout.count());

			for(int i=oldLayout.count()-1, j=i-1;i>0;i--,j--)
			{
				oldLayout[i]= oldLayout[j];
			}
		}
		break;
		default: {
			oldLayout.resize(newLayout.count());
			QRectF rect = item->geometry();
			for(int i=0, j=oldLayout.count()-1;i<oldLayout.count();i++,j--)
			{
				oldLayout[i]= item->axisType()==AxisItem::X_AXIS?rect.left():rect.top();
			}
		}
		break;
	}


    if(animation->state()!=QAbstractAnimation::Stopped) {
        animation->stop();
    }

    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::OutQuart);
    QVariantAnimation::KeyValues value;
    animation->setKeyValues(value); //workaround for wrong interpolation call
    animation->setKeyValueAt(0.0, qVariantFromValue(oldLayout));
    animation->setKeyValueAt(1.0, qVariantFromValue(newLayout));

    QTimer::singleShot(0,animation,SLOT(start()));
}

void ChartAnimator::updateLayout(SplineChartItem* item, QVector<QPointF>& oldPoints ,QVector<QPointF>& newPoints, QVector<QPointF>& oldControlPoints, QVector<QPointF>& newControlPoints,int index)
{
    SplineAnimation* animation = static_cast<SplineAnimation*>(m_animations.value(item));

    Q_ASSERT(animation);

    if(newPoints.count()<2 || newControlPoints.count()<2) return;

    bool empty = oldPoints.count()==0;


    if(animation->state()!=QAbstractAnimation::Stopped) {
         animation->stop();
    }

    animation->setDuration(duration);
    if(!empty)
    animation->setAnimationType(ChartAnimation::MoveDownAnimation);
    else
    animation->setAnimationType(ChartAnimation::LineDrawAnimation);

    animation->setEasingCurve(QEasingCurve::OutQuart);
    animation->setValues(oldPoints,newPoints,oldControlPoints,newControlPoints,index);

    QTimer::singleShot(0,animation,SLOT(start()));
}


void ChartAnimator::updateLayout(XYChartItem* item,  QVector<QPointF>& oldPoints , QVector<QPointF>& newPoints, int index)
{
    XYAnimation* animation = static_cast<XYAnimation*>(m_animations.value(item));

    Q_ASSERT(animation);

    if(newPoints.count()==0) return;

    bool empty = oldPoints.count()==0;


    if(animation->state()!=QAbstractAnimation::Stopped) {
        animation->stop();
    }

    animation->setDuration(duration);
    if(!empty)
    animation->setAnimationType(ChartAnimation::MoveDownAnimation);
    else
    animation->setAnimationType(ChartAnimation::LineDrawAnimation);

    animation->setEasingCurve(QEasingCurve::OutQuart);
    animation->setValues(oldPoints,newPoints,index);

    QTimer::singleShot(0,animation,SLOT(start()));
}

void ChartAnimator::addAnimation(PieChartItem* item, QPieSlice *slice, const PieSliceLayout &layout)
{
    PieAnimation* animation = static_cast<PieAnimation*>(m_animations.value(item));
    Q_ASSERT(animation);
    animation->addSlice(slice, layout);
}

void ChartAnimator::removeAnimation(PieChartItem* item, QPieSlice *slice)
{
    PieAnimation* animation = static_cast<PieAnimation*>(m_animations.value(item));
    Q_ASSERT(animation);
    animation->removeSlice(slice);
}

void ChartAnimator::updateLayout(PieChartItem* item, const PieLayout &layout)
{
    PieAnimation* animation = static_cast<PieAnimation*>(m_animations.value(item));
    Q_ASSERT(animation);
    animation->updateValues(layout);
}

void ChartAnimator::updateLayout(PieChartItem* item, QPieSlice *slice, const PieSliceLayout &layout)
{
    PieAnimation* animation = static_cast<PieAnimation*>(m_animations.value(item));
    Q_ASSERT(animation);
    animation->updateValue(slice, layout);
}

void ChartAnimator::setState(State state,const QPointF& point)
{
	m_state=state;
	m_point=point;
}

QTCOMMERCIALCHART_END_NAMESPACE
