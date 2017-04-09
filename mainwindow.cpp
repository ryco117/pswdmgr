#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "getpswd.h"
#include "getsiteuserpass.h"
#include "rmsitedialog.h"

#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QLineEdit>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pswdColumns << "Site" << "Username" << "Password" << "Show Pswd";
    ui->PswdsTable->setHorizontalHeaderLabels(pswdColumns);
    ui->PswdsTable->resizeColumnsToContents();
    masterPswd.Clear();
    SetEnableModify(false);

    QString homeOTR = QDir::homePath() + "/.pswds";
    QFileInfo check_file(homeOTR);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile())
    {
        GetPswd getPswd(&masterPswd, this);
        if(getPswd.exec() == QDialog::Accepted)
        {
            try
            {
                pswdmgr* pswdMgrPtr = new pswdmgr(homeOTR.toStdString(), masterPswd.GetStr());
                pswds.reset(pswdMgrPtr);
            }
            catch(std::exception& err)
            {
                masterPswd.Clear();
                pswds.reset(nullptr);
                QMessageBox msg(QMessageBox::Critical, "Error", tr(err.what()), QMessageBox::Ok, this);
                msg.exec();
                return;
            }

            RedrawTable();

            SetEnableModify(true);
            openFileName = homeOTR;
            ui->statusBar->showMessage(tr("Successfully opened password file ") + openFileName, 8000);
        }
        else
        {
            masterPswd.Clear();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RedrawTable()
{
    // Reset
    for(int i = ui->PswdsTable->rowCount()-1; i >= 0; i--)
    {
        ui->PswdsTable->removeRow(i);
    }
    unsigned int curRow = 0;

    // Fill Pswds List Structure
    for(std::map<std::string, std::shared_ptr<pswdmgr::UserPass>>::iterator siteIter = pswds->IterBegin();
        siteIter != pswds->IterEnd(); siteIter++)
    {
        // Creating a new table row
        ui->PswdsTable->insertRow(curRow);

        // Add label
        QTableWidgetItem* siteItm = new QTableWidgetItem(siteIter->first.c_str());
        ui->PswdsTable->setItem(curRow, 0, siteItm);

        // Add user
        QTableWidgetItem* userItm = new QTableWidgetItem(siteIter->second->username.c_str());
        ui->PswdsTable->setItem(curRow, 1, userItm);

        // Add password line edit widget
        QLineEdit* pswdWdgt = new QLineEdit(reinterpret_cast<char*>(siteIter->second->password.GetStr()), ui->PswdsTable);
        pswdWdgt->setEchoMode(QLineEdit::Password);
        pswdWdgt->setProperty("curRow", curRow);
        connect(pswdWdgt, SIGNAL(editingFinished()), this, SLOT(PswdChanged()));
        ui->PswdsTable->setCellWidget(curRow, 2, pswdWdgt);

        // Add show password button widget
        QWidget* pContainBttn = new QWidget();
        QHBoxLayout* pLayoutBttn = new QHBoxLayout(pContainBttn);
        QPushButton* bttnWdgt = new QPushButton("", this);
        bttnWdgt->setProperty("curRow", curRow);
        connect(bttnWdgt, SIGNAL(pressed()), this, SLOT(ShowPswdPress()));
        connect(bttnWdgt, SIGNAL(released()), this, SLOT(ShowPswdRelease()));

        pLayoutBttn->addWidget(bttnWdgt);
        pLayoutBttn->setAlignment(Qt::AlignCenter);
        pLayoutBttn->setContentsMargins(4, 4, 4, 4);
        pContainBttn->setLayout(pLayoutBttn);
        ui->PswdsTable->setCellWidget(curRow, 3, pContainBttn);


        curRow++;
    }
    ui->PswdsTable->resizeColumnsToContents();
}

void MainWindow::SetEnableModify(bool open)
{
    ui->actionAdd_Password->setEnabled(open);
    ui->actionChange_Password->setEnabled(open);
    ui->actionRemove_Site_Pswd->setEnabled(open);
    ui->actionWriteoutToFile->setEnabled(open);
    ui->actionWriteout->setEnabled(open);
    ui->PswdsTable->setEnabled(open);
}

std::string MainWindow::SiteAtRow(unsigned int row)
{
    return ui->PswdsTable->item(row, 0)->text().toStdString();
}

std::string MainWindow::UserAtRow(unsigned int row)
{
    return ui->PswdsTable->item(row, 1)->text().toStdString();
}

SecureString MainWindow::PswdAtRow(unsigned int row)
{
    QLineEdit* pswdEdit = reinterpret_cast<QLineEdit*>(ui->PswdsTable->cellWidget(row, 2));
    return std::move(SecureString(pswdEdit->text().toLocal8Bit().data(), pswdEdit->text().toLocal8Bit().size()));
}

// SLOTS
// ======

// File
void MainWindow::OpenPswdFile(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Any File (*)"));
    if(fileName.length() == 0)
        return;

    // Clear any old settings
    SetEnableModify(false);
    openFileName.clear();
    masterPswd.Clear();
    for(int i = ui->PswdsTable->rowCount()-1; i >= 0; i--)
    {
        ui->PswdsTable->removeRow(i);
    }

    GetPswd getPswd(&masterPswd, this);
    if(getPswd.exec() == QDialog::Accepted)
    {
        try
        {
            pswdmgr* pswdMgrPtr = new pswdmgr(fileName.toStdString(), masterPswd.GetStr());
            pswds.reset(pswdMgrPtr);
        }
        catch(std::exception& err)
        {
            masterPswd.Clear();
            pswds.reset(nullptr);
            QMessageBox msg(QMessageBox::Critical, "Error", tr(err.what()), QMessageBox::Ok, this);
            msg.exec();
            return;
        }

        RedrawTable();

        SetEnableModify(true);
        openFileName = fileName;
        ui->statusBar->showMessage(tr("Successfully opened password file ") + openFileName, 8000);
    }
    else
    {
        masterPswd.Clear();
    }
}

void MainWindow::WriteoutPswds(void)
{
    if(openFileName.length())
    {
        try
        {
            pswds->WriteOut(openFileName.toStdString());
            ui->statusBar->showMessage(tr("Successfully wrote to password file ") + openFileName, 8000);
        }
        catch(const std::exception& err)
        {
            QMessageBox msg(QMessageBox::Critical, "Error", tr(err.what()), QMessageBox::Ok, this);
            msg.exec();
        }
    }
}

void MainWindow::WriteoutToFile()
{
    // Check we have an open file
    if(openFileName.length())
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Any File (*)"));
        if(fileName.length() == 0)
            return;

        try
        {
            pswds->WriteOut(fileName.toStdString());
            ui->statusBar->showMessage(tr("Successfully wrote to password file ") + fileName, 8000);
        }
        catch(const std::exception& err)
        {
            QMessageBox msg(QMessageBox::Critical, "Error", tr(err.what()), QMessageBox::Ok, this);
            msg.exec();
        }
    }
}

void MainWindow::CreateNewPswdFile(void)
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Password File"), QDir::homePath(), tr("Any File (*)"));
    if(fileName.length() == 0)
        return;

    QFile file(fileName);
    file.open(QFile::ReadWrite);
    if(!file.isOpen())
    {
        ui->statusBar->showMessage(tr("Cannot create file ") + fileName, 8000);
        return;
    }
    file.close();

    // Clear any old settings
    SetEnableModify(false);
    openFileName.clear();
    masterPswd.Clear();
    for(int i = ui->PswdsTable->rowCount()-1; i >= 0; i--)
    {
        ui->PswdsTable->removeRow(i);
    }

    GetPswd getPswd(&masterPswd, this);
    if(getPswd.exec() == QDialog::Accepted)
    {
        try
        {
            pswdmgr* pswdMgrPtr = new pswdmgr(fileName.toStdString(), masterPswd.GetStr());
            pswds.reset(pswdMgrPtr);
        }
        catch(std::exception& err)
        {
            masterPswd.Clear();
            pswds.reset(nullptr);
            QMessageBox msg(QMessageBox::Critical, "Error", tr(err.what()), QMessageBox::Ok, this);
            msg.exec();
            return;
        }

        RedrawTable();

        SetEnableModify(true);
        openFileName = fileName;
        ui->statusBar->showMessage(tr("Successfully created password file ") + openFileName, 8000);
    }
    else
    {
        masterPswd.Clear();
    }
}

