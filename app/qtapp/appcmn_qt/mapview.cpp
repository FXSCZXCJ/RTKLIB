//---------------------------------------------------------------------------
// gmview.c: map view
//---------------------------------------------------------------------------
#ifdef QWEBENGINE
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#endif

#include <QShowEvent>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QLabel>

#include <cmath>

#include "mapviewopt.h"
#include "mapview.h"

#include "ui_mapview.h"

#define RTKLIB_GM_TEMP ":/html/rtklib_gm.htm"
#define RTKLIB_GM_FILE "rtklib_gm_a.htm"
#define RTKLIB_LL_TEMP ":/html/rtklib_ll.htm"
#define RTKLIB_LL_FILE "rtklib_ll_a.htm"
#define URL_GM_API     "http://maps.google.com/maps/api/js"
#define MAP_OPACITY    0.8
#define INIT_ZOOM      8  // initial zoom level

/* guess the coordinate datum of a tile source from its URL ---------------
* Amap / Tencent serve GCJ-02 tiles; everything else is WGS-84.
*-------------------------------------------------------------------------*/
static QString inferDatum(const QString &url)
{
    if (url.contains("autonavi", Qt::CaseInsensitive) ||
        url.contains("amap", Qt::CaseInsensitive) ||
        url.contains("qq.com", Qt::CaseInsensitive))
        return QStringLiteral("gcj02");
    return QStringLiteral("wgs84");
}

//---------------------------------------------------------------------------
MapView::MapView(QWidget *parent)
    : QDialog(parent), ui(new Ui::MapView)
{
    loaded = false;
    ui->setupUi(this);

    selectedMap = 0;
    center_latitude = center_longitude = 0.0;
    highlightedMark = -1;
    autoMaxZoom = false;
    /* standard top-level window: minimise/maximise buttons, Aero Snap, resize grip */
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setSizeGripEnabled(true);

    /* never let the map window collapse to a tiny size while dragging/resizing */
    setMinimumSize(480, 320);
    for (int i = 0; i < MAXMAPSRC; i++) mapDatums[i] = "";

    for (int i = 0; i < NUM_MARK; i++) {
        markState[i] = 0;
        markPosition[i][0] = markPosition[i][1] = NAN;
    }

    mapViewOptDialog = new MapViewOptDialog(this);
    timerLL.setInterval(100);
    timerGM.setInterval(100);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MapView::reject);
    connect(ui->btnOptions, &QPushButton::clicked, this, &MapView::showOptionsDialog);
    connect(ui->btnShrink, &QPushButton::clicked, this, &MapView::zoomOut);
    connect(ui->btnExpand, &QPushButton::clicked, this, &MapView::zoomIn);
    connect(ui->btnCenter, &QPushButton::clicked, this, &MapView::center);
    connect(ui->rBMapSelect1, &QRadioButton::clicked, this, &MapView::selectMapLL);
    connect(ui->rBMapSelect2, &QRadioButton::clicked, this, &MapView::selectMapGM);
    connect(&timerLL, &QTimer::timeout, this, &MapView::timerLLTimer);
    connect(&timerGM, &QTimer::timeout, this, &MapView::timerGMTimer);

    mapStrings[0][0] = "OpenStreetMap";
    mapStrings[0][1] = "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
    mapStrings[0][2] = "https://osm.org/copyright";
    mapDatums[0] = "wgs84";

    mapStrings[1][0] = "高德路网";
    mapStrings[1][1] = "https://webrd01.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x={x}&y={y}&z={z}";
    mapStrings[1][2] = "https://www.amap.com/";
    mapDatums[1] = "gcj02";

    mapStrings[2][0] = "高德卫星";
    mapStrings[2][1] = "https://webst01.is.autonavi.com/appmaptile?style=6&x={x}&y={y}&z={z}";
    mapStrings[2][2] = "https://www.amap.com/";
    mapDatums[2] = "gcj02";

    mapStrings[3][0] = "腾讯路网";
    mapStrings[3][1] = "https://rt0.map.gtimg.com/tile?z={z}&x={x}&y={y}&type=vector&styleid=3";
    mapStrings[3][2] = "https://map.qq.com/";
    mapDatums[3] = "gcj02";

    mapStrings[4][0] = "腾讯卫星";
    mapStrings[4][1] = "https://p0.map.gtimg.com/sateTiles/{z}/{x}/{y}.jpg";
    mapStrings[4][2] = "https://map.qq.com/";
    mapDatums[4] = "gcj02";

    mapStrings[5][0] = "Google 路网";
    mapStrings[5][1] = "https://mt0.google.com/vt/lyrs=m&x={x}&y={y}&z={z}";
    mapStrings[5][2] = "https://maps.google.com/";
    /* Google China tiles (roads + imagery) are offset to GCJ-02; use WGS-84
     * instead when you access the international service from abroad. */
    mapDatums[5] = "gcj02";

    mapStrings[6][0] = "Google 卫星";
    mapStrings[6][1] = "https://mt0.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
    mapStrings[6][2] = "https://maps.google.com/";
    /* 实测：mt0.google.com 的卫星影像未偏移（WGS-84），用 GCJ-02 会向东偏 ~500 m */
    mapDatums[6] = "wgs84";

    mapStrings[7][0] = "Esri 影像";
    mapStrings[7][1] = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}";
    mapStrings[7][2] = "https://www.esri.com/";
    mapDatums[7] = "wgs84";

    mapStrings[8][0] = "Carto 淡色";
    mapStrings[8][1] = "https://basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png";
    mapStrings[8][2] = "https://carto.com/attributions";
    mapDatums[8] = "wgs84";

    mapStrings[9][0] = "天地图影像";
    mapStrings[9][1] = "https://t0.tianditu.gov.cn/img_w/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=img&STYLE=default&TILEMATRIXSET=w&FORMAT=tiles&TILEMATRIX={z}&TILEROW={y}&TILECOL={x}&tk=YOUR_TIANDITU_KEY";
    mapStrings[9][2] = "https://www.tianditu.gov.cn/";
    mapDatums[9] = "wgs84";

    QHBoxLayout *layout = new QHBoxLayout();
    ui->wgMap->setContentsMargins(0, 0, 0, 0);
    ui->wgMap->setLayout(layout);

