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

FileManager::FileManager(QObject *parent) : QObject(parent)
{

}

bool FileManager::loadFile(const QString &fileName, EditorWidget *editor)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, tr("Inkflow"),
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
        QMessageBox::warning(nullptr, tr("Inkflow"),
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
        QMessageBox::warning(nullptr, tr("Inkflow"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    // Simple approach: Save the plain text as HTML-escaped content inside basic HTML
    // out << "<html><body><pre>" << editor->toPlainText().toHtmlEscaped() << "</pre></body></html>";

    // Better approach: Use QTextDocument's HTML conversion (but this converts the *formatted* view, not raw MD)
    // This requires parsing Markdown to HTML first (e.g., with cmark) and setting it on a temp document
    // Or, if using a hybrid approach where the QTextDocument *is* the rendered HTML, you get it directly.

    // For now, assuming EditorWidget contains raw Markdown:
    // You would need to integrate cmark here to convert editor->toPlainText() to HTML string
    // QString markdownText = editor->toPlainText();
    // QString htmlString = convertMarkdownToHtml(markdownText); // <-- Implement this function using cmark
    // out << htmlString;

    // Placeholder using QTextDocument's conversion (shows formatted view as HTML):
    out << editor->document()->toHtml(); // This exports the *rendered* HTML based on QTextDocument state

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
    // Similar to HTML export, this prints the *rendered* QTextDocument
    // Again, for true Markdown -> PDF, parse with cmark -> HTML -> QTextDocument -> PDF
    editor->document()->print(&printer);
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return true;
}

// QString FileManager::convertMarkdownToHtml(const QString &markdown) {
//     // Integrate cmark library here
//     // 1. Convert QString to null-terminated char* (or use cmark's UTF-8 functions)
//     // 2. Call cmark functions: cmark_parse_document(), cmark_render_html()
//     // 3. Convert result back to QString
//     // 4. Return QString
//     // Remember to free cmark memory!
//     return QString(); // Placeholder
// }