// Manage
void MainWindow::AddSitePswd(void)
{
    if(openFileName.length())
    {
        std::string site, username;
        SecureString pswd;
        GetSiteUserPass sup(site, username, pswd, this);
        if(sup.exec() == QDialog::Accepted)
        {
            pswds->AddSite(site, username, pswd.GetStr());
            SetEnableModify(false);
            RedrawTable();
            SetEnableModify(true);
            ui->statusBar->showMessage(tr("Added site ") + tr(site.c_str()), 4000);
        }
    }
}

void MainWindow::RemoveSitePswd(void)
{
    if(openFileName.length())
    {
        std::string site;
        RmSiteDialog siteDialog(site, this);
        if(siteDialog.exec() == QDialog::Accepted)
        {
            pswds->RemoveSite(site);
            SetEnableModify(false);
            RedrawTable();
            SetEnableModify(true);
            ui->statusBar->showMessage(tr("Removed user/password mapping for ") + tr(site.c_str()), 4000);
        }
    }
}

void MainWindow::ChangePswd(void)
{
    if(openFileName.length())
    {
        SecureString newPswd;
        GetPswd getPswd(&newPswd, this);
        if(getPswd.exec() == QDialog::Accepted)
        {
            try
            {
                pswds->CreateKey(newPswd.GetStr());
                masterPswd.PullFrom(newPswd);

                ui->statusBar->showMessage("Master password succesfully changed, no changes are written to disk..", 8000);
            }
            catch(const std::exception& err)
            {
                QMessageBox msg(QMessageBox::Critical, "Error", err.what(), QMessageBox::Ok, this);
                msg.exec();
            }
        }
    }
}

