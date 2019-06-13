#include "puzzlewindow.h"

#include <QPainter>
#include <QTime>
#include <QMouseEvent>

#include <cassert>
#include <cmath>

namespace{
    static const QString PUZZLE_FILE = ":/images/puzzle.png";
    static const int WIDTH_SETTINGS_PANEL = 110;
    static const int HEIGHT_SETTINGS_PANEL = 255;
    static const int INTERVAL = 40;
    static const int INTERVAL_QUEUE_COLLECTING = 10;
    static const int NUM_SQUIERS = 4;
    static const int MAX_DIAL = 180;
    static const int OFFSET = NUM_SQUIERS * NUM_SQUIERS;
    static const int MAX_QUEUE_SIZE_FOR_DRAWING = 10;
    static const int DEADLINE = 100000;
}

PuzzleWindow::PuzzleWindow(QWidget *parent) :
    QMainWindow(parent),isStopped(true),
    isFiltered(false), isAlphaMixered(false), lastAnimatedTime(QTime::currentTime())
{
    setPuzzleArea();
    puzzle = QImage(PUZZLE_FILE);

    setModelTextureCoordinates();

    setupUi(this);

    setPuzzleArea();

    connect(&timer,SIGNAL(timeout()),SLOT(sl_onTimeout()));
    connect(&timerProgress,SIGNAL(timeout()),SLOT(sl_onTimeoutProgress()));

    timer.setInterval(INTERVAL);
    timerProgress.setInterval(INTERVAL_QUEUE_COLLECTING);
    timerProgress.start();

    onProgress(0.f);
}

void PuzzleWindow::setPuzzleArea(){
    puzzleArea = QImage(this->width() - WIDTH_SETTINGS_PANEL , this->height(), QImage::Format_RGB888);
    puzzleArea.fill(QColor(Qt::white).rgb());
}

void PuzzleWindow::setModelTextureCoordinates(){

       int step = puzzle.width() / NUM_SQUIERS;
       assert(step > 0);
       /* calculate all such triangles
            |\
            | \
            |__\
        */
       for(int i = 0; i<puzzle.width(); i += step){
           for(int j = 0; j<puzzle.height(); j += step){

               models.append(QSharedPointer<TriangleAnimationModel>(new TriangleAnimationModel
                    (QPointF( static_cast<float>(i) / puzzle.width(),
                    static_cast<float>( j) / puzzle.height()) ,
                    QPointF(static_cast<float>(puzzle.width() - step > step ? i + step : puzzle.width()) / puzzle.width(),
                    static_cast<float>(j) / puzzle.height()),
                    QPointF(static_cast<float>(i) / puzzle.width() ,
                    static_cast<float>(qMin(j + step,puzzle.height())) / puzzle.height()))));
           }
      }
      /* calculate all such triangles
         ____
         \  |
          \ |
           \|
       */
      for(int i = 0; i<puzzle.width(); i += step){
          for(int j = 0; j<puzzle.height(); j += step){
              models.append(QSharedPointer<TriangleAnimationModel>(new TriangleAnimationModel
                    (QPointF(static_cast<float>(i) / puzzle.width(),
                    static_cast<float>(puzzle.height() - step > step ? j + step : puzzle.height()) / puzzle.height()),
                    QPointF(static_cast<float>(puzzle.width() - step > step ? i + step : puzzle.width()) / puzzle.width(),
                    static_cast<float>(j) / puzzle.height()),
                    QPointF(static_cast<float>(puzzle.width() - step > step ? i + step : puzzle.width()) / puzzle.width(),
                    static_cast<float>(puzzle.height() - step > step ? j + step : puzzle.height()) / puzzle.height()))));
          }
     }
}

PuzzleWindow::~PuzzleWindow(){
}

void PuzzleWindow::resizeEvent(QResizeEvent * ){

    setPuzzleArea();

    const int offsetWidth = this->width() - WIDTH_SETTINGS_PANEL - 1;
    puzzlePanel->setGeometry(QRect(QPoint(offsetWidth, 0), QPoint(offsetWidth + WIDTH_SETTINGS_PANEL, HEIGHT_SETTINGS_PANEL)));
    getProgress(dial->value(), true);
}

void PuzzleWindow::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.drawImage(0, 0, puzzleArea);
}

