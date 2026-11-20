# RTKLIB-EX 2.5.1 · Windows Qt GUI 中文版构建说明

本目录是从 `https://github.com/rtklibexplorer/RTKLIB` 克隆的 **RTKLIB-EX**（main 分支，
`v2.5.1-103-g755e6379`）。目标是把 `app\qtapp` 的 Qt 界面编译出来，并启用仓库里已有的
简体中文翻译。

## 产物位置

```
RTKLIB-EX\bin\qtapp\
├─ rtknavi_qt.exe        实时 RTK（中文已验证）
├─ rtkpost_qt.exe        事后处理（中文已验证）
├─ rtkplot_qt.exe        绘图
├─ rtkconv_qt.exe        格式转换
├─ strsvr_qt.exe         数据流服务
├─ rtkget_qt.exe         FTP/HTTP 下载
├─ rtklaunch_qt.exe      启动器
├─ srctblbrows_qt.exe    源表浏览
├─ run_zh.bat            中文启动脚本
├─ Qt6*.dll / platforms\ / styles\ / imageformats\ / tls\ ...
├─ rtklib.dll            RTKLIB 核心库（必须与 exe 同目录）
└─ vcruntime140*.dll / vcruntime140_1.dll / vccorlib140.dll ...
```

预编译的 **VCL 版**（英文，非 Qt）在 `D:\Project\Work_Project\RTK-Tools\_prebuilt_RTKLIB_EX_2.5.1\`，
仅作对照，界面无法汉化。

## 依赖

| 组件 | 位置 / 版本 |
| --- | --- |
| Qt | `D:\Dev\SDK\Qt\6.5.3\msvc2019_64`（qtbase + qtserialport + qtwebengine + qtpositioning + qtwebchannel，含 lrelease） |
| MSVC | VS 2022 BuildTools 14.44（`vcvars64.bat`） |
| Ninja | `D:\Dev\mingw64\bin\ninja.exe` |
| CMake | 3.16+ |

注意：**不要**把 mingw 的 `bin` 加进 PATH，否则 CMake 会选中 gcc 而不是 cl。

## 构建 / 部署

```powershell
# 1. 编译（vcvars64 + Ninja 单配置 Release）
.\build_qt.bat

# 2. 部署：复制 exe、rtklib.dll、VC 运行库，并运行 windeployqt
.\deploy_qt.bat
```

`build_qt.bat` 首次会执行：

```
cmake -S . -B build-qt -G Ninja ^
      -DCMAKE_MAKE_PROGRAM=D:/Dev/mingw64/bin/ninja.exe ^
      -DCMAKE_PREFIX_PATH=D:/Dev/SDK/Qt/6.5.3/msvc2019_64 ^
      -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
