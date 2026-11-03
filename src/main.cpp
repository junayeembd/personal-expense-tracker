#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PersonalProjects");
    QCoreApplication::setApplicationName("PersonalExpenseTracker");
    QCoreApplication::setApplicationVersion("2.1.0");

    // Store the JSON data beside the project when running from build/.
    // If the expected project data folder is not writable, use the OS app-data folder.
    const QString executableDir = QCoreApplication::applicationDirPath();
    QString dataDirectory = QDir(executableDir).absoluteFilePath("../data");
    if (!QDir().mkpath(dataDirectory)) {
        dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDirectory);
    }

    MainWindow window(dataDirectory);
    window.show();
    return app.exec();
}
