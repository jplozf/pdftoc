#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMainWindow>
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
    typedef struct Bookmark
    {
        QString title;
        int level;
        int page;
    };
    QVector<Bookmark> bookmarks;
    int currentBookmarkIndex;

private slots:
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

    void on_treeWidget_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
    void GetBookmarks(QString fPDf);
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
