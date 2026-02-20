#include <QString>
#include <QDebug>

int main() {
    // Missing %2
    QString str = "Values: %1, %3";
    
    QString result = str
        .arg("1") // Replaces %1
        .arg("2") // Replaces %3 (next lowest)
        .arg("3"); // No placeholders left

    qDebug() << result;
    return 0;
}
