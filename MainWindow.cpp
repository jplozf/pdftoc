#include "MainWindow.h"
#include "ui_MainWindow.h"

// ****************************************************************************
// Constants
// ****************************************************************************
const QString APP_NAME = "PDFToC";
const QString DUMP_FILE = "dump.txt";
const QString DUMP_NO_BOOKMARK = "dump_no_bookmark.txt";
const QString DUMP_TEMP_FILE = "dump_temp.txt";
const int NOTHING = -42;
const int MESSAGE_TIMEOUT = 3000;

// ****************************************************************************
// MainWindow::MainWindow()
// ****************************************************************************
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(APP_NAME);
    // APP_FOLDER
    QDir appDir = QDir(QDir::homePath()).filePath(".pdftoc");
    if (!appDir.exists())
        appDir.mkpath(".");
    this->appDir = appDir;
    this->DumpFile = this->appDir.filePath(DUMP_FILE);
    this->currentBookmarkIndex = NOTHING;
    ui->btnDeleteBookmark->setEnabled(false);
    ui->btnInsertBookmark->setEnabled(false);
    ui->btnSaveBookmark->setEnabled(false);
    ui->btnUpBookmark->setEnabled(false);
    ui->btnDownBookmark->setEnabled(false);
    ui->btnSave->setEnabled(false);
    ui->action_Save->setEnabled(false);
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(slotDoExit()));
    //
    ui->btnSaveBookmark->setShortcut(QKeySequence("F2"));
    ui->btnSaveBookmark->setStatusTip("F2 : Update bookmark");
    ui->btnInsertBookmark->setShortcut(QKeySequence("F3"));
    ui->btnInsertBookmark->setStatusTip("F3 : Insert bookmark");
    ui->btnDeleteBookmark->setShortcut(QKeySequence("F8"));
    ui->btnDeleteBookmark->setStatusTip("F8 : Delete bookmark");
    //
    this->lblDirty = new QLabel();
    this->lblDirty->setIndent(10);
    ui->statusbar->addPermanentWidget(this->lblDirty);
    this->lblBookmarks = new QLabel("0 bookmark(s)");
    this->lblBookmarks->setIndent(10);
    ui->statusbar->addPermanentWidget(this->lblBookmarks);
    //
    showMessage("Welcome");
}

// ****************************************************************************
// MainWindow::MainWindow()
// ****************************************************************************
MainWindow::~MainWindow()
{
    delete ui;
}

// ****************************************************************************
// MainWindow::MainWindow()
// ****************************************************************************
void MainWindow::on_btnLoadFile_clicked()
{
    QString homeDir = QDir::homePath();
    this->PDFFile = QFileDialog::getOpenFileName(this,
                                                 tr("Open PDF File"),
                                                 homeDir,
                                                 tr("PDF Files (*.pdf)"));
    if (this->PDFFile != "") {
        ui->txtFileName->setText(this->PDFFile);
        this->bookmarks.clear();
        this->GetBookmarks(this->PDFFile);
        QDesktopServices::openUrl(QUrl::fromLocalFile(this->PDFFile));
        showMessage(this->PDFFile + " open");
        ui->btnSave->setEnabled(true);
        ui->action_Save->setEnabled(true);
        this->dirty = false;
    }
}

// ****************************************************************************
// MainWindow::on_btnSave_clicked()
// ****************************************************************************
void MainWindow::on_btnSave_clicked()
{
    this->SaveBookmarks();
}

// ****************************************************************************
// MainWindow::on_btnSaveBookmark_clicked()
// ****************************************************************************
void MainWindow::on_btnSaveBookmark_clicked()
{
    this->dirty = true;
    if (this->currentBookmarkIndex != NOTHING) {
        this->bookmarks[this->currentBookmarkIndex].title = ui->txtBookmark->text();
        this->bookmarks[this->currentBookmarkIndex].page = ui->txtPage->text().toInt();
        this->bookmarks[this->currentBookmarkIndex].level = ui->txtIndent->text().toInt();
        this->RefreshBookmarks();
        showMessage("Bookmark updated");
    } else {
        Bookmark bm;
        bm.title = ui->txtBookmark->text();
        bm.page = ui->txtPage->text().toInt();
        bm.level = ui->txtIndent->text().toInt();
        this->bookmarks.append(bm);
        this->RefreshBookmarks();
        showMessage("Bookmark added");
    }
    ui->txtBookmark->selectAll();
    ui->txtBookmark->setFocus();
}

