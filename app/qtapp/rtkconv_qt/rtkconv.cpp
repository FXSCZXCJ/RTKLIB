#include "convmain.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;

    bool ok = translator.load(QLocale(), "rtklib", "_", ":/i18n/");
    // RTKLIB_LANG=zh_CN / en / de overrides the system UI language
    const QByteArray lang = qgetenv("RTKLIB_LANG");
    if (!lang.isEmpty()) {
        QString code = QString::fromLatin1(lang).replace('-', '_');
        ok = translator.load("rtklib_" + code, ":/i18n/");
    }
    if (!ok)
        qDebug("Failed to open translation file.");

    a.installTranslator(&translator);


    MainWindow w(NULL);

    w.show();

    return a.exec();
}
