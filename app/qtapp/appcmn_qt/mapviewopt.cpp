//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QFile>
#include <QDir>
#include <QAction>
#include <QLineEdit>
#include <QComboBox>

#include "viewer.h"
#include "mapviewopt.h"

#include "ui_mapviewopt.h"


//---------------------------------------------------------------------------
MapViewOptDialog::MapViewOptDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::MapViewOpt)
{
    ui->setupUi(this);

    QAction *acNotes = new QAction("?", this);
    ui->lEApiKey->addAction(acNotes, QLineEdit::TrailingPosition);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MapViewOptDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MapViewOptDialog::reject);
    connect(acNotes, &QAction::triggered, this, &MapViewOptDialog::showNotes);
}
//---------------------------------------------------------------------------
void MapViewOptDialog::showNotes()
{
    QString filename, dir;
    TextViewer *viewer;
    
    dir = qApp->applicationDirPath(); // exe directory
    filename = QDir(dir).absoluteFilePath("gmview_notes.txt");
    if (!QFile::exists(filename))
        filename = QDir(dir).absoluteFilePath("../appcmn_qt/gmview_notes.txt");

    viewer = new TextViewer(this);
    viewer->setWindowTitle(filename);
    viewer->setOption(0);
    viewer->exec();
    viewer->read(filename);
}
//---------------------------------------------------------------------------
void MapViewOptDialog::setApiKey(const QString & apiKey)
{
    ui->lEApiKey->setText(apiKey);
}
//---------------------------------------------------------------------------
QString MapViewOptDialog::getApiKey()
{
    return ui->lEApiKey->text();
}
//---------------------------------------------------------------------------
void MapViewOptDialog::setMapDatum(int i, const QString & datum)
{
    QComboBox *cmb = findChild<QComboBox *>(QStringLiteral("cmbDatum%1").arg(i + 1));
    if (!cmb)
        return;

    cmb->setCurrentIndex((datum == "bd09") ? 2 : ((datum == "gcj02") ? 1 : 0));
}
//---------------------------------------------------------------------------
QString MapViewOptDialog::getMapDatum(int i)
{
    QComboBox *cmb = findChild<QComboBox *>(QStringLiteral("cmbDatum%1").arg(i + 1));
    if (!cmb)
        return "wgs84";

    const int idx = cmb->currentIndex();
    return (idx == 2) ? "bd09" : ((idx == 1) ? "gcj02" : "wgs84");
}
//---------------------------------------------------------------------------
void MapViewOptDialog::setAutoMaxZoom(bool on)
{
    ui->chkAutoMaxZoom->setChecked(on);
}
//---------------------------------------------------------------------------
bool MapViewOptDialog::getAutoMaxZoom()
{
    return ui->chkAutoMaxZoom->isChecked();
}
//---------------------------------------------------------------------------
void MapViewOptDialog::setMapStrings(int i, const QString &title, const QString &url, const QString &attr)
{
    if ((i < 0) || (i > 11))
        return;

    QLineEdit *titleEdit = findChild<QLineEdit *>(QStringLiteral("lEMapTitle%1").arg(i + 1));
    QLineEdit *urlEdit   = findChild<QLineEdit *>(QStringLiteral("lEMapTile%1").arg(i + 1));
    if (!titleEdit || !urlEdit)
        return;

    titleEdit->setText(title);
    urlEdit->setText(url);

    attrs[i] = attr;
}
//---------------------------------------------------------------------------
void MapViewOptDialog::getMapStrings(int i, QString &title, QString &url, QString &attr)
{
    if ((i < 0) || (i > 11))
        return;

    QLineEdit *titleEdit = findChild<QLineEdit *>(QStringLiteral("lEMapTitle%1").arg(i + 1));
    QLineEdit *urlEdit   = findChild<QLineEdit *>(QStringLiteral("lEMapTile%1").arg(i + 1));
    if (!titleEdit || !urlEdit)
        return;

    title = titleEdit->text();
    url = urlEdit->text();
    attr = attrs[i];
}
//---------------------------------------------------------------------------