#ifdef QWEBENGINE
    webBrowser = new QWebEngineView(ui->wgMap);
    connect(webBrowser, &QWebEngineView::loadFinished, this, &MapView::pageLoaded);
    layout->addWidget(webBrowser);
    pageState = new MapViewPageState(this);
    connect(pageState, &MapViewPageState::basemapChanged, this,
            [this](const QString &name) { selectedBasemap = name; });
    webChannel = new QWebChannel(this);
    webChannel->registerObject(QStringLiteral("state"), pageState);

#else
    QLabel *label = new QLabel();
    label->setText(tr("QWebEngine is not available to show a map."));
    layout->addWidget(label);
#endif

    ui->rBMapSelect1->setChecked(!selectedMap);
    ui->rBMapSelect2->setChecked(selectedMap);
    selectMap(selectedMap);
}
//---------------------------------------------------------------------------
void MapView::setApiKey(const QString & key)
{
    apiKey = key;
}
//---------------------------------------------------------------------------
const QString &MapView::getApiKey()
{
    return apiKey;
}
//---------------------------------------------------------------------------
void MapView::selectMapLL()
{
    selectMap(0);
}
//---------------------------------------------------------------------------
void MapView::selectMapGM()
{
    selectMap(1);
}
//---------------------------------------------------------------------------
void MapView::showOptionsDialog()
{
    mapViewOptDialog->move(x() + width() / 2 - mapViewOptDialog->width() / 2,
                           y() + height() / 2 - mapViewOptDialog->height() / 2);

    mapViewOptDialog->setApiKey(apiKey);
    mapViewOptDialog->setAutoMaxZoom(autoMaxZoom);

    for (int i = 0; i < MAXMAPSRC; i++) {
        mapViewOptDialog->setMapStrings(i, mapStrings[i][0], mapStrings[i][1], mapStrings[i][2]);
        mapViewOptDialog->setMapDatum(i, mapDatums[i]);
    }

    if (mapViewOptDialog->exec() != QDialog::Accepted)
        return;

    apiKey = mapViewOptDialog->getApiKey();
    autoMaxZoom = mapViewOptDialog->getAutoMaxZoom();

    for (int i = 0; i < MAXMAPSRC; i++) {
        mapViewOptDialog->getMapStrings(i, mapStrings[i][0], mapStrings[i][1], mapStrings[i][2]);
        mapDatums[i] = mapViewOptDialog->getMapDatum(i);
    }

    showMap(selectedMap);
}
//---------------------------------------------------------------------------
void MapView::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    /* some window managers send a degenerate resize while the window is being
     * dragged; bounce back to the minimum size instead of collapsing */
    if (width() < 480 || height() < 320) {
        resize(qMax(width(), 480), qMax(height(), 320));
    }
}
//---------------------------------------------------------------------------
void MapView::pageLoaded(bool ok)
{
    if (!ok) return;

#ifdef QWEBENGINE
    QFile webchannel(":/html/qwebchannel.js");
    webchannel.open(QIODevice::ReadOnly);
    webBrowser->page()->runJavaScript(webchannel.readAll());
    webBrowser->page()->runJavaScript("new QWebChannel(qt.webChannelTransport,function(channel) {window.qstate = channel.objects.state; window.qstate.text=document.getElementById('state').value; window.qstate.setBasemap(document.getElementById('layername').value);});");
#endif

    loaded = true;

    /* the page may still be initialising when the first updateMap() ran;
     * redraw once after load so the rover marker shows up centered */
    QTimer::singleShot(300, this, &MapView::updateMap);
}
//---------------------------------------------------------------------------
void MapView::zoomOut()
{
    execFunction(selectedMap, "ZoomOut()");
}
//---------------------------------------------------------------------------
void MapView::zoomIn()
{
    execFunction(selectedMap, "ZoomIn()");
}
//---------------------------------------------------------------------------
void MapView::center()
{
    setCenter(center_latitude, center_longitude);
}
//---------------------------------------------------------------------------
void MapView::showMap(int map)
{
    if (map == 0) {
        showMapLL();
    } else if (map == 1) {
        showMapGM();
    }
}
//---------------------------------------------------------------------------
void MapView::showMapLL()
{
#ifdef QWEBENGINE
    QString pageSource;
    int i, j;

    QFile ifp(RTKLIB_LL_TEMP);

    if (!ifp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    {
    QTextStream out(&pageSource);
    while (!ifp.atEnd()) {
        QString buff = ifp.readLine();
        out << buff;
        if (buff.contains("<script src=\"qrc:/leaflet/leaflet.js\"></script>")) out << "<script type=\"text/javascript\" src=\"https://getfirebug.com/firebug-lite.js\"></script>\n";
        if (!buff.contains("// start map tiles")) continue;
        for (i = 0, j = 1; i < MAXMAPSRC; i++) {
            if (mapStrings[i][0] == "") continue;
            QString title = mapStrings[i][0];
            QString url = mapStrings[i][1];
            QString attr = mapStrings[i][2];

            out << QStringLiteral("var tile%1 = L.tileLayer('%2', {\n").arg(j).arg(url);
            out << QStringLiteral("  attribution: \"<a href='%1' target='_blank'>%2</a>\",\n").arg(attr, title);
            int maxNZ = (url.contains("google", Qt::CaseInsensitive) ||
                         url.contains("arcgisonline", Qt::CaseInsensitive) ||
                         url.contains("cartocdn", Qt::CaseInsensitive) ||
                         url.contains("openstreetmap", Qt::CaseInsensitive)) ? 19 : 18;
            out << QStringLiteral("  opacity: %1%2, maxNativeZoom: %3, maxZoom: 22});\n")
                   .arg(MAP_OPACITY, 0, 'f', 1)
                   .arg(url.contains("gtimg.com", Qt::CaseInsensitive) ? ", tms: true" : "")
                   .arg(maxNZ);
            out << QStringLiteral("tile%1.datum = \"%2\";\n").arg(j)
                   .arg(mapDatums[i].isEmpty() ? inferDatum(url) : mapDatums[i]);
            j++;
        }
        out << "var basemaps = {";
        for (i = 0, j = 1; i < MAXMAPSRC; i++) {
            if (mapStrings[i][0] == "") continue;
            QString title = mapStrings[i][0];
            out << QStringLiteral("%1\"%2\":tile%3").arg((j == 1) ? "" : ",", title).arg(j);
            j++;
        }
        out << "};\n";
    }
    }
    ifp.close();

    /* open at the maximum zoom level of the active layer */
    pageSource.replace(QStringLiteral("var autoMaxZoom = false;"),
                       QStringLiteral("var autoMaxZoom = %1;")
                           .arg(autoMaxZoom ? "true" : "false"));

    /* restore the basemap chosen last time */
    QString basemap = selectedBasemap;
    basemap.replace("\\", "\\\\").replace("'", "\\'");
    pageSource.replace(QStringLiteral("var selBasemap = '';"),
                       QStringLiteral("var selBasemap = '%1';").arg(basemap));

    webBrowser->setHtml(pageSource);
    webBrowser->page()->setWebChannel(webChannel);
    webBrowser->show();
    loaded = false;

    timerLL.start();
#endif
}
//---------------------------------------------------------------------------
void MapView::timerLLTimer()
{
    if (!setState(0)) return;

    setView(0, center_latitude, center_longitude, INIT_ZOOM);
    updateMap();

    timerLL.stop();
}
//---------------------------------------------------------------------------
void MapView::showMapGM()
{
#ifdef QWEBENGINE
    QString pageSource;

    QFile ifp(RTKLIB_GM_TEMP);

    if (!ifp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&pageSource);
    while (!ifp.atEnd()) {
        QString buff = ifp.readLine();

        if (!apiKey.isEmpty()) {
            buff = buff.replace(QStringLiteral("_insertKeyHere_"), apiKey);
        }

        out << buff;
    }
    ifp.close();

    webBrowser->setHtml(pageSource);
    webBrowser->page()->setWebChannel(webChannel);
    webBrowser->show();
    loaded = false;

    timerGM.start();
#endif
}
//---------------------------------------------------------------------------
void MapView::timerGMTimer()
{
    if (!setState(1)) return;

    setView(1, center_latitude, center_longitude, INIT_ZOOM);
    updateMap();

    timerGM.stop();
}

//---------------------------------------------------------------------------
void MapView::setView(int map, double lat, double lon, int zoom)
{
    execFunction(map, QStringLiteral("SetView(%1,%2,%3)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(zoom));
}
//---------------------------------------------------------------------------
void MapView::updateMap()
{
    /* full redraw: drop old markers so re-entrant calls do not stack them */
    execFunction(selectedMap, QStringLiteral("ClearMark()"));

    /* if the map center was never set, follow the first valid mark */
    if (center_latitude == 0.0 && center_longitude == 0.0) {
        for (int i = 0; i < NUM_MARK; i++) {
            if (!std::isnan(markPosition[i][0]) && !std::isnan(markPosition[i][1])) {
                center_latitude  = markPosition[i][0];
                center_longitude = markPosition[i][1];
                break;
            }
        }
    }

    setCenter(center_latitude, center_longitude);

    for (int i = 0; i < NUM_MARK; i++) {
        if (std::isnan(markPosition[i][0]) || std::isnan(markPosition[i][0])) continue;

        addMark(selectedMap, i, markPosition[i][0], markPosition[i][1], markTitle[i], markMessage[i]);

        if (markState[i])
            showMark(i);
        else
            hideMark(i);
    }
}
//---------------------------------------------------------------------------
void MapView::selectMap(int map)
{
    selectedMap = map;

    setView(selectedMap, center_latitude, center_longitude, INIT_ZOOM);

    showMap(selectedMap);
}

//---------------------------------------------------------------------------
void MapView::setCenter(double lat, double lon)
{
    QString func = QStringLiteral("SetCent(%1,%2)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9);

    execFunction(selectedMap, func);

    center_latitude = lat;
    center_longitude = lon;
}
//---------------------------------------------------------------------------
void MapView::setViewBounds(double min_lat, double min_lon, double max_lat, double max_lon)
{ //TODO: to be implemented for Google Maps
    execFunction(selectedMap, QStringLiteral("SetViewBounds(%1,%2,%3,%4)").arg(min_lat, 0, 'f', 9).arg(min_lon, 0, 'f', 9).
                 arg(max_lat, 0, 'f', 9).arg(max_lon, 0, 'f', 9));
};
//---------------------------------------------------------------------------
void MapView::addMark(int map, int index, double lat, double lon, const QString &title, const QString &msg)
{
    if (index >= NUM_MARK) return;

    QString func = QStringLiteral("AddMark(%1,%2,'%3','%4')").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(title, msg);
    execFunction(map, func);

    markTitle[index] = title;
    markMessage[index] = msg;
    markPosition[index][0] = lat;
    markPosition[index][1] = lon;

    func = QStringLiteral("ShowMark('%1')").arg(title);
    execFunction(map, func);

    markState[index] = 1;
}
//---------------------------------------------------------------------------
void MapView::setMark(int index, const QString &title, double lat, double lon)
{
    if (index >= NUM_MARK) return;

    markTitle[index] = title;
    QString func = QStringLiteral("PosMark(%1,%2,'%3')").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(title);

    execFunction(selectedMap, func);

    markPosition[index][0] = lat;
    markPosition[index][1] = lon;
}
//---------------------------------------------------------------------------
void MapView::showMark(int index)
{
    if (index >= NUM_MARK) return;

    QString func = QStringLiteral("ShowMark('%1')").arg(markTitle[index]);

    execFunction(selectedMap,func);

    markState[index] = 1;
}
//---------------------------------------------------------------------------
void MapView::hideMark(int index)
{
    if (index >= NUM_MARK) return;

    QString func = QStringLiteral("HideMark('%1')").arg(markTitle[index]);

    execFunction(selectedMap,func);

    markState[index] = 0;
}
//---------------------------------------------------------------------------
void MapView::highlightMark(int index)
{
    if (index >= NUM_MARK) return;

    QString func = QStringLiteral("HighlightMark('%1')").arg(markTitle[index]);

    execFunction(selectedMap, func);

    highlightedMark = index;
}
//---------------------------------------------------------------------------
void MapView::clearMark()
{
    QString func = QStringLiteral("ClearMark()");

    execFunction(selectedMap,func);

    highlightedMark = -1;
    for (int i = 0; i < NUM_MARK; i++) {
        markState[i] = 0;
        markPosition[i][0] = markPosition[i][1] = NAN;
    }
}
//---------------------------------------------------------------------------
int MapView::setState(int map)
{
    Q_UNUSED(map)
    QApplication::processEvents();

    if (!loaded) return 0;
    return pageState->getText().toInt();
}
//---------------------------------------------------------------------------
void MapView::execFunction(int map, const QString &func)
{
    Q_UNUSED(map)

    if (!loaded) return;

#ifdef QWEBENGINE
    QWebEnginePage *page = webBrowser->page();
    if (page == NULL) return;

    page->runJavaScript(func);
#else
    Q_UNUSED(func)
#endif
}
//---------------------------------------------------------------------------
bool MapView::isLoaded()
{
    return setState(1);
}
//---------------------------------------------------------------------------
void MapView::loadOptions(QSettings &settings)
{
    QStringList strs;
    for (int i = 0; i < MAXMAPSRC; i++)
    {
        strs = settings.value(QStringLiteral("set/mapstring%1").arg(i), "").toString().split(",");
        if (strs.length() >= 3 && !strs[0].trimmed().isEmpty())
        {
            for (int j = 0; j < 3; j++)
                mapStrings[i][j] = strs[j];
            if (strs.length() >= 4)
                mapDatums[i] = strs[3].trimmed();
        }
    }

    apiKey = settings.value("mapview/apikey" ,"").toString();
    autoMaxZoom = settings.value("mapview/autozoom", false).toBool();
    selectedBasemap = settings.value("mapview/basemap", "").toString();

    /* older configs have only 3 fields: leave the datum empty here so that
     * showMapLL() can infer it from the tile URL (Amap/Tencent -> gcj02) */
}
//---------------------------------------------------------------------------
void MapView::saveOptions(QSettings &settings)
{
    for (int i = 0; i < MAXMAPSRC; i++)
    {
        QString datum = mapDatums[i].isEmpty() ? inferDatum(mapStrings[i][1]) : mapDatums[i];
        QString str = mapStrings[i][0] + "," + mapStrings[i][1] + "," + mapStrings[i][2] + "," + datum;
        settings.setValue(QStringLiteral("set/mapstring%1").arg(i), str);
    }

    settings.setValue("mapview/apikey", apiKey);
    settings.setValue("mapview/autozoom", autoMaxZoom);
    settings.setValue("mapview/basemap", selectedBasemap);
}
//---------------------------------------------------------------------------
