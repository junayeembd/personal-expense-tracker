#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PersonalProjects");
    QCoreApplication::setApplicationName("PersonalExpenseTracker");
    QCoreApplication::setApplicationVersion("3.0.0");

    // Store the JSON data beside the project when running from build/.
    // If the expected project data folder is not writable, use the OS app-data folder.
    const QString executableDir = QCoreApplication::applicationDirPath();
    QString dataDirectory = QDir(executableDir).absoluteFilePath("../data");
    bool dataDirectoryWritable = QDir().mkpath(dataDirectory);
    if (dataDirectoryWritable) {
        const QString probePath = QDir(dataDirectory).filePath(".write-test");
        QFile probe(probePath);
        dataDirectoryWritable = probe.open(QIODevice::WriteOnly | QIODevice::Truncate);
        if (dataDirectoryWritable) {
            probe.close();
            probe.remove();
        }
    }
    if (!dataDirectoryWritable) {
        dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (!QDir().mkpath(dataDirectory)) {
            return 1;
        }
    }

    MainWindow window(dataDirectory);
    window.show();
    return app.exec();
}
