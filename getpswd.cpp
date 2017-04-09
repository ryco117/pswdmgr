#include "getpswd.h"
#include "ui_getpswd.h"

#include <string.h>

#include <iostream>
using namespace std;


GetPswd::GetPswd(SecureString* pswd, QWidget *parent) :
    QDialog(parent), pswdStr(pswd), ui(new Ui::GetPswd)
{
    ui->setupUi(this);
    ui->PswdEdit->setFocus();
    ui->buttonBox->setEnabled(false);
}

GetPswd::~GetPswd()
{
    delete ui;
}


// SLOTS
// ======

void GetPswd::PswdChanged(QString pswd)
{
    if(pswd.length())
    {
        ui->buttonBox->setEnabled(true);
    }
    else
    {
        pswdStr->Clear();
        ui->buttonBox->setEnabled(false);
    }
}

void GetPswd::PswdAccepted()
{
    // Replace current password buffer
    SecureString tempSecureStr(ui->PswdEdit->text().toLocal8Bit().data(), ui->PswdEdit->text().toLocal8Bit().length());
    pswdStr->PullFrom(tempSecureStr);

    emit accept();
}
