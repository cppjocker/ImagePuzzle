#include "triangleanimationmodel.h"

#include <QTime>

#include <cmath>
#include <cassert>

TriangleAnimationModel::TriangleAnimationModel(const QPointF& first, const QPointF& second, const QPointF& third):
    currentTriangle(Triangle<QPoint>(QPoint(-1, -1), QPoint(-1, -1), QPoint(-1, -1))),
    textureTriangle(Triangle<QPointF>(first, second, third)),
    degree(-1), curve(BezeCurve(QPointF(0.f,0.f))), numPixelBorder(0), numPixelTriangle(0), numPixelTransparent(0)
{
    curve = BezeCurve(textureTriangle.middle());
    setNewDegree();
}

TriangleAnimationModel::BezeCurve::BezeCurve(const QPointF& _p3)
    :p0(QPointF(-1.f, -1.f)),p1(QPointF(-1.f, -1.f)),p2(QPointF(-1.f, -1.f)),p3(_p3)
{
    setRandomPoints();
}

QPointF TriangleAnimationModel::BezeCurve::moveOnCurve(float progress)const{
    return pow((1 - progress), 3) * p0 +
            pow(1 - progress, 2) * p1 * 3 * progress +
            (1 - progress) * p2 * 3 * pow(progress, 2) +
            p3 * pow(progress, 3);

}

void TriangleAnimationModel::BezeCurve::setNewCurve(){
    setRandomPoints();
}

void TriangleAnimationModel::BezeCurve::setRandomPoints(){
    QTime midnight(0, 0, 0);

    static bool isRandGenerate = false;

    if(!isRandGenerate){
        qsrand(midnight.secsTo(QTime::currentTime()));
        isRandGenerate = true;
    }

    p0 = QPointF((static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5,   // [-0.5, 1.5]
                (static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5) ; // [-0.5, 1.5]
    p1 = QPointF((static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5,   // [-0.5, 1.5]
                (static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5) ; // [-0.5, 1.5]
    p2 = QPointF((static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5,   // [-0.5, 1.5]
                (static_cast<float>(qrand() % RAND_MAX)) / (RAND_MAX - 1) * 2 - 0.5) ; // [-0.5, 1.5]

    assert(p0.x() >= -0.5f && p0.x() <= 1.5f);
    assert(p1.x() >= -0.5f && p1.x() <= 1.5f);
    assert(p2.x() >= -0.5f && p2.x() <= 1.5f);

    assert(p0.y() >= -0.5f && p0.y() <= 1.5f);
    assert(p1.y() >= -0.5f && p1.y() <= 1.5f);
    assert(p2.y() >= -0.5f && p2.y() <= 1.5f);
}

QPointF TriangleAnimationModel::getNextCurvePoint(float progress)const{
    return curve.moveOnCurve(progress);
}

void TriangleAnimationModel::setNewDegree(){
    QTime midday(12, 0, 0);

    static bool isRandGenerate = false;

    if(isRandGenerate == false){
        qsrand(midday.secsTo(QTime::currentTime()));
        isRandGenerate = true;
    }
    static const int MAX_DEGREE = 360;
    degree = qrand() % (MAX_DEGREE);// [0, 359]
}
