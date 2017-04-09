#ifndef GETPSWD_H
#define GETPSWD_H

#include <crypto/SecureString.h>
#include <QDialog>

namespace Ui {
class GetPswd;
}

class GetPswd : public QDialog
{
    Q_OBJECT

public:
    explicit GetPswd(SecureString* pswdOut, QWidget *parent = 0);
    ~GetPswd();

public slots:
    void PswdChanged(QString pswd);
    void PswdAccepted();

private:
    Ui::GetPswd *ui;
    SecureString* pswdStr;
};

#endif // GETPSWD_H
