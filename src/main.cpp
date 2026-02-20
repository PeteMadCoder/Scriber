#include <QApplication>
#include "mainwindow.h"
#include <QIcon>
#include <QCommandLineParser>
#include <QFile>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Scriber");
    app.setApplicationVersion("0.1");
    app.setApplicationDisplayName("Scriber - Markdown Editor");
    app.setWindowIcon(QIcon(":/resources/icons/appicon.png"));

    MainWindow window;

    // Process command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Distraction-free Markdown Editor");
    parser.addHelpOption();
    parser.addVersionOption();

    // Add a positional argument for the file to open
    parser.addPositionalArgument("file", "The file to open");

    // Process the actual command line arguments
    parser.process(app);

    // Check if a file was specified via positional argument
    const QStringList args = parser.positionalArguments();
    QString filePath;
    
    if (!args.isEmpty()) {
        filePath = args.first();
    } else {
        // Check regular arguments (for OS file open events)
        QStringList appArgs = app.arguments();
        if (appArgs.size() > 1) {
            filePath = appArgs.at(1);
        }
    }
    
    // Open file if specified
    if (!filePath.isEmpty()) {
        // Check if the file exists before trying to open it
        if (QFile::exists(filePath)) {
            qDebug() << "Opening file:" << filePath;
            window.openFile(filePath);
        } else {
            qWarning() << "File does not exist:" << filePath;
        }
    } else {
        // No file specified, create an empty untitled document
        window.newFile();
    }

    window.show();
    return app.exec();
}