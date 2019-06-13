#ifndef PUZZLEWINDOW_H
#define PUZZLEWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include <QTime>

#include "ui_mainwindow.h"

#include "triangleanimationmodel.h"

class PuzzleWindow : public QMainWindow, public  Ui_PuzzleWindow
{
    Q_OBJECT
public:
    explicit PuzzleWindow(QWidget *parent = 0);
    ~PuzzleWindow();
protected:
    void paintEvent(QPaintEvent*);
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *);
private slots:
    void sl_onStartDraw();
    void sl_onStopDraw();
    void sl_onInit();
    void sl_onAlphaMixChanged(int);
    void sl_onDegreeChanged(int);
    void sl_onFilterChanged(int);
    void sl_onTimeout();
    void sl_onTimeoutProgress();
private:
    void setModelTextureCoordinates();

    void setPuzzleArea();
    void getProgress(int val, bool drawImmediately);
    // calculate next animations on progress 'progress'
    void onProgress(const float progress);
    // bilinear filtaration to point 'point2Filter'
    QRgb makeFilter(QPointF point2Filter, int& alpha);
    bool isStopped;
    bool isFiltered;
    bool isAlphaMixered;
    QImage puzzleArea;
    QTimer timer;
    QImage puzzle;
    QTime lastAnimatedTime;
    QTimer timerProgress;

    QVector<float> progresses;
    QVector<QSharedPointer<TriangleAnimationModel> > models;
};

#endif // PUZZLEWINDOW_H