bool PuzzleWindow::eventFilter(QObject* obj, QEvent *event){
    if(event->type() == QEvent::MouseMove){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint point(mouseEvent->pos().x(), mouseEvent->pos().y() );
        for(int i = models.size()-1; i >= 0; --i){
            if(models[i]->interSect(point)){
                QMainWindow::statusBar()->showMessage(QString("Pixels: Not transparent = %1 border = %2 all = %3 Triangle id = %4 Triangle = "
                    "{{%5, %6}, {%7, %8}, {%9, %10}} pos = {%11, %12}").
                    arg(models[i]->getPixelTransparent()).
                    arg(models[i]->getPixelBorder()).
                    arg(models[i]->getPixelTriangle()).
                    arg(i).
                    arg(models[i]->getFirst().x()).
                    arg(models[i]->getFirst().y()).
                    arg(models[i]->getSecond().x()).
                    arg(models[i]->getSecond().y()).
                    arg(models[i]->getThird().x()).
                    arg(models[i]->getThird().y()).
                    arg(mouseEvent->pos().x()).
                    arg(mouseEvent->pos().y()), 10000);
                return false;
            }
        }
        QMainWindow::statusBar()->clearMessage();
    }
    else{
        QMainWindow::eventFilter(obj, event);
    }
    return false;
}

void PuzzleWindow::sl_onStartDraw(){
    if(!isStopped){
        return;
    }
    isStopped = false;
    lastAnimatedTime = QTime::currentTime();
    timer.start();
}

void PuzzleWindow::sl_onStopDraw(){
    if(isStopped){
        return;
    }
    timer.stop();
    isStopped = true;
}

void PuzzleWindow::sl_onTimeout(){
    QTime tmp = QTime::currentTime();
    int milliseconds = lastAnimatedTime.msecsTo(tmp);
    dial->setValue((dial->value() + milliseconds / INTERVAL) % (dial->maximum() + 1));
    lastAnimatedTime = tmp;
}

void PuzzleWindow::sl_onTimeoutProgress(){
    if(progresses.size() > MAX_QUEUE_SIZE_FOR_DRAWING){
        assert(progresses.size() > 0);
        setPuzzleArea();
        onProgress(progresses[progresses.size() - 1]);
    }
    else{
        foreach(const float progress, progresses){
            setPuzzleArea();
            onProgress(progress);
        }
    }
    progresses.clear();
}

void PuzzleWindow::sl_onFilterChanged(int state){
    setPuzzleArea();
    if(Qt::Unchecked == state ){
        isFiltered = false;
    }
    else{
        isFiltered = true;
    }
    getProgress(dial->value(), false);
}

void PuzzleWindow::sl_onInit(){
    foreach(const QSharedPointer<TriangleAnimationModel>& model, models ){
        model->setNewCurve();
    }
    dial->setValue(0);
    onProgress(0.f);
}

void PuzzleWindow::sl_onDegreeChanged(int newDegree){
     getProgress(newDegree, false);
}

void PuzzleWindow::sl_onAlphaMixChanged(int state){
    setPuzzleArea();
    if(Qt::Unchecked == state ){
        isAlphaMixered = false;
    }
    else{
        isAlphaMixered = true;
    }
    getProgress(dial->value(), false);
}

void PuzzleWindow::getProgress(int newDegree, bool drawImmediately){
    if(newDegree > MAX_DIAL ){
        newDegree = MAX_DIAL - (newDegree - MAX_DIAL);
    }
    float progress = static_cast<float>(newDegree) / MAX_DIAL;
    if(drawImmediately){
       onProgress(progress);
    }
    else{
       progresses.append(progress);
    }
}

