// filemanager.h
#pragma once

#include <QObject>
#include <QString>
class EditorWidget;
class QTextDocument;

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);

    bool loadFile(const QString &fileName, EditorWidget *editor);
    bool saveFile(const QString &fileName, EditorWidget *editor);
    bool exportToHtml(const QString &fileName, EditorWidget *editor);
    bool exportToPdf(const QString &fileName, EditorWidget *editor);

signals:

private:
    // Helper functions if needed
};