#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "pswdmgr.h"

#include <QMainWindow>
#include <memory>
#include <crypto/SecureString.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void RedrawTable();
    ~MainWindow();

private slots:
    // File
    void OpenPswdFile(void);
    void WriteoutPswds(void);
    void WriteoutToFile(void);
    void CreateNewPswdFile(void);

    // Manage
    void AddSitePswd(void);
    void RemoveSitePswd(void);
    void ChangePswd(void);

    // Help
    void OpenAbout(void);
    void OpenHelp(void);

    // Cells
    void CellChanged(int row, int col);
    void CellEntered(int row, int col);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<pswdmgr> pswds;
    SecureString masterPswd;
    QStringList pswdColumns;
    QString openFileName;

    // Set if editing site-field, so can remove from map if edited or deleted
    QString curSite;

    void SetEnableModify(bool open);
    std::string SiteAtRow(unsigned int row);
    std::string UserAtRow(unsigned int row);
    SecureString PswdAtRow(unsigned int row);
};

#endif // MAINWINDOW_H