// ****************************************************************************
// MainWindow::on_btnUpBookmark_clicked()
// ****************************************************************************
void MainWindow::on_btnUpBookmark_clicked()
{
    this->dirty = true;
    std::swap(this->bookmarks[this->currentBookmarkIndex - 1],
              this->bookmarks[this->currentBookmarkIndex]);
    this->currentBookmarkIndex--;
    this->RefreshBookmarks();
    showMessage("Bookmark moved up");
}

// ****************************************************************************
// MainWindow::on_btnDownBookmark_clicked()
// ****************************************************************************
void MainWindow::on_btnDownBookmark_clicked()
{
    this->dirty = true;
    std::swap(this->bookmarks[this->currentBookmarkIndex + 1],
              this->bookmarks[this->currentBookmarkIndex]);
    this->currentBookmarkIndex++;
    this->RefreshBookmarks();
    showMessage("Bookmark moved down");
}

// ****************************************************************************
// MainWindow::on_btnDeleteBookmark_clicked()
// ****************************************************************************
void MainWindow::on_btnDeleteBookmark_clicked()
{
    if (this->currentBookmarkIndex != NOTHING) {
        this->dirty = true;
        this->bookmarks.remove(this->currentBookmarkIndex);
        if (this->currentBookmarkIndex > 0) {
            this->currentBookmarkIndex--;
        } else {
            this->currentBookmarkIndex = NOTHING;
        }
        this->RefreshBookmarks();
        showMessage("Bookmark deleted");
    }
}

// ****************************************************************************
// MainWindow::GetBookmarks()
// ****************************************************************************
void MainWindow::GetBookmarks(QString fPDF)
{
    QProcess *myProcess = new QProcess(this);
    QStringList args = {fPDF, "dump_data", "output", this->DumpFile};
    qDebug() << args;
    myProcess->execute("pdftk", args);
    //
    this->fDump = new QFile(this->DumpFile);
    this->fDump->open(QIODevice::ReadOnly);

    ui->txtTitle->setText(GetInfoValue(fDump, "Title"));
    ui->lblPages->setText(GetVariable(fDump, "NumberOfPages"));
    this->lblBookmarks->setText(QString::number(GetAllBookmarks(fDump)) + " bookmark(s)");

    ui->treeWidget->setColumnCount(3);
    QStringList headerLabels;
    headerLabels.push_back("Page");
    headerLabels.push_back("Level");
    headerLabels.push_back("Title");
    ui->treeWidget->setHeaderLabels(headerLabels);
    // this->currentBookmarkIndex = 0;
    this->RefreshBookmarks();

    this->fDump->close();
}

// ****************************************************************************
// MainWindow::RefreshBookmarks()
// ****************************************************************************
int MainWindow::RefreshBookmarks()
{
    ui->treeWidget->clear();
    for (int i = 0; i < this->bookmarks.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString::number(bookmarks[i].page));
        item->setText(1, QString::number(bookmarks[i].level));
        item->setText(2, bookmarks[i].title);
        ui->treeWidget->addTopLevelItem(item);
        ui->btnUpBookmark->setEnabled(true);
        ui->btnDownBookmark->setEnabled(true);
        if (i == this->currentBookmarkIndex) {
            qDebug() << "Selected Item #" << i;
            ui->treeWidget->setCurrentItem(item);
            // ui->treeWidget->currentItem()->setSelected(true);
            if (i == 0) {
                ui->btnUpBookmark->setEnabled(false);
            }
            if (i == this->bookmarks.size() - 1) {
                ui->btnDownBookmark->setEnabled(false);
            }
        }
    }
    this->lblBookmarks->setText(QString::number(this->bookmarks.size()) + " bookmark(s)");
    if (this->dirty) {
        this->lblDirty->setText("*modified*");
    } else {
        this->lblDirty->setText("");
    }

    return this->bookmarks.size();
}

