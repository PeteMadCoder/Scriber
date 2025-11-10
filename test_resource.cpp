// test_resource.cpp
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication> // Need this for Q_INIT_RESOURCE if linking statically
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // Initialize Qt

    // --- Method 1: Try opening the resource file ---
    QFile file(":/resources/themes/dark.css");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qCritical() << "CRITICAL: Could not open ':/themes/light.css'";
         // --- Method 2: List contents of resource root ---
         QDir resRoot(":/");
         qInfo() << "Contents of resource root (:/):" << resRoot.entryList();
         // --- Method 3: Try a non-existent file to confirm error path ---
         QFile dummy(":/this/does/not/exist.txt");
         if (!dummy.open(QIODevice::ReadOnly)) {
             qInfo() << "Confirmed error opening non-existent file:" << dummy.errorString();
         }
         return -1;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    qDebug() << "SUCCESS: Opened and read ':/themes/light.css'";
    qDebug() << "First 200 characters:";
    qDebug().noquote() << content.left(200); // Print first 200 chars without quotes

    return 0;
}