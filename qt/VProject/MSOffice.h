#ifndef MSOFFICE_H
#define MSOFFICE_H

#include <QString>

class MSOfficeHelper
{
protected:
    int mIsWordInstalled, mIsExcelInstalled;

    MSOfficeHelper();

    int IsInstalledInternal(const QString &lSubKeyname);
public:
    static MSOfficeHelper * GetInstance() {
        static MSOfficeHelper * lMSOfficeHelper = NULL;
        if (!lMSOfficeHelper) {
            lMSOfficeHelper = new MSOfficeHelper();
            //qAddPostRoutine(ProjectList::clean);
        }
        return lMSOfficeHelper;
    }

    bool IsWordInstalled();
    bool IsExcelInstalled();
};

#define gMSOffice MSOfficeHelper::GetInstance()

#endif // MSOFFICE_H