// ****************************************************************************
// MainWindow::GetVariable()
// ****************************************************************************
QString MainWindow::GetVariable(QFile *fDump, QString var)
{
    QTextStream in(fDump);
    QString val("*NONE");
    in.seek(0);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains(var)) {
            qDebug() << line;
            int idx = line.indexOf(":");
            val = line.mid(idx + 1).trimmed();
        }
    }
    return val;
}

// ****************************************************************************
// MainWindow::GetInfoValue()
// ****************************************************************************
QString MainWindow::GetInfoValue(QFile *fDump, QString key)
{
    QTextStream in(fDump);
    QString val("*NONE");
    in.seek(0);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("InfoKey: " + key)) {
            QString l2 = in.readLine();
            qDebug() << l2;
            int idx = l2.indexOf(":");
            val = l2.mid(idx + 1).trimmed();
        }
    }
    return val;
}

// ****************************************************************************
// MainWindow::GetAllBookmarks()
// ****************************************************************************
int MainWindow::GetAllBookmarks(QFile *fDump)
{
    int rc = 0;
    bool readEnabled = false;
    QString outText;
    QString newText;
    QTextStream in(fDump);
    in.seek(0);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("BookmarkBegin")) {
            outText = line + "\n";
            readEnabled = true;
        }
        if (readEnabled) {
            outText += line + "\n";
        }
        if (!readEnabled) {
            newText += line + "\n";
        }
        if (line.contains("BookmarkPageNumber:")) {
            readEnabled = false;
            // We have all we need, let's process it...
            int startPos = outText.indexOf("BookmarkTitle:") + QString("BookmarkTitle:").length();
            int endPos = outText.indexOf("BookmarkLevel:");
            int length = endPos - startPos;
            Bookmark bm;
            // Title
            bm.title = outText.mid(startPos, length).trimmed();
            // Looking for the remaining parts...
            QTextStream stream(&outText);
            QString line;
            while (stream.readLineInto(&line)) {
                // Level
                if (line.contains("BookmarkLevel:")) {
                    int idx = line.indexOf(":");
                    bm.level = line.mid(idx + 1).trimmed().toInt();
                }
                // Page
                if (line.contains("BookmarkPageNumber:")) {
                    int idx = line.indexOf(":");
                    bm.page = line.mid(idx + 1).trimmed().toInt();
                }
            }
            qDebug() << "Title :" << bm.title;
            qDebug() << "Level :" << bm.level;
            qDebug() << "Page :" << bm.page;
            this->bookmarks.append(bm);
            rc++;
        }
    }
    // Write temporary dump file without bookmarks
    QFile file = this->appDir.filePath(DUMP_NO_BOOKMARK);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&file);
        stream << newText;
        file.close();
    }
    return rc;
}

// ****************************************************************************
// MainWindow::on_treeWidget_itemClicked()
// ****************************************************************************
void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    qDebug() << "ITEM_SELECTION_CHANGED";
    this->currentBookmarkIndex = ui->treeWidget->currentIndex().row();
    //QTreeWidgetItem *item = ui->treeWidget->currentItem();
    ui->txtBookmark->setText(item->text(2));
    ui->txtIndent->setText(item->text(1));
    ui->txtPage->setText(item->text(0));

    ui->btnUpBookmark->setEnabled(true);
    ui->btnDownBookmark->setEnabled(true);
    if (this->currentBookmarkIndex == 0) {
        ui->btnUpBookmark->setEnabled(false);
    }
    if (this->currentBookmarkIndex == this->bookmarks.size() - 1) {
        ui->btnDownBookmark->setEnabled(false);
    }
}

