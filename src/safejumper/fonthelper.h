#ifndef FONTHELPER_H
#define FONTHELPER_H

#include <memory>

#include <QFont>
#include <QWidget>

//	FontHelper::SetFont(this);
class FontHelper
{
public:
    static QFont & font();
    static QFont & bold();
    static QFont pt(size_t pt, bool bold = false);
    static void SetFont(QWidget * w);
private:
    static void Init();
    static std::auto_ptr<QFont> _f;
    static std::auto_ptr<QFont> _bold;
};

#endif // FONTHELPER_H
