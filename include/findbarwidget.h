#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

class EditorWidget;

/**
 * @brief Find bar widget for searching text in documents
 *
 * Embedded search bar that appears at the bottom of the editor
 * with support for case-sensitive and whole-word searches.
 */
class FindBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindBarWidget(QWidget *parent = nullptr);
    
    /// Set the editor to search in
    void setEditor(EditorWidget *editor);
    
    /// Show the find bar and focus the input
    void showFindBar();
    
    /// Hide the find bar
    void hideFindBar();
    
    /// Check if the find bar is visible
    bool isFindBarVisible() const { return m_isFindBarVisible; }

signals:
    void findNextRequested();
    void findPreviousRequested();
    void findTextChanged(const QString &text);
    void findBarHidden();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onFindNext();
    void onFindPrevious();
    void onFindTextEdited();
    void onClose();

private:
    void createLayout();
    void updateFindStatus(bool found);

    QLineEdit *findLineEdit;
    QLabel *findStatusLabel;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *closeButton;

    EditorWidget *currentEditor;
    bool m_isFindBarVisible;
};
