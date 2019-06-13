#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QMatrix>

enum Location{
    Location_out = 0,
    Location_in = 1,
    Location_border = 2
};

template<typename T> class Triangle{
public:
    Triangle(const T& _first, const T& _second, const T& _third):first(_first), second(_second), third(_third){
        orderApexs();
    }
    Triangle(const Triangle<T>& another){
        first = another.first;
        second = another.second;
        third = another.third;
    }

    T getFirst()const{return first;}
    T getSecond()const{return second;}
    T getThird()const{return third;}

    void changeApexs(const T& newFirst, const T& newSecond, const T& newThird){
        first = newFirst;
        second = newSecond;
        third = newThird;

        orderApexs();
    }


    Triangle<T>& operator+=(const T& point){
        first += point;
        second += point;
        third += point;
        return *this;
    }

    // rotate triangle from 'pointRotateFrom' on degree 'degree'
    Triangle<T>& rotate(T pointRotateFrom, double degree){
        QMatrix matRot;
        matRot.rotate(degree);
        first = (first - pointRotateFrom) * matRot;
        second = (second - pointRotateFrom) * matRot;
        third = (third - pointRotateFrom)  * matRot;

        first += pointRotateFrom;
        second += pointRotateFrom;
        third += pointRotateFrom;

        orderApexs();

        return *this;
    }

    T middle() const{
        return T((first.x() + second.x() + third.x()) / 3 , (first.y() + second.y() + third.y()) / 3);
    }

    // find where point is located relatively triangle
    // point into triangle if square of triangle equal sum of three thriangles
    //which maked on matching apex of triangle with point 'point'
    Location pointLocation(const T& point)const{
        if(square(first, second, third) == square(point, second, third)
                + square(first, point, third)
                + square(first, second, point)){
                    return Location_in;
        }
        return Location_out;
    }

    ~Triangle(){}
private:
    int square(const T& _first, const T& _second, const T& _third) const {
        return abs(_second.x() * _third.y() - _third.x() * _second.y()
                   - _first.x() * _third.y() + _third.x() * _first.y()
                    + _first.x() * _second.y() - _second.x() * _first.y());
    }

    const Triangle& operator=(const Triangle&){}

    // order in ascending mode relatively Y coordinat {first, second, third}

    void orderApexs(){
        if(second.y() > third.y() || (second.y() == third.y() && second.x() > third.x())){
            qSwap(second, third);
        }
        if(first.y() > third.y() || (first.y() == third.y() && first.x() > third.x())){
            qSwap(first, third);
        }
        if(first.y() > second.y() || (first.y() == second.y() && first.x() > second.x())){
            qSwap(first, second);
        }

    }

    T first;
    T second;
    T third;
};

#endif // TRIANGLE_H
