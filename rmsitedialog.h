#ifndef RMSITEDIALOG_H
#define RMSITEDIALOG_H

#include <QDialog>

namespace Ui {
class RmSiteDialog;
}

class RmSiteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RmSiteDialog(std::string* siteOut, QWidget *parent = 0);
    ~RmSiteDialog();

public slots:
    void Accepted();

private:
    Ui::RmSiteDialog *ui;
    std::string* site;
};

#endif // RMSITEDIALOG_H
