#include <QString>
#include <QDebug>
#include <iostream>

int main() {
    QString v1 = "1";
    QString v2 = "2";
    QString v3 = "3";
    QString v4 = "4";
    QString v5 = "5";
    QString v6 = "6";
    QString v7 = "7";
    QString v8 = "8";
    QString v9 = "9";
    QString v10 = "10";
    QString v11 = "11";
    QString v12 = "12";
    QString v13 = "13";

    QString str = "Values: %1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13";
    
    QString result = str
        .arg(v1)
        .arg(v2)
        .arg(v3)
        .arg(v4)
        .arg(v5)
        .arg(v6)
        .arg(v7)
        .arg(v8)
        .arg(v9)
        .arg(v10)
        .arg(v11)
        .arg(v12)
        .arg(v13);

    qDebug() << result;
    return 0;
}
