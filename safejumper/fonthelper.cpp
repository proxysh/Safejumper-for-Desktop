#include "fonthelper.h"

#include <QFontDatabase>
#include <QDebug>

std::auto_ptr<QFont> FontHelper::_f;
std::auto_ptr<QFont> FontHelper::_bold;

QFont FontHelper::pt(size_t pt, bool bold) // = false)
{
	Init();
	QFont f(bold ? *_bold : *_f);
	qreal qpt;
	static qreal base = 7.5;
	static qreal koef = 0.5;
	if (pt < base)
		qpt = pt;
	else
		qpt = base + ((qreal)pt - base) * koef;
	f.setPointSizeF(qpt);
	return f;
}

QFont & FontHelper::font()
{
	Init();
	return *_f.get();
}

QFont & FontHelper::bold()
{
	Init();
	return *_bold.get();
}

void FontHelper::Init()
{
	// _f and _bold must be initialized here
	if (!_f.get())
	{
#ifdef Q_OS_MAC
		;			// TODO: -2 test - not using custom font on Mac
#else
		//int id =
		QFontDatabase::addApplicationFont(":/fonts/opensans");
		//int id2 =
		QFontDatabase::addApplicationFont(":/fonts/opensans-bold");
		static const char * n = "Open Sans";
		static const char * nb = "Open Sans Bold";
//qDebug() << QFontDatabase::applicationFontFamilies(id);
//qDebug() << QFontDatabase::applicationFontFamilies(id2);

		_f.reset(new QFont(n));
		_bold.reset(new QFont(nb));
		_bold->setBold(true);
#endif
	}
}

void FontHelper::SetFont(QWidget * w)
{
#ifndef Q_OS_MAC
	QObjectList widgetList = w->children();
	for (int k = 0, sz = widgetList.size(); k < sz; ++k)
	{
		QWidget * tw = dynamic_cast<QWidget*>(widgetList[k]);
		if (tw)
		{
			int pt = tw->fontInfo().pointSize();
			bool bold = tw->fontInfo().bold();
			tw->setFont(FontHelper::pt(pt, bold));
			SetFont(tw);
		}
	}
#endif		// Q_OS_MAC
}