// ****************************************************************************
// MainWindow::SaveBookmarks()
// ****************************************************************************
void MainWindow::SaveBookmarks()
{
    this->UpdateDumpFile();
    this->UpdateTitle();
    QDateTime now = QDateTime::currentDateTime();
    QString outFile = QFileInfo(this->PDFFile).absolutePath() + QDir::separator()
                      + QFileInfo(this->PDFFile).baseName() + now.toString(".yyyyMMdd-hhmmss")
                      + ".pdf";
    QProcess *myProcess = new QProcess(this);
    QStringList args = {this->PDFFile,
                        "update_info",
                        this->appDir.filePath(DUMP_TEMP_FILE),
                        "output",
                        outFile};
    qDebug() << args;
    myProcess->execute("pdftk", args);
    QDesktopServices::openUrl(QUrl::fromLocalFile(outFile));
    showMessage(outFile + " created");
    this->dirty = false;
    this->RefreshBookmarks();
}

// ****************************************************************************
// MainWindow::UpdateDumpFile()
// ****************************************************************************
void MainWindow::UpdateDumpFile()
{
    QString bmText;
    // Clean up of any previous Dump Temp File to prevent to add bookmarks again and again
    QFile::remove(this->appDir.filePath(DUMP_TEMP_FILE));
    // Get a clean Dump Temp File
    QFile::copy(this->appDir.filePath(DUMP_NO_BOOKMARK), this->appDir.filePath(DUMP_TEMP_FILE));
    QFile file = this->appDir.filePath(DUMP_TEMP_FILE);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream dumpText(&file);
        QTreeWidgetItemIterator it(ui->treeWidget);
        while (*it) {
            Bookmark bm;
            bm.page = (*it)->text(0).toInt();
            bm.level = (*it)->text(1).toInt();
            bm.title = (*it)->text(2);
            bmText
                = QString("BookmarkBegin\nBookmarkTitle: %1\nBookmarkLevel: " "%2\nBookmarkPageN" "umber: %3\n")
                      .arg(bm.title)
                      .arg(bm.level)
                      .arg(bm.page);
            dumpText << bmText;
            ++it;
        }
        file.close();
    }
}

// ****************************************************************************
// MainWindow::UpdateTitle()
// ****************************************************************************
void MainWindow::UpdateTitle()
{
    QString s = "";
    bool found = false;
    QFile file = this->appDir.filePath(DUMP_TEMP_FILE);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream dumpText(&file);
        while (!dumpText.atEnd()) {
            QString line = dumpText.readLine();
            if (line.contains("InfoKey: Title")) {
                found = true;
                s += line + "\n";
                s += "InfoValue: " + ui->txtTitle->text().trimmed() + "\n";
                line = dumpText.readLine(); // dummy read to skip the next line
            } else {
                s += line + "\n";
            }
        }
        if (!found) {
            s += "InfoBegin\n";
            s += "InfoKey: Title\n";
            s += "InfoValue: " + ui->txtTitle->text().trimmed() + "\n";
        }
        // qDebug() << s;
        file.resize(0);
        file.write(s.toUtf8());
        file.close();
    }
}

// ****************************************************************************
// MainWindow::showMessage()
// ****************************************************************************
void MainWindow::showMessage(const QString txt)
{
    ui->statusbar->showMessage(txt, MESSAGE_TIMEOUT);
}

// ****************************************************************************
// MainWindow::on_btnInsertBookmark_clicked()
// ****************************************************************************
void MainWindow::on_btnInsertBookmark_clicked()
{
    Bookmark bm;
    this->dirty = true;
    bm.title = ui->txtBookmark->text();
    bm.page = ui->txtPage->text().toInt();
    bm.level = ui->txtIndent->text().toInt();
    if (this->currentBookmarkIndex == NOTHING) {
        this->bookmarks.insert(0, bm);
        this->currentBookmarkIndex = 0;
    } else {
        this->bookmarks.insert(this->currentBookmarkIndex + 1, bm);
        this->currentBookmarkIndex++;
    }
    this->RefreshBookmarks();
    ui->txtBookmark->selectAll();
    ui->txtBookmark->setFocus();
}