void PuzzleWindow::onProgress(const float progress){
    const int scaleX = puzzleArea.width() /10;
    const int scaleY = puzzleArea.height()/10;
    const int imageX = scaleX * 4;
    const int imageY = scaleY * 4;

    assert(imageX != 0);
    assert(imageY != 0);

    for(int k = 0; k<models.size(); ++k){
        QSharedPointer<TriangleAnimationModel>& model = models[k];
        const Triangle<QPointF>& startPosTriangle(model->getTextureTriangle());
        Triangle<QPointF> currentTriangle(startPosTriangle);
        const float currentDegree = model->getDegree() * progress;

        assert(startPosTriangle.getFirst().x() >= 0.f && startPosTriangle.getFirst().x() <= 1.f);
        assert(startPosTriangle.getFirst().y() >= 0.f && startPosTriangle.getFirst().y() <= 1.f);
        assert(startPosTriangle.getSecond().x() >= 0.f && startPosTriangle.getSecond().x() <= 1.f);
        assert(startPosTriangle.getSecond().y() >= 0.f && startPosTriangle.getSecond().y() <= 1.f);
        assert(startPosTriangle.getThird().x() >= 0.f && startPosTriangle.getThird().x() <= 1.f);
        assert(startPosTriangle.getThird().y() >= 0.f && startPosTriangle.getThird().y() <= 1.f);

        QPointF coordinateRotateFrom = startPosTriangle.middle();

        currentTriangle.rotate(coordinateRotateFrom, currentDegree);           // rotate
        QPointF curvePoint(model->getNextCurvePoint(1.f - progress) );

        assert(-0.5f <= curvePoint.x() && 1.5 >= curvePoint.x());
        assert(-0.5f <= curvePoint.y() && 1.5 >= curvePoint.y());

        currentTriangle += curvePoint - startPosTriangle.middle(); //shift

        assert(currentTriangle.getFirst().x() >= -0.75f && currentTriangle.getFirst().x() <= 1.75f);
        assert(currentTriangle.getFirst().y() >= -0.75f && currentTriangle.getFirst().y() <= 1.75f);
        assert(currentTriangle.getSecond().x() >= -0.75f && currentTriangle.getSecond().x() <= 1.75f);
        assert(currentTriangle.getSecond().y() >= -0.75f && currentTriangle.getSecond().y() <= 1.75f);
        assert(currentTriangle.getThird().x() >= -0.75f && currentTriangle.getThird().x() <= 1.75f);
        assert(currentTriangle.getThird().y() >= -0.75f && currentTriangle.getThird().y() <= 1.75f);

        const float offsetFromModelCoordinat = 0.75f;

        const int scaledFirstX = (currentTriangle.getFirst().x() + offsetFromModelCoordinat) * imageX;
        const int scaledSecondX = (currentTriangle.getSecond().x() + offsetFromModelCoordinat) * imageX;
        const int scaledThirdX = (currentTriangle.getThird().x() + offsetFromModelCoordinat) * imageX;

        const int scaledFirstY = (currentTriangle.getFirst().y() + offsetFromModelCoordinat) * imageY;
        const int scaledSecondY = (currentTriangle.getSecond().y() + offsetFromModelCoordinat) * imageY;
        const int scaledThirdY = (currentTriangle.getThird().y() + offsetFromModelCoordinat) * imageY;

        assert(puzzleArea.width() > scaledFirstX && 0 < scaledFirstX );
        assert(puzzleArea.width() > scaledSecondX && 0 < scaledSecondX );
        assert(puzzleArea.width() > scaledThirdX && 0 < scaledThirdX );

        assert(puzzleArea.height() > scaledFirstY && 0 < scaledFirstY);
        assert(puzzleArea.height() > scaledSecondY && 0 < scaledSecondY);
        assert(puzzleArea.height() > scaledThirdY && 0 < scaledThirdY);

        assert(scaledFirstY <= scaledSecondY );
        assert(scaledSecondY <= scaledThirdY);

        Triangle<QPoint> currentPixelTriangle(QPoint(scaledFirstX, scaledFirstY),
                                              QPoint(scaledSecondX, scaledSecondY),
                                              QPoint(scaledThirdX, scaledThirdY));
        model->setCurrentTriangle(currentPixelTriangle);

        int error_1_2 = abs(scaledSecondX - scaledFirstX ) - (scaledSecondY - scaledFirstY );
        int error_1_3 = abs(scaledThirdX - scaledFirstX) - (scaledThirdY - scaledFirstY );

        int nextLinePointX_1_2 = scaledFirstX;
        int nextLinePointY_1_2 = scaledFirstY;

        int nextLinePointX_1_3 = scaledFirstX;
        int nextLinePointY_1_3 = scaledFirstY;

        bool isFilled = true;
        int numPixelTriangle = 0;
        int numPixelBorder = 0;
        int numPixelTransparent = 0;

        // draw lines from the top (first Y coordinate) in both directions until (second Y coordinate not achieved)
        puzzleArea.setPixel(QPoint(scaledSecondX,scaledSecondY), QColor(Qt::black).rgb());
        numPixelBorder++;

        int l = 0;
        for(l = 0; l < DEADLINE &&
            (nextLinePointX_1_2 != scaledSecondX
             || nextLinePointY_1_2 < scaledSecondY);++l)
        {
            // filling Y line
            if((nextLinePointY_1_2 == nextLinePointY_1_3) && !isFilled){
                const int leftX = qMin(nextLinePointX_1_2, nextLinePointX_1_3);
                const int rightX = qMax(nextLinePointX_1_2, nextLinePointX_1_3);
                for(int i = leftX+1; i<rightX; ++i){
                    QMatrix rotBack;

                    // rotation and shifting current point to point in model coordinates
                    QPointF texturePoint = QPointF(static_cast<float>(i) / imageX - offsetFromModelCoordinat ,
                                                   static_cast<float>(nextLinePointY_1_2) / imageY - offsetFromModelCoordinat );
                    texturePoint = ((texturePoint - curvePoint + startPosTriangle.middle()) - coordinateRotateFrom) *  rotBack.rotate(-currentDegree);
                    texturePoint += coordinateRotateFrom;

                    // normalizing coordinates if there is imprecisions of calculations (< 0.f or > 1.f)
                    float normX = (0.f > texturePoint.x()) ? 0.f : texturePoint.x();
                    normX = (1.f < normX) ? 1.f : normX;
                    float normY = (0.f > texturePoint.y()) ? 0.f : texturePoint.y();
                    normY = (1.f < normY) ? 1.f : normY;

                    int alpha = 0;
                    float alphaMixVal = 0.f;

                    QRgb color;
                    if(isFiltered){
                        color = makeFilter(QPointF(normX, normY), alpha);
                        alphaMixVal = (isAlphaMixered ? static_cast<float>(alpha) / 255 : 1.f);
                    }
                    else{
                        color = puzzle.pixel(QPoint(normX * (puzzle.width() - 1) + 0.5f,
                                                    normY * (puzzle.height() - 1) + 0.5f));
                        // use alphaMix or not
                        alphaMixVal = (isAlphaMixered ?
                                    (static_cast<float>  (qAlpha(puzzle.pixel(QPoint(normX * (puzzle.width() - 1) + 0.5f,
                                                                                     normY * (puzzle.height() - 1) + 0.5f)))) / 255) : 1.f);
                    }

                    //knowledge of not transporant pixels
                    const float ebs = 0.0001f;
                    if(fabs(alphaMixVal - 1.f) < ebs){
                        numPixelTransparent++;
                    }

                    assert(0 < nextLinePointY_1_2 && puzzleArea.height() > nextLinePointY_1_2);

                    puzzleArea.setPixel(QPoint(i, nextLinePointY_1_2),
                            qRgb((1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_1_2)).red() +
                            alphaMixVal * QColor(color).red(),
                            (1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_1_2)).green() +
                            alphaMixVal * QColor(color).green(),
                            (1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_1_2)).blue() +
                            alphaMixVal * QColor(color).blue()));
                    numPixelTriangle++;
                }
                isFilled = true;
            }
            // draw next point at 1_2 line
            // wait if another line is behind (Y coordinates)
            else if(nextLinePointY_1_2 <= nextLinePointY_1_3){
                assert(0 < nextLinePointY_1_2 && puzzleArea.height() > nextLinePointY_1_2);

                puzzleArea.setPixel(QPoint(nextLinePointX_1_2, nextLinePointY_1_2), QColor(Qt::black).rgb());
                numPixelBorder++;

                int curErr = error_1_2 * 2;
                if(curErr > -(scaledSecondY - scaledFirstY )){
                    nextLinePointX_1_2+= (scaledFirstX < scaledSecondX ? 1: -1);
                    error_1_2 -= (scaledSecondY - scaledFirstY );
                }
                if(curErr < abs(scaledFirstX - scaledSecondX)){
                    nextLinePointY_1_2++;
                    error_1_2 += abs(scaledFirstX - scaledSecondX);
                    isFilled = false;
                }
            }
            // draw next point at 1_3 line
            // wait if another line is behind (Y coordinates)
            else if(nextLinePointY_1_3 <= nextLinePointY_1_2){
                assert(0 < nextLinePointY_1_3 && puzzleArea.height() > nextLinePointY_1_3);

                puzzleArea.setPixel(QPoint(nextLinePointX_1_3,nextLinePointY_1_3), QColor(Qt::black).rgb());
                numPixelBorder++;

                int curErr = error_1_3 * 2;
                if(curErr > -( scaledThirdY - scaledFirstY )){
                    nextLinePointX_1_3 += (scaledFirstX < scaledThirdX ? 1: -1);
                    error_1_3 -= (scaledThirdY - scaledFirstY );
                }
                if(curErr < abs(scaledFirstX - scaledThirdX )){
                    nextLinePointY_1_3++;
                    error_1_3 += abs(scaledFirstX - scaledThirdX);
                    isFilled = false;
                }
            }
            else{
                assert(false && "State of rasterization is invalid.");
            }
        }
        assert(l < DEADLINE);
        int nextLinePointX_2_3 = scaledSecondX;
        int nextLinePointY_2_3 = scaledSecondY;

        int error_2_3 = abs(scaledThirdX - scaledSecondX) - (scaledThirdY - scaledSecondY );

        isFilled = true;

        // continue of algorithm : draw lines from second Y coordanat to third Y coordinat and continue
        // drawing of (1,3) - line until third apex is not achieved
        puzzleArea.setPixel(QPoint(scaledThirdX,scaledThirdY), QColor(Qt::black).rgb());
        numPixelBorder++;

        for(l = 0;l<DEADLINE;++l){
            // filling Y line
            if((nextLinePointY_2_3 == nextLinePointY_1_3) && !isFilled){
                const int leftX = qMin(nextLinePointX_2_3, nextLinePointX_1_3);
                const int rightX = qMax(nextLinePointX_2_3, nextLinePointX_1_3);
                for(int i = leftX+1 ;i<rightX;++i){
                    QMatrix rotBack;

                    // rotation and shifting current point to point in model coordinates
                    QPointF texturePoint = QPointF(static_cast<float>(i) / imageX - offsetFromModelCoordinat ,
                            static_cast<float>(nextLinePointY_2_3) / imageY - offsetFromModelCoordinat );
                    texturePoint = ((texturePoint - curvePoint + startPosTriangle.middle()) - coordinateRotateFrom) *  rotBack.rotate(-currentDegree);
                    texturePoint += coordinateRotateFrom;

                    // normalizing coordinates if there is imprecisions of calculations (< 0.f or > 1.f)
                    float normX = (0.f > texturePoint.x()) ? 0.f : texturePoint.x();
                    normX = (1.f < normX) ? 1.f : normX;
                    float normY = (0.f > texturePoint.y()) ? 0.f : texturePoint.y();
                    normY = (1.f < normY) ? 1.f : normY;

                    // use alphaMix or not

                    int alpha = 0;
                    float alphaMixVal = 0.f;

                    QRgb color;

                    if(isFiltered){
                        color = makeFilter(QPointF(normX, normY), alpha);
                        alphaMixVal = (isAlphaMixered ? static_cast<float>(alpha) / 255 : 1.f);
                    }
                    else{
                        color = puzzle.pixel(QPoint(normX * (puzzle.width() - 1) + 0.5,
                                                    normY * (puzzle.height() - 1) + 0.5));
                        alphaMixVal = (isAlphaMixered ?
                                                        static_cast<float>  (qAlpha(puzzle.pixel(QPoint(normX * (puzzle.width() - 1) + 0.5f,
                                                                                     normY * (puzzle.height() - 1) + 0.5f)))) / 255 : 1.f);
                    }

                    //knowledge of not transporant pixels
                    const float ebs = 0.0001f;
                    if(fabs(alphaMixVal - 1.f) < ebs){
                        numPixelTransparent++;
                    }


                    assert(0 < nextLinePointY_2_3 && puzzleArea.height() > nextLinePointY_2_3);

                    puzzleArea.setPixel(QPoint(i, nextLinePointY_2_3),
                            qRgb((1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_2_3)).red() +
                            alphaMixVal * QColor(color).red(),
                            (1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_2_3)).green() +
                            alphaMixVal * QColor(color).green(),
                            (1 - alphaMixVal) * QColor(puzzleArea.pixel(i, nextLinePointY_2_3)).blue() +
                            alphaMixVal * QColor(color).blue()));
                    numPixelTriangle++;
                }
                isFilled = true;
            }
            // draw next point at 2_3 line
            // wait if another line is behind (Y coordinates)
            else if(nextLinePointY_2_3 <= nextLinePointY_1_3 && (nextLinePointX_2_3 != scaledThirdX || nextLinePointY_2_3 < scaledThirdY)){
                assert(0 < nextLinePointY_2_3 && puzzleArea.height() > nextLinePointY_2_3);

                puzzleArea.setPixel(QPoint(nextLinePointX_2_3, nextLinePointY_2_3), QColor(Qt::black).rgb());
                numPixelBorder++;

                int curErr = error_2_3 * 2;
                if(curErr > -(scaledThirdY - scaledSecondY )){
                    nextLinePointX_2_3+= (scaledSecondX < scaledThirdX ? 1: -1);
                    error_2_3 -= (scaledThirdY - scaledSecondY );
                }
                if(curErr < abs(scaledSecondX - scaledThirdX)){
                    nextLinePointY_2_3++;
                    error_2_3 += abs(scaledSecondX - scaledThirdX);
                    isFilled = false;
                }
            }
            // draw next point at 1_3 line
            // wait if another line is behind (Y coordinates)
            else if(nextLinePointY_1_3 <= nextLinePointY_2_3 && (nextLinePointX_1_3 != scaledThirdX || nextLinePointY_1_3 < scaledThirdY)){
                assert(0 < nextLinePointY_1_3 && puzzleArea.height() > nextLinePointY_1_3);

                puzzleArea.setPixel(QPoint(nextLinePointX_1_3, nextLinePointY_1_3), QColor(Qt::black).rgb());
                numPixelBorder++;

                int curErr = error_1_3 * 2;
                if(curErr > -( scaledThirdY - scaledFirstY )){
                    nextLinePointX_1_3 += (scaledFirstX < scaledThirdX ? 1: -1);
                    error_1_3 -= (scaledThirdY - scaledFirstY );
                }
                if(curErr < abs(scaledFirstX - scaledThirdX )){
                    nextLinePointY_1_3++;
                    error_1_3 += abs(scaledFirstX - scaledThirdX);
                    isFilled = false;
                }
            }
            else{
                break;
            }
        }
        assert(l < DEADLINE);

        model->setPixelBorder(numPixelBorder);
        model->setPixelTriangle(numPixelTriangle + numPixelBorder);
        model->setPixelTransparent(numPixelTransparent);
    }
    update();
}

