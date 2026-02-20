#include "findbarwidget.h"
#include "editorwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>

FindBarWidget::FindBarWidget(QWidget *parent)
    : QWidget(parent)
    , currentEditor(nullptr)
    , m_isFindBarVisible(false)
{
    setObjectName("findBar");
    createLayout();
    
    // Connect signals
    connect(findLineEdit, &QLineEdit::returnPressed, this, &FindBarWidget::onFindNext);
    connect(findNextButton, &QPushButton::clicked, this, &FindBarWidget::onFindNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &FindBarWidget::onFindPrevious);
    connect(closeButton, &QPushButton::clicked, this, &FindBarWidget::onClose);
    connect(findLineEdit, &QLineEdit::textEdited, this, &FindBarWidget::onFindTextEdited);
}

void FindBarWidget::setEditor(EditorWidget *editor)
{
    currentEditor = editor;
}

void FindBarWidget::showFindBar()
{
    setVisible(true);
    m_isFindBarVisible = true;
    findLineEdit->setFocus();
    findLineEdit->selectAll();
    emit findBarHidden(); // Signal that focus changed
}

void FindBarWidget::hideFindBar()
{
    hide();
    m_isFindBarVisible = false;
    emit findBarHidden();
}

void FindBarWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideFindBar();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void FindBarWidget::onFindNext()
{
    if (!currentEditor) return;
    
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) {
        updateFindStatus(false);
        return;
    }
    
    Qt::CaseSensitivity caseSensitivity = caseSensitiveCheckBox->isChecked() 
        ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool wholeWords = wholeWordsCheckBox->isChecked();
    
    QTextDocument::FindFlags flags;
    if (caseSensitivity == Qt::CaseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    
    bool found = false;
    QTextCursor cursor;
    
    if (wholeWords) {
        QRegularExpression regex("\\b" + QRegularExpression::escape(searchText) + "\\b",
            caseSensitivity == Qt::CaseSensitive 
                ? QRegularExpression::NoPatternOption 
                : QRegularExpression::CaseInsensitiveOption);
        cursor = currentEditor->document()->find(regex, currentEditor->textCursor());
        found = !cursor.isNull();
    } else {
        found = currentEditor->find(searchText, flags);
        cursor = currentEditor->textCursor();
    }
    
    if (found) {
        currentEditor->setTextCursor(cursor);
        currentEditor->ensureCursorVisible();
    }
    
    updateFindStatus(found);
}

void FindBarWidget::onFindPrevious()
{
    if (!currentEditor) return;
    
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) {
        updateFindStatus(false);
        return;
    }
    
    Qt::CaseSensitivity caseSensitivity = caseSensitiveCheckBox->isChecked() 
        ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool wholeWords = wholeWordsCheckBox->isChecked();
    
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (caseSensitivity == Qt::CaseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    
    bool found = false;
    
    if (wholeWords) {
        QRegularExpression regex("\\b" + QRegularExpression::escape(searchText) + "\\b",
            caseSensitivity == Qt::CaseSensitive 
                ? QRegularExpression::NoPatternOption 
                : QRegularExpression::CaseInsensitiveOption);
        QTextCursor cursor = currentEditor->textCursor();
        cursor = currentEditor->document()->find(regex, cursor, flags);
        found = !cursor.isNull();
        if (found) {
            currentEditor->setTextCursor(cursor);
        }
    } else {
        found = currentEditor->find(searchText, flags);
    }
    
    updateFindStatus(found);
}

void FindBarWidget::onFindTextEdited()
{
    onFindNext();
}

void FindBarWidget::onClose()
{
    hideFindBar();
}

void FindBarWidget::updateFindStatus(bool found)
{
    if (findLineEdit->text().isEmpty()) {
        findStatusLabel->setText("");
    } else {
        findStatusLabel->setText(found ? tr("Found") : tr("Not found"));
    }
}

void FindBarWidget::createLayout()
{
    QLabel *findLabel = new QLabel(tr("Find:"));
    findLineEdit = new QLineEdit();
    findStatusLabel = new QLabel();
    findStatusLabel->setMinimumWidth(150);
    findStatusLabel->setStyleSheet("QLabel { color : gray; }");
    
    caseSensitiveCheckBox = new QCheckBox(tr("Case sensitive"));
    wholeWordsCheckBox = new QCheckBox(tr("Whole words"));
    
    findNextButton = new QPushButton(tr("Next"));
    findPreviousButton = new QPushButton(tr("Previous"));
    closeButton = new QPushButton();
    closeButton->setFixedSize(24, 24);
    QIcon closeIcon = QIcon::fromTheme("window-close", QIcon::fromTheme("dialog-close"));
    if (!closeIcon.isNull()) {
        closeButton->setIcon(closeIcon);
    } else {
        closeButton->setText(tr("×"));
    }
    closeButton->setToolTip(tr("Close find bar (Escape)"));
    
    QHBoxLayout *optionsLayout = new QHBoxLayout;
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addStretch();
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(findStatusLabel);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(findPreviousButton);
    buttonsLayout->addWidget(closeButton);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(findLabel);
    topLayout->addWidget(findLineEdit);
    topLayout->addLayout(optionsLayout);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(buttonsLayout);
}