```

## 中文界面是怎么生效的

1. 仓库自带 `app\qtapp\<app>\rtklib_zh_CN.ts`（rtknavi 1459 条 / rtkpost 730 / rtkplot 920 /
   rtkconv 512 / strsvr 386，均已译完）；
2. 每个子工程用 `qt_add_lrelease()` 把 `.ts` 编成 `.qm`，再用 `qt_add_resources()`
   以 `PREFIX "/i18n"` + `QT_RESOURCE_ALIAS "rtklib_zh_CN.qm"` 嵌入 exe；
3. 程序启动时 `QTranslator::load(QLocale(), "rtklib", "_", ":/i18n/")` 按系统语言加载；
4. 另加了 `RTKLIB_LANG` 覆盖（对 8 个程序都生效）：
   - `RTKLIB_LANG=zh_CN` 中文
   - `RTKLIB_LANG=en` 英文
   - 不设置则跟随系统 UI 语言

命令行直接指定：

```powershell
$env:RTKLIB_LANG='zh_CN'; .\rtkpost_qt.exe
```

或双击 `run_zh.bat`（默认 rtknavi，也可 `run_zh.bat rtkpost_qt`）。

> 如果系统 UI 语言是英文（例如某些账号），不设 `RTKLIB_LANG` 会显示英文 —— 这是 Qt
> `QLocale()` 的行为，不是翻译缺失。

## 构建时踩到并已修掉的坑

| 现象 | 原因 | 处理 |
| --- | --- | --- |
| `No CMAKE_CXX_COMPILER` | VS 生成器未找到工具链 / mingw gcc 抢占 | 用 `vcvars64.bat` + Ninja，PATH 里不放 mingw |
| `Could NOT find BLAS` | `test/utest` 依赖 BLAS | 根 `CMakeLists.txt` 的 `add_subdirectory(test)` 改为 `if(BUILD_TESTING)` |
| `Unexpected arguments` at `qt_standard_project_setup` | Qt 6.5 无 `I18N_TRANSLATED_LANGUAGES` | 6.7 以下调用无参版本，翻译由 `qt_add_lrelease` 显式处理 |
| 启动即 `0xC0000135` | 缺 `rtklib.dll`、`vcruntime140*.dll` | `deploy_qt.bat` 复制 `lib\rtklib.dll` 与 VC143 CRT |
| 界面仍英文 | `.qm` 在工作目录，资源别名变成 `:/i18n/D:/Project/...` 绝对路径 | 显式 `QT_RESOURCE_ALIAS` |
| 有 WebEngine 但地图不出现 | 源码用 `${Qt6WebEngineWidgets_FOUND}` 判断，Qt 6.5 下该变量为空 | 改为 `if(TARGET Qt6::WebEngineWidgets)` |
| WebEngine 装了仍找不到 | 缺 `qtpositioning` / `qtwebchannel` 依赖模块 | 一并安装后重新 configure |
| exe 没有图标（白板） | `.pro` 里的 `RC_FILE` 在 CMake 里没对应项，`add_executable()` 未列 `.rc` | 把 `<app>.rc` 加入源文件列表（见 `tools\i18n_qt\patch_rc.py`） |
| 改了头文件后程序启动即崩（`0xC0000374` 堆损坏） | 用 `Copy-Item` 恢复源码时保留了旧 mtime，Ninja 没重编部分 `.obj`，类布局不一致 | 删除 `build-qt` 全量重编 |

## 待办 / 可选

- 地图瓦片：支持 **12 个源**，默认预置 10 个：
  `OpenStreetMap` / `高德路网` / `高德卫星` / `腾讯路网` / `腾讯卫星` /
  `Google 路网` / `Google 卫星` / `Esri 影像` / `Carto 淡色` / `天地图影像`。
  每个源带自己的 **Datum**：`WGS-84` / `GCJ-02` / `BD-09`；切换图层时自动重算
  所有轨迹点和视野中心（逐点转换，不是平移）。
  - **Google 卫星（`mt0.google.com/vt?lyrs=s`）实测是未偏移影像 → 默认 `wgs84`**；
    Google 路网在中国区是偏移版，默认 `gcj02`；两者可分别调，以实测为准；
    走国际版服务或境外时，两者都应改成 `wgs84`；
  - 腾讯瓦片需要 TMS（y 翻转），程序按 URL 里的 `gtimg.com` 自动开启；
  - `天地图影像` 需要把 URL 里的 `YOUR_TIANDITU_KEY` 换成你自己的 tk；
  - `Esri` / `Carto` / `OSM` 是真 WGS-84，不要选 GCJ-02；
  - 配置存 `rtkplot_qt.ini`：`mapstring0=标题,URL,版权,坐标系`（第 4 段）。
- 地图缩放：`选项...` 里的 `打开时使用最大缩放级别`（`mapview/autozoom=1`）会让每次
  打开地图都直接到当前图层的**原生**最大级别（谷歌/ESRI/Carto/OSM 19，高德/腾讯/天地图 18），
  并用该 zoom 代替 fitBounds。手动缩放可到 **22 级**（约 0.3 m/像素，比例尺可到 1 m 级），
  超过原生级别的部分由瓦片拉伸显示（会变糊）。
- 底图记忆：用右上角图层按钮切过的底图会被记住（`mapview/basemap=高德路网`），
  下次打开地图直接回到那一层，不再回到第一个；正常关闭程序时写入 ini。
- **百度地图**：BD-09 坐标转换已支持（`wgs84→gcj02→bd09`），但百度瓦片用的是
  私有墨卡托与自有瓦片编号，**不能当标准 TMS 直接填 URL**。要用百度底图需要
  额外实现一套百度投影层（或提供 AK 走 BMapGL），目前未默认启用。

- 地图页签已启用（Qt WebEngine），`rtkplot_qt` 菜单可见 `显示地图/航点(W)`、`地图视图`；
  地图底图来自在线 Leaflet/OSM，离线或内网时只显示空白底图，轨迹仍可显示；
- `data\` 下的 `igs14.atx`、`*.cmd`、`*.conf` 未复制到 `bin\qtapp`，用到 ATX/命令行模板时
  在程序里指向 `RTKLIB-EX\data\` 即可；
- 若要重新生成翻译源：`lupdate`（Qt\bin）扫描源码，配合 `lrelease` 编译。
