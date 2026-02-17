// filemanager.cpp
#include "filemanager.h"
#include "editorwidget.h" // Need the full declaration for document()
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog> 
#include <QFileDialog>
#include <QDir>
#include <QApplication>
#include <cmark.h>

FileManager::FileManager(QObject *parent) : QObject(parent)
{

}

QString FileManager::convertMarkdownToHtml(const QString &markdown)
{
    QByteArray utf8 = markdown.toUtf8();
    int options = CMARK_OPT_DEFAULT | CMARK_OPT_SMART | CMARK_OPT_VALIDATE_UTF8; // Enable smart quotes and UTF-8 validation
    char *html = cmark_markdown_to_html(utf8.constData(), utf8.size(), options);
    if (!html) {
        return QString();
    }
    QString result = QString::fromUtf8(html);
    free(html);
    return result;
}

bool FileManager::loadFile(const QString &fileName, EditorWidget *editor)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, tr("Scriber"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    editor->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    // editor->setCurrentFile(fileName); // If EditorWidget had this method
    // MainWindow should handle setting currentFile
    return true;
}

bool FileManager::saveFile(const QString &fileName, EditorWidget *editor)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, tr("Scriber"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << editor->toPlainText(); // Save raw Markdown
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    editor->document()->setModified(false);
    // MainWindow should handle setting currentFile
    return true;
}

bool FileManager::exportToHtml(const QString &fileName, EditorWidget *editor)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, tr("Scriber"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    
    QString htmlContent = convertMarkdownToHtml(editor->toPlainText());
    
    // Wrap in a basic HTML structure
    out << "<!DOCTYPE html>\n";
    out << "<html>\n<head>\n";
    out << "<meta charset=\"utf-8\">\n";
    out << "<title>" << QFileInfo(fileName).baseName() << "</title>\n";
    out << "<style>\n";
    out << "body { font-family: sans-serif; line-height: 1.6; max-width: 800px; margin: 0 auto; padding: 2rem; }\n";
    out << "pre { background-color: #f4f4f4; padding: 1em; border-radius: 4px; overflow-x: auto; }\n";
    out << "code { background-color: #f4f4f4; padding: 0.2em 0.4em; border-radius: 3px; }\n";
    out << "blockquote { border-left: 4px solid #ddd; margin: 0; padding-left: 1em; color: #666; }\n";
    out << "table { border-collapse: collapse; width: 100%; margin: 1em 0; }\n";
    out << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    out << "th { background-color: #f4f4f4; }\n";
    out << "</style>\n";
    out << "</head>\n<body>\n";
    out << htmlContent;
    out << "\n</body>\n</html>";

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
     return true;
}

bool FileManager::exportToPdf(const QString &fileName, EditorWidget *editor)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    // Set default PDF options if needed
    // printer.setPageSize(QPageSize(QPageSize::A4));
    // printer.setPageOrientation(QPageLayout::Portrait);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    
    QString htmlContent = convertMarkdownToHtml(editor->toPlainText());
    
    // Use a temporary QTextDocument to render the HTML for printing
    QTextDocument doc;
    // Add some basic styling for the PDF rendering
    QString styledHtml = QString(
        "<html><head><style>"
        "body { font-family: sans-serif; }"
        "pre { background-color: #f4f4f4; padding: 10px; }"
        "code { background-color: #f4f4f4; }"
        "blockquote { border-left: 4px solid #ddd; padding-left: 10px; color: #666; }"
        "</style></head><body>%1</body></html>").arg(htmlContent);
        
    doc.setHtml(styledHtml);
    doc.print(&printer);

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return true;
}
