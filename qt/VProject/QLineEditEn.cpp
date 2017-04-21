#include "QLineEditEn.h"

#define NOMINMAX
#include <windows.h>

QLineEditEn::QLineEditEn(QWidget *parent) : QLineEdit(parent)
{

}

void QLineEditEn::focusInEvent(QFocusEvent * event) {
    QLineEdit::focusInEvent(event);

    // caps off
    if (GetKeyState(VK_CAPITAL) & 1) {
        INPUT lInput[2];
        lInput[0].type = INPUT_KEYBOARD;
        lInput[0].ki.wVk = VK_CAPITAL;
        lInput[0].ki.dwFlags = 0;
        lInput[0].ki.time = 0;
        lInput[0].ki.dwExtraInfo = 0;
        memcpy(&lInput[1], &lInput[0], sizeof(INPUT));
        lInput[0].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, (LPINPUT) &lInput, sizeof(INPUT));
    }
    // set english
    wchar_t lOrigLayoutName[KL_NAMELENGTH];
    if (GetKeyboardLayoutName(lOrigLayoutName)) {
        if ((QString::fromWCharArray(lOrigLayoutName).toInt(NULL, 16) & 0xFFFF) != 0x0409) {
            LoadKeyboardLayout(L"00000409", KLF_ACTIVATE);
        }
    }

}
