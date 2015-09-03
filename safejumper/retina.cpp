#include "retina.h"
#include "common.h"

#include <QApplication>


bool IsRetina()
{
	bool b = false;
#if defined(Q_OS_MAC)
	if (g_pTheApp->devicePixelRatio() >= 2)
		b = true;
#endif
	return b;
}
