#include "RawRecovery.h"
#include <QtWidgets/QApplication>



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);


	RawRecovery w;
	w.show();
	return a.exec();
}