// ****************************************************************************
// MainWindow::on_txtBookmark_textChanged()
// ****************************************************************************
void MainWindow::on_txtBookmark_textChanged(const QString &arg1)
{
    if (ui->txtBookmark->text().isEmpty() || ui->txtIndent->text().isEmpty()
        || ui->txtPage->text().isEmpty()) {
        ui->btnDeleteBookmark->setEnabled(false);
        ui->btnInsertBookmark->setEnabled(false);
        ui->btnSaveBookmark->setEnabled(false);
        ui->btnUpBookmark->setEnabled(false);
        ui->btnDownBookmark->setEnabled(false);
    } else {
        ui->btnDeleteBookmark->setEnabled(true);
        ui->btnInsertBookmark->setEnabled(true);
        ui->btnSaveBookmark->setEnabled(true);
        ui->btnUpBookmark->setEnabled(true);
        ui->btnDownBookmark->setEnabled(true);
        ui->btnSaveBookmark->setShortcut(QKeySequence(Qt::Key_F2));
    }
}

// ****************************************************************************
// MainWindow::on_txtPage_textChanged()
// ****************************************************************************
void MainWindow::on_txtPage_textChanged(const QString &arg1)
{
    if (ui->txtBookmark->text().isEmpty() || ui->txtIndent->text().isEmpty()
        || ui->txtPage->text().isEmpty()) {
        ui->btnDeleteBookmark->setEnabled(false);
        ui->btnInsertBookmark->setEnabled(false);
        ui->btnSaveBookmark->setEnabled(false);
        ui->btnUpBookmark->setEnabled(false);
        ui->btnDownBookmark->setEnabled(false);
    } else {
        ui->btnDeleteBookmark->setEnabled(true);
        ui->btnInsertBookmark->setEnabled(true);
        ui->btnSaveBookmark->setEnabled(true);
        ui->btnUpBookmark->setEnabled(true);
        ui->btnDownBookmark->setEnabled(true);
    }
}

// ****************************************************************************
// MainWindow::on_txtIndent_textChanged()
// ****************************************************************************
void MainWindow::on_txtIndent_textChanged(const QString &arg1)
{
    if (ui->txtBookmark->text().isEmpty() || ui->txtIndent->text().isEmpty()
        || ui->txtPage->text().isEmpty()) {
        ui->btnDeleteBookmark->setEnabled(false);
        ui->btnInsertBookmark->setEnabled(false);
        ui->btnSaveBookmark->setEnabled(false);
        ui->btnUpBookmark->setEnabled(false);
        ui->btnDownBookmark->setEnabled(false);
    } else {
        ui->btnDeleteBookmark->setEnabled(true);
        ui->btnInsertBookmark->setEnabled(true);
        ui->btnSaveBookmark->setEnabled(true);
        ui->btnUpBookmark->setEnabled(true);
        ui->btnDownBookmark->setEnabled(true);
    }
}

// ****************************************************************************
// MainWindow::on_action_Save_triggered()
// ****************************************************************************
void MainWindow::on_action_Save_triggered()
{
    this->SaveBookmarks();
}

// ****************************************************************************
// MainWindow::on_action_Quit_triggered()
// ****************************************************************************
void MainWindow::on_action_Quit_triggered() {}

// ****************************************************************************
// MainWindow::on_action_Open_triggered()
// ****************************************************************************
void MainWindow::on_action_Open_triggered()
{
    this->on_btnLoadFile_clicked();
}

// *****************************************************************************
// MainWindow::slotDoExit()
// *****************************************************************************
void MainWindow::slotDoExit()
{
    this->close();
}

// *****************************************************************************
// MainWindow::closeEvent()
// *****************************************************************************
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (this->dirty) {
        QMessageBox::StandardButton rc;
        rc = QMessageBox::question(
            this,
            APP_NAME,
            QString("Table Of Contents has been modified.\nDo you want to save it ?\n"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (rc == QMessageBox::Cancel) {
            showMessage("Cancel exit");
            event->ignore();
        } else {
            if (rc == QMessageBox::Yes) {
                this->SaveBookmarks();
                showMessage("Exiting");
                event->accept();
            }
        }
    }
}

// *****************************************************************************
// MainWindow::on_txtTitle_editingFinished()
// *****************************************************************************
void MainWindow::on_txtTitle_editingFinished()
{
    this->dirty = true;
    this->RefreshBookmarks();
}
