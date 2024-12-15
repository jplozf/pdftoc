#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QTextDocument>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString PDFFile;
    QString DumpFile;
    QDir appDir;
    QFile *fDump;
    QLabel *lblBookmarks;
    QLabel *lblDirty;
    bool dirty = false;
    typedef struct Bookmark
    {
        QString title;
        int level;
        int page;
    };
    QVector<Bookmark> bookmarks;
    int currentBookmarkIndex;
    void closeEvent(QCloseEvent *event);

private slots:
    void slotDoExit();
    void on_btnLoadFile_clicked();
    void on_btnSave_clicked();
    void on_btnSaveBookmark_clicked();
    void on_btnUpBookmark_clicked();
    void on_btnDownBookmark_clicked();
    void on_btnDeleteBookmark_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_btnInsertBookmark_clicked();

    void on_txtBookmark_textChanged(const QString &arg1);

    void on_txtPage_textChanged(const QString &arg1);

    void on_txtIndent_textChanged(const QString &arg1);

    void on_action_Save_triggered();

    void on_action_Quit_triggered();

    void on_action_Open_triggered();

    void on_txtTitle_editingFinished();

private:
    Ui::MainWindow *ui;
    int GetBookmarks(QString fPDf);
    QString GetVariable(QFile *fDump, QString var);
    QString GetInfoValue(QFile *fDump, QString key);
    int GetAllBookmarks(QFile *fDump);
    int RefreshBookmarks();
    void SaveBookmarks();
    void UpdateDumpFile();
    void UpdateTitle();
    void showMessage(const QString txt);
};
#endif // MAINWINDOW_H