// Help
void MainWindow::OpenAbout()
{
    QMessageBox msg(QMessageBox::Information, "About", "This is a Qt wrapper of pswdmgr, a very lightweight password manager utility to make use of the experimental library CryptoLibrary."
        " All website credentials are written to disk in an AES-256 encrypted file with a random IV and a 256-bit key."
        "The key is generated by hashing your password with scrypt (with a high difficulty) and a random salt (to ensure same password never maps to the same key twice).", QMessageBox::Ok, this);

    msg.exec();
}

void MainWindow::OpenHelp()
{
    QMessageBox msg(QMessageBox::Information, "Help", "Its simple!\nStart by creating a new password file by clicking in the menu bar 'File->Create New Pswd File...' then select an appropriate location and name for the file. At the prompt, enter a memorable but sufficiently complex password as the master password to the file to finalize its creation.\n"
"From here you can create/remove credentials using the 'Manage' menu field, as well as edit existing entries by double clicking them. Changes such as edits and changing the current password are not written to disc unill 'File->Writeout' is pressed, writing to the last opened, created, or saved-to location. Additionally, 'File->Write To File...' can be used to specify a different/new password file to write to.", QMessageBox::Ok, this);
    msg.exec();
}

// Cells
void MainWindow::CellChanged(int row, int col)
{
    if(ui->PswdsTable->isEnabled())
    {
        switch (col) {
        case 0:
        {
            // Website changed
            std::shared_ptr<pswdmgr::UserPass> uPass = pswds->At(curSite.toStdString());
            pswds->RemoveSite(curSite.toStdString());

            std::string site = SiteAtRow(row);
            pswds->AddSite(site, uPass);
            break;
        }
        case 1:
        {
            // Username ..
            std::string site = SiteAtRow(row);
            pswds->At(site)->username = UserAtRow(row);
            break;
        }
        default:
            ui->statusBar->showMessage("Wat!! How do got heer!", 5000);
            return;
        }

        //ui->statusBar->showMessage(tr("Changed cell at row ") + QString::number(row) + tr(" and col ") + QString::number(col), 4000);
    }
}

void MainWindow::CellEntered(int row, int col)
{
    if(col == 0)
    {
        curSite = QString(SiteAtRow(row).c_str());
    }
    else
    {
        curSite.clear();
    }

    //ui->statusBar->showMessage(tr("Entered cell at row ") + QString::number(row) + tr(" and col ") + QString::number(col), 4000);
}

void MainWindow::PswdChanged()
{
    unsigned int row = sender()->property("curRow").toUInt();
    SecureString newPass = PswdAtRow(row);
    std::string site = SiteAtRow(row);
    pswds->At(site)->password.PullFrom(newPass);
}

void MainWindow::ShowPswdPress()
{
    unsigned int row = sender()->property("curRow").toUInt();
    reinterpret_cast<QLineEdit*>(ui->PswdsTable->cellWidget(row, 2))->setEchoMode(QLineEdit::Normal);
}

void MainWindow::ShowPswdRelease()
{
    unsigned int row = sender()->property("curRow").toUInt();
    reinterpret_cast<QLineEdit*>(ui->PswdsTable->cellWidget(row, 2))->setEchoMode(QLineEdit::Password);
}
