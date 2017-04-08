#include "rmsitedialog.h"
#include "ui_rmsitedialog.h"

RmSiteDialog::RmSiteDialog(std::string& siteOut, QWidget *parent) :
    QDialog(parent), site(siteOut), ui(new Ui::RmSiteDialog)
{
    ui->setupUi(this);
}

RmSiteDialog::~RmSiteDialog()
{
    delete ui;
}

void RmSiteDialog::Accepted()
{
    site = ui->SiteEdit->text().toStdString();

    emit accept();
}
