#ifndef GETSITEUSERPASS_H
#define GETSITEUSERPASS_H

#include <QDialog>
#include <crypto/SecureString.h>

namespace Ui {
class GetSiteUserPass;
}

class GetSiteUserPass : public QDialog
{
    Q_OBJECT

public:
    explicit GetSiteUserPass(std::string& siteOut, std::string& usernameOut, SecureString& pswdOut, QWidget *parent = 0);
    ~GetSiteUserPass();

public slots:
    void Accepted(void);

private:
    Ui::GetSiteUserPass *ui;
    std::string& site;
    std::string& username;
    SecureString& pswd;
};

#endif // GETSITEUSERPASS_H