QRgb PuzzleWindow::makeFilter(QPointF point2Filter, int &alpha){
    float newCoordX = point2Filter.x() * (puzzle.width() - 1);
    int roundedNewCoordX = static_cast<int>(newCoordX);
    if(roundedNewCoordX == (puzzle.width() - 1)) {roundedNewCoordX --;}
    float shiftX = newCoordX - roundedNewCoordX;

    float newCoordY = point2Filter.y() * (puzzle.height() - 1);
    int roundedNewCoordY = static_cast<int>(newCoordY);
    if(roundedNewCoordY == (puzzle.height() - 1)){roundedNewCoordY--;}
    float shiftY = newCoordY - roundedNewCoordY;

    int red = QColor(puzzle.pixel(roundedNewCoordX , roundedNewCoordY)).red() * (1 - shiftX) * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX + 1, roundedNewCoordY)).red() *  shiftX * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX + 1 , roundedNewCoordY + 1)).red() * shiftX * shiftY +
              QColor(puzzle.pixel(roundedNewCoordX  , roundedNewCoordY + 1)).red() * (1 - shiftX) * shiftY;
    int green = QColor(puzzle.pixel(roundedNewCoordX , roundedNewCoordY)).green() * (1 - shiftX) * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX +1 , roundedNewCoordY )).green() *  shiftX * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX + 1 , roundedNewCoordY + 1)).green() * shiftX * shiftY +
              QColor(puzzle.pixel(roundedNewCoordX , roundedNewCoordY +1 )).green() * (1 - shiftX) * shiftY;
    int blue = QColor(puzzle.pixel(roundedNewCoordX , roundedNewCoordY)).blue() * (1 - shiftX) * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX +1, roundedNewCoordY )).blue() *  shiftX * (1 - shiftY) +
              QColor(puzzle.pixel(roundedNewCoordX + 1 , roundedNewCoordY + 1)).blue() * shiftX * shiftY +
              QColor(puzzle.pixel(roundedNewCoordX  , roundedNewCoordY + 1 )).blue() * (1 - shiftX) * shiftY;
    alpha = qAlpha(puzzle.pixel(roundedNewCoordX , roundedNewCoordY)) * (1 - shiftX) * (1 - shiftY) +
            qAlpha(puzzle.pixel(roundedNewCoordX +1, roundedNewCoordY )) *  shiftX * (1 - shiftY) +
            qAlpha(puzzle.pixel(roundedNewCoordX + 1 , roundedNewCoordY + 1)) * shiftX * shiftY +
            qAlpha(puzzle.pixel(roundedNewCoordX  , roundedNewCoordY + 1 )) * (1 - shiftX) * shiftY;
    return qRgb(red, green, blue);
}

