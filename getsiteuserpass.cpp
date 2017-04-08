#include "getsiteuserpass.h"
#include "ui_getsiteuserpass.h"

GetSiteUserPass::GetSiteUserPass(std::string& siteOut, std::string& usernameOut, SecureString& pswdOut, QWidget *parent) :
    QDialog(parent), ui(new Ui::GetSiteUserPass), site(siteOut), username(usernameOut), pswd(pswdOut)
{
    ui->setupUi(this);
}

GetSiteUserPass::~GetSiteUserPass()
{
    delete ui;
}

void GetSiteUserPass::Accepted()
{
    site = ui->siteEdit->text().toStdString();
    username = ui->userEdit->text().toStdString();
    SecureString tempSecureStr(ui->pswdEdit->text().toLocal8Bit().data(), ui->pswdEdit->text().toLocal8Bit().length());
    pswd.PullFrom(tempSecureStr);

    // Zero
    memset(ui->pswdEdit->text().data(), 0, ui->pswdEdit->text().length() * sizeof(QChar));

    emit accept();
}
