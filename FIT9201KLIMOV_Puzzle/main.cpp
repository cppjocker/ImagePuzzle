#include <QtGui/QApplication>
#include "puzzlewindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PuzzleWindow w;
    w.show();
    a.installEventFilter(&w);
    return a.exec();
}
