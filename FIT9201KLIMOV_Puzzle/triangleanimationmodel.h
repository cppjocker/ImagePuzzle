#ifndef TRIANGLEANIMATIONMODEL_H
#define TRIANGLEANIMATIONMODEL_H

#include "triangle.h"

class TriangleAnimationModel
{
public:
    TriangleAnimationModel(const QPointF& first, const QPointF& second, const QPointF& third);

    void setPixelBorder(int _numPixelBorder){numPixelBorder =  _numPixelBorder;}
    void setPixelTriangle(int _numPixelTriangle){numPixelTriangle = _numPixelTriangle;}
    void setPixelTransparent(int _numPixelTransparent){
        numPixelTransparent = _numPixelTransparent;
    }
    int getPixelBorder()const{return numPixelBorder;}
    int getPixelTriangle()const{return numPixelTriangle;}
    int getPixelTransparent()const{return numPixelTransparent;}
    QPoint getFirst()const {return currentTriangle.getFirst();}
    QPoint getSecond()const {return currentTriangle.getSecond();}
    QPoint getThird()const {return currentTriangle.getThird();}

    void setNewCurve(){curve.setNewCurve();}
    void setCurrentTriangle(const Triangle<QPoint>& current){
       currentTriangle.changeApexs(current.getFirst(), current.getSecond(), current.getThird());
    }
    bool interSect(const QPoint& point)const{
        return (currentTriangle.pointLocation(point) == Location_in);
    }

    float getDegree()const {return degree;}
    const Triangle<QPointF>& getTextureTriangle()const {return textureTriangle;}
    QPointF getNextCurvePoint(float progress) const;
    void setNewDegree();
private:
    // (1 - t)^3 * Po + 3t * (1 - t)^2 * P1 + 3t^2 * (1-t) * P2 + t^3 * P3
    class BezeCurve{
    public:
        BezeCurve(const QPointF& p3);
        void setNewCurve();
        QPointF moveOnCurve(float progress) const;
    private:
        QPointF p0; // end of curve
        QPointF p1;
        QPointF p2;
        QPointF p3; // start of curve
        void setRandomPoints();
    };
    Triangle<QPoint> currentTriangle;
    Triangle<QPointF> textureTriangle;
    int degree;
    BezeCurve curve;
    int numPixelBorder;
    int numPixelTriangle;
    int numPixelTransparent;
};

#endif // TRIANGLEANIMATIONMODEL_H
