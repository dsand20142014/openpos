#include "mx25keyboardplugin.h"
#include "mx25keyboardhandler.h"

Mx25KeyboardPlugin::Mx25KeyboardPlugin()
{
}

QWSKeyboardHandler *Mx25KeyboardPlugin::create(const QString &driver, const QString &device)
{
    Q_UNUSED(device);
    if (driver.compare(QLatin1String("Mx25_4x5"), Qt::CaseInsensitive))
        return 0;
    return new Mx25KeyboardHandler(device);
}

QStringList Mx25KeyboardPlugin::keys() const
{
    return (QStringList() << QLatin1String("Mx25_4x5"));
}

Q_EXPORT_PLUGIN2(Mx25_4x5, Mx25KeyboardPlugin)
