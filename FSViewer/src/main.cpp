#include "FSViewer.h"
#include <QtWidgets/QApplication>

//#include <QTextCodec>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//QTextCodec *codec = QTextCodec::codecForName("CP1251");
	//QTextCodec::setCodecForLocale(codec);
	FSViewer w;
	w.show();
	return a.exec();
}
