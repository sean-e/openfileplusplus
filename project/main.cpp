#include <QApplication>
#include <QSystemTrayIcon>
#include "AppWindow.h"
#include "../setup/OpenFileVersion.txt"


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(OpenFile);

	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication app(argc, argv);
	QCoreApplication::setApplicationName("OpenFile++");
	QCoreApplication::setOrganizationName("Fester");
	QCoreApplication::setApplicationVersion(OpenFileAppVersion);

#ifndef QT_NO_SYSTEMTRAYICON
// 	if (QSystemTrayIcon::isSystemTrayAvailable())
// 		QApplication::setQuitOnLastWindowClosed(false);
#endif

	AppWindow window;
	return app.exec();
}
