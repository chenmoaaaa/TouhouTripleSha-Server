#include "mainwindow.h"
#include "startscene.h"
#include "roomscene.h"
#include "server.h"
#include "client.h"
#include "generaloverview.h"
#include "cardoverview.h"
#include "ui_mainwindow.h"
#include "scenario-overview.h"
#include "window.h"
#include "pixmapanimation.h"
#include "record-analysis.h"
#include "audio.h"

#include <qmath.h>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVariant>
#include <QMessageBox>
#include <QTime>
#include <QCheckBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QLabel>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QUrl>

#ifdef Q_OS_WIN
#include "Windows.h"
#endif

class FitView: public QGraphicsView {
public:
    FitView(QGraphicsScene *scene): QGraphicsView(scene) {
        setSceneRect(Config.Rect);
        setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    }

    virtual void resizeEvent(QResizeEvent *event) {
        QGraphicsView::resizeEvent(event);
        MainWindow *main_window = qobject_cast<MainWindow *>(parentWidget());
        if (scene()->inherits("RoomScene")) {
            RoomScene *room_scene = qobject_cast<RoomScene *>(scene());
            QRectF newSceneRect(0, 0, event->size().width(), event->size().height());
            room_scene->setSceneRect(newSceneRect);
            room_scene->adjustItems();
            setSceneRect(room_scene->sceneRect());
            if (newSceneRect != room_scene->sceneRect())
                fitInView(room_scene->sceneRect(), Qt::KeepAspectRatio);
            else
                this->resetTransform();
            main_window->setBackgroundBrush(false);
            return;
        } else if (scene()->inherits("StartScene")) {
            StartScene *start_scene = qobject_cast<StartScene *>(scene());
            QRectF newSceneRect(-event->size().width() / 2, -event->size().height() / 2,
                                event->size().width(), event->size().height());
            start_scene->setSceneRect(newSceneRect);
            setSceneRect(start_scene->sceneRect());
            if (newSceneRect != start_scene->sceneRect())
                fitInView(start_scene->sceneRect(), Qt::KeepAspectRatio);
        }
        if (main_window)
            main_window->setBackgroundBrush(true);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), server(NULL)
{
    ui->setupUi(this);
    scene = NULL;

    connection_dialog = new ConnectionDialog(this);
    connect(ui->actionStart_Game, SIGNAL(triggered()), connection_dialog, SLOT(exec()));
    connect(connection_dialog, SIGNAL(accepted()), this, SLOT(startConnection()));

    config_dialog = new ConfigDialog(this);
    connect(ui->actionConfigure, SIGNAL(triggered()), config_dialog, SLOT(show()));
    connect(config_dialog, SIGNAL(bg_changed()), this, SLOT(changeBackground()));

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAcknowledgement_2, SIGNAL(triggered()), this, SLOT(on_actionAcknowledgement_triggered()));

    StartScene *start_scene = new StartScene;

#ifdef AUDIO_SUPPORT

    if(Config.EnableBgMusic) {
        // start playing background music
        QString bgm = "audio/bgm/main.ogg";
        if (QFile::exists(bgm)) {
            Audio::stopBackgroundMusic();
            Audio::playBackgroundMusic(bgm);
            Audio::setBackgroundMusicVolume(Config.BGMVolume);
        }
    }
    Audio::setEffectVolume(Config.EffectVolume);

#endif

    QList<QAction *> actions;
    actions << ui->actionStart_Game
            << ui->actionStart_Server
            << ui->actionReplay
            << ui->actionConfigure
            << ui->actionGeneral_Overview
            << ui->actionCard_Overview
            << ui->actionScenario_Overview
            << ui->actionAbout;

    foreach (QAction *action, actions)
        start_scene->addButton(action);
    view = new FitView(scene);

    setCentralWidget(view);
    restoreFromConfig();

    BackLoader::preload();
    gotoScene(start_scene);

    addAction(ui->actionShow_Hide_Menu);
    addAction(ui->actionFullscreen);

    connect(ui->actionRestart_Game, SIGNAL(triggered()), this, SLOT(startConnection()));
    connect(ui->actionReturn_to_Main_Menu, SIGNAL(triggered()), this, SLOT(gotoStartScene()));

    systray = NULL;
}

void MainWindow::restoreFromConfig() {
    resize(Config.value("WindowSize", QSize(1366, 706)).toSize());
    move(Config.value("WindowPosition", QPoint(-8, -8)).toPoint());
    Qt::WindowStates window_state = (Qt::WindowStates)(Config.value("WindowState", 0).toInt());
    if (window_state != Qt::WindowMinimized)
        setWindowState(window_state);

    QFont font;
    if (Config.UIFont != font) {
        font = Config.AppFont;
#ifdef Q_OS_ANDROID
        font = QFont("DroidSansFallback", 6);
#endif
        QApplication::setFont(font, "QTextEdit");
    }

    ui->actionEnable_Hotkey->setChecked(Config.EnableHotKey);
    ui->actionNever_nullify_my_trick->setChecked(Config.NeverNullifyMyTrick);
    ui->actionNever_nullify_my_trick->setEnabled(false);
}

void MainWindow::closeEvent(QCloseEvent *) {
    Config.setValue("WindowSize", size());
    Config.setValue("WindowPosition", pos());
    Config.setValue("WindowState", (int)windowState());
}

MainWindow::~MainWindow() {
    delete ui;
    view->deleteLater();
    if (scene)
        scene->deleteLater();
    QSanSkinFactory::destroyInstance();
}

void MainWindow::gotoScene(QGraphicsScene *scene) {
    if (this->scene)
        this->scene->deleteLater();
    this->scene = scene;
    view->setScene(scene);
    QResizeEvent e(QSize(view->size().width() - 4, view->size().height() - 4), view->size()); // WARNING: Magic Number
    view->resizeEvent(&e);
    changeBackground();
}

void MainWindow::on_actionExit_triggered() {
    QMessageBox::StandardButton result;
    result = QMessageBox::question(this,
                                   tr("Sanguosha"),
                                   tr("Are you sure to exit?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        delete systray;
        systray = NULL;
        close();
    }
}

void MainWindow::on_actionStart_Server_triggered() {
    ServerDialog *dialog = new ServerDialog(this);
    int accept_type = dialog->config();
    if (accept_type == 0)
        return;

    server = new Server(this);
    if (!server->listen()) {
        QMessageBox::warning(this, tr("Warning"), tr("Can not start server!"));
        return;
    }

    if (accept_type == 1) {
        server->daemonize();

        ui->actionStart_Game->disconnect();
        connect(ui->actionStart_Game, SIGNAL(triggered()), this, SLOT(startGameInAnotherInstance()));

        StartScene *start_scene = qobject_cast<StartScene *>(scene);
        if (start_scene) {
            start_scene->switchToServer(server);
            if (Config.value("EnableMinimizeDialog", false).toBool())
                this->on_actionMinimize_to_system_tray_triggered();
        }
    } else {
        Config.HostAddress = "127.0.0.1";
        startConnection();
    }
}

void MainWindow::checkVersion(const QString &server_version, const QString &server_mod) {
    QString client_mod = Sanguosha->getMODName();
    if (client_mod != server_mod) {
        QMessageBox::warning(this, tr("Warning"), tr("Client MOD name is not same as the server!"));
        return;
    }

    Client *client = qobject_cast<Client *>(sender());
    QString client_version = Sanguosha->getVersionNumber();

    if (server_version == client_version) {
        client->signup();
        connect(client, SIGNAL(server_connected()), SLOT(enterRoom()));
        return;
    }

    client->disconnectFromHost();

    QString text = tr("Server version is %1, client version is %2 <br/>").arg(server_version).arg(client_version);
    if (server_version > client_version)
        text.append(tr("Your client version is older than the server's, please update it <br/>"));
    else
        text.append(tr("The server version is older than your client version, please ask the server to update<br/>"));

    text.append(tr("QQ Group: %1").arg(221093508));
    QMessageBox::warning(this, tr("Warning"), text);
}

void MainWindow::startConnection() {
    Client *client = new Client(this);

    connect(client, SIGNAL(version_checked(QString, QString)), SLOT(checkVersion(QString, QString)));
    connect(client, SIGNAL(error_message(QString)), SLOT(networkError(QString)));
}

void MainWindow::on_actionReplay_triggered() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString location = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QString last_dir = Config.value("LastReplayDir").toString();
    if (!last_dir.isEmpty())
        location = last_dir;

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a replay file"),
                                                    location,
                                                    tr("Pure text replay file (*.txt);; Image replay file (*.png)"));

    if (filename.isEmpty())
        return;

    QFileInfo file_info(filename);
    last_dir = file_info.absoluteDir().path();
    Config.setValue("LastReplayDir", last_dir);

    Client *client = new Client(this, filename);
    connect(client, SIGNAL(server_connected()), SLOT(enterRoom()));
    client->signup();
}

void MainWindow::networkError(const QString &error_msg) {
    if (isVisible())
        QMessageBox::warning(this, tr("Network error"), error_msg);
}

void BackLoader::preload() {
    QStringList emotions = G_ROOM_SKIN.getAnimationFileNames();

    foreach (QString emotion, emotions) {
        int n = PixmapAnimation::GetFrameCount(emotion);
        for (int i = 0; i < n; i++) {
            QString filename = QString("image/system/emotion/%1/%2.png").arg(emotion).arg(QString::number(i));
            G_ROOM_SKIN.getPixmapFromFileName(filename, true);
        }
    }
}

void MainWindow::enterRoom() {
    // add current ip to history
    if (!Config.HistoryIPs.contains(Config.HostAddress)) {
        Config.HistoryIPs << Config.HostAddress;
        Config.HistoryIPs.sort();
        Config.setValue("HistoryIPs", Config.HistoryIPs);
    }

    ui->actionStart_Game->setEnabled(false);
    ui->actionStart_Server->setEnabled(false);
    ui->actionReplay->setEnabled(false);
    ui->actionRestart_Game->setEnabled(false);
    ui->actionReturn_to_Main_Menu->setEnabled(false);

    RoomScene *room_scene = new RoomScene(this);
    ui->actionView_Discarded->setEnabled(true);
    ui->actionView_distance->setEnabled(true);
    ui->actionServerInformation->setEnabled(true);
    ui->actionSurrender->setEnabled(true);
    ui->actionNever_nullify_my_trick->setEnabled(true);
    ui->actionSaveRecord->setEnabled(true);
    ui->actionPause_Resume->setEnabled(true);
    ui->actionHide_Show_chat_box->setEnabled(true);

    connect(ClientInstance, SIGNAL(surrender_enabled(bool)), ui->actionSurrender, SLOT(setEnabled(bool)));

    connect(ui->actionView_Discarded, SIGNAL(triggered()), room_scene, SLOT(toggleDiscards()));
    connect(ui->actionView_distance, SIGNAL(triggered()), room_scene, SLOT(viewDistance()));
    connect(ui->actionServerInformation, SIGNAL(triggered()), room_scene, SLOT(showServerInformation()));
    connect(ui->actionSurrender, SIGNAL(triggered()), room_scene, SLOT(surrender()));
    connect(ui->actionSaveRecord, SIGNAL(triggered()), room_scene, SLOT(saveReplayRecord()));
    connect(ui->actionPause_Resume, SIGNAL(triggered()), room_scene, SLOT(pause()));
    connect(ui->actionHide_Show_chat_box, SIGNAL(triggered()), room_scene, SLOT(setChatBoxVisibleSlot()));

    if (ServerInfo.EnableCheat) {
        ui->menuCheat->setEnabled(true);

        connect(ui->actionDeath_note, SIGNAL(triggered()), room_scene, SLOT(makeKilling()));
        connect(ui->actionDamage_maker, SIGNAL(triggered()), room_scene, SLOT(makeDamage()));
        connect(ui->actionRevive_wand, SIGNAL(triggered()), room_scene, SLOT(makeReviving()));
        connect(ui->actionExecute_script_at_server_side, SIGNAL(triggered()), room_scene, SLOT(doScript()));
    } else {
        ui->menuCheat->setEnabled(false);
        ui->actionDeath_note->disconnect();
        ui->actionDamage_maker->disconnect();
        ui->actionRevive_wand->disconnect();
        ui->actionSend_lowlevel_command->disconnect();
        ui->actionExecute_script_at_server_side->disconnect();
    }

    connect(room_scene, SIGNAL(restart()), this, SLOT(startConnection()));
    connect(room_scene, SIGNAL(return_to_start()), this, SLOT(gotoStartScene()));
    connect(room_scene, SIGNAL(game_over_dialog_rejected()), this, SLOT(enableDialogButtons()));

    gotoScene(room_scene);
}

void MainWindow::gotoStartScene() {
    ServerInfo.DuringGame = false;
    delete systray;
    systray = NULL;

    if (server) {
        server->deleteLater();
        server = NULL;
    }
    if (Self) {
        delete Self;
        Self = NULL;
    }

    StartScene *start_scene = new StartScene;

    QList<QAction *> actions;
    actions << ui->actionStart_Game
            << ui->actionStart_Server
            << ui->actionReplay
            << ui->actionConfigure
            << ui->actionGeneral_Overview
            << ui->actionCard_Overview
            << ui->actionScenario_Overview
            << ui->actionAbout;

    ui->actionStart_Game->setEnabled(true);
    ui->actionStart_Server->setEnabled(true);
    ui->actionReplay->setEnabled(true);
    ui->actionRestart_Game->setEnabled(false);
    ui->actionReturn_to_Main_Menu->setEnabled(false);

    foreach (QAction *action, actions)
        start_scene->addButton(action);

    setCentralWidget(view);

    ui->menuCheat->setEnabled(false);
    ui->actionDeath_note->disconnect();
    ui->actionDamage_maker->disconnect();
    ui->actionRevive_wand->disconnect();
    ui->actionSend_lowlevel_command->disconnect();
    ui->actionExecute_script_at_server_side->disconnect();
    gotoScene(start_scene);

    addAction(ui->actionShow_Hide_Menu);
    addAction(ui->actionFullscreen);

    if (ClientInstance) {
        ClientInstance->disconnectFromHost();
        delete ClientInstance;
    }
}

void MainWindow::enableDialogButtons() {
    ui->actionRestart_Game->setEnabled(true);
    ui->actionReturn_to_Main_Menu->setEnabled(true);
}

void MainWindow::startGameInAnotherInstance() {
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
}

void MainWindow::on_actionGeneral_Overview_triggered() {
    GeneralOverview *overview = GeneralOverview::getInstance(this);
    overview->fillGenerals(Sanguosha->getGeneralList());
    overview->show();
}

void MainWindow::on_actionCard_Overview_triggered() {
    CardOverview *overview = CardOverview::getInstance(this);
    overview->loadFromAll();
    overview->show();
}

void MainWindow::on_actionEnable_Hotkey_toggled(bool checked) {
    if (Config.EnableHotKey != checked) {
        Config.EnableHotKey = checked;
        Config.setValue("EnableHotKey", checked);
    }
}

void MainWindow::on_actionNever_nullify_my_trick_toggled(bool checked) {
    if (Config.NeverNullifyMyTrick != checked) {
        Config.NeverNullifyMyTrick = checked;
        Config.setValue("NeverNullifyMyTrick", checked);
    }
}

void MainWindow::on_actionAbout_triggered() {
    // Cao Cao's pixmap
    QString content =  "<center> <br/> <img src='image/system/shencc.png'> <br/> </center>";

    int number = 221093508;
    content.append(tr("This is the open source clone of the <b>QSanguosha</b>,"
                      "totally written in C++ Qt GUI framework <br/>"
                      "Official QQ Group Number: %1<br/>"
                      "Members:<br/>"
                      "%2<br/>").arg(number).arg(Sanguosha->getMembers()));

    QString config;

#ifdef QT_NO_DEBUG
    config = "release";
#else
    config = "debug";
#endif

    content.append(tr("Current version: %1 %2 (%3)<br/>")
                      .arg(Sanguosha->getVersion())
                      .arg(config)
                      .arg(Sanguosha->getVersionName()));

    const char *date = __DATE__;
    const char *time = __TIME__;
    content.append(tr("Compilation time: %1 %2 <br/>").arg(date).arg(time));

    QString project_url = "https://github.com/BeginnerSlob/TouhouTripleSha";
    content.append(tr("Source code: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(project_url));
    content.append(tr("Tieba: TouhouTripleSha Ba"));

    Window *window = new Window(tr("About QSanguosha"), QSize(420, 570));
    scene->addItem(window);
    window->setZValue(32766);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->shift(scene->inherits("RoomScene") ? scene->width() : 0,
                  scene->inherits("RoomScene") ? scene->height() : 0);

    window->appear();
}

void MainWindow::setBackgroundBrush(bool centerAsOrigin) {
    if (scene) {
        if (centerAsOrigin) {
            QPixmap pixmap(Config.BackgroundImage);
            QBrush brush(pixmap);
            qreal sx = (qreal)width() / qreal(pixmap.width());
            qreal sy = (qreal)height() / qreal(pixmap.height());

            QTransform transform;
            transform.translate(-(qreal)width() / 2, -(qreal)height() / 2);
            transform.scale(sx, sy);
            brush.setTransform(transform);
            scene->setBackgroundBrush(brush);
        } else {
            QString pic = "image/system/backdrop/tableBg.jpg";
            if (ServerInfo.GameMode == "chunxue")
                pic = "image/system/backdrop/chunxue.jpg";
            else if (ServerInfo.GameMode == "couple")
                pic = "image/system/backdrop/couple.jpg";
            else if (ServerInfo.GameMode == "02_1v1")
                pic = "image/system/backdrop/KOF.jpg";
            else if (ServerInfo.GameMode == "03_1v1v1")
                pic = "image/system/backdrop/tripod.jpg";
            else if (ServerInfo.GameMode == "jianniang")
                pic = "image/system/backdrop/jianniang.jpg";
            else if (ServerInfo.GameMode == "04_1v3")
                pic = "image/system/backdrop/hulao.jpg";
            QPixmap pixmap(pic);
            QBrush brush(pixmap);
            qreal sx = (qreal)(width() - 217) / qreal(pixmap.width());
            qreal sy = (qreal)(height() - 170) / qreal(pixmap.height());
            QTransform transform;
            transform.scale(sx, sy);
            brush.setTransform(transform);
            scene->setBackgroundBrush(brush);
        }
    }
}

void MainWindow::changeBackground() {
    bool centerAsOrigin = scene != NULL && !scene->inherits("RoomScene");
    setBackgroundBrush(centerAsOrigin);

    if (scene->inherits("StartScene")) {
        StartScene *start_scene = qobject_cast<StartScene *>(scene);
        start_scene->setServerLogBackground();
    }
}

void MainWindow::on_actionFullscreen_triggered()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::on_actionShow_Hide_Menu_triggered()
{
    QMenuBar *menu_bar = menuBar();
    menu_bar->setVisible(! menu_bar->isVisible());
}

void MainWindow::on_actionMinimize_to_system_tray_triggered()
{
    if (systray == NULL) {
        QIcon icon("image/system/magatamas/4.png");
        systray = new QSystemTrayIcon(icon, this);

        QAction *appear = new QAction(tr("Show main window"), this);
        connect(appear, SIGNAL(triggered()), this, SLOT(show()));
        connect(appear, SIGNAL(triggered()), systray, SLOT(hide()));

        QMenu *menu = new QMenu;
        menu->addAction(appear);
        menu->addMenu(ui->menuGame);
        menu->addMenu(ui->menuView);
        menu->addMenu(ui->menuOptions);
        menu->addMenu(ui->menuHelp);

        systray->setContextMenu(menu);
    }
    systray->show();
    systray->showMessage(windowTitle(), tr("Game is minimized"));

    hide();
}

void MainWindow::on_actionRole_assign_table_triggered()
{
    QString content;

    QStringList headers;
    headers << tr("Count") << tr("Lord") << tr("Loyalist") << tr("Rebel") << tr("Renegade");
    foreach (QString header, headers)
        content += QString("<th>%1</th>").arg(header);

    content = QString("<tr>%1</tr>").arg(content);

    QStringList rows;
    rows << "2 1 0 1 0" << "3 1 0 1 1" << "4 1 0 2 1"
         << "5 1 1 2 1" << "6 1 1 3 1" << "6d 1 1 2 2"
         << "7 1 2 3 1" << "8 1 2 4 1" << "8d 1 2 3 2"
         << "8z 1 3 4 0" << "9 1 3 4 1" << "10 1 3 4 2"
         << "10z 1 4 5 0" << "10o 1 3 5 1";

    foreach (QString row, rows) {
        QStringList cells = row.split(" ");
        QString header = cells.takeFirst();
        if (header.endsWith("d")) {
            header.chop(1);
            header += tr(" (double renegade)");
        }
        if (header.endsWith("z")) {
            header.chop(1);
            header += tr(" (no renegade)");
        }
        if (header.endsWith("o")) {
            header.chop(1);
            header += tr(" (single renegade)");
        }

        QString row_content;
        row_content = QString("<td>%1</td>").arg(header);
        foreach (QString cell, cells)
            row_content += QString("<td>%1</td>").arg(cell);

        content += QString("<tr>%1</tr>").arg(row_content);
    }

    content = QString("<table border='1'>%1</table").arg(content);

    Window *window = new Window(tr("Role assign table"), QSize(240, 450));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->shift(scene && scene->inherits("RoomScene") ? scene->width() : 0,
                  scene && scene->inherits("RoomScene") ? scene->height() : 0);
    window->setZValue(32766);

    window->appear();
}

void MainWindow::on_actionScenario_Overview_triggered()
{
    ScenarioOverview *dialog = new ScenarioOverview(this);
    dialog->show();
}

BroadcastBox::BroadcastBox(Server *server, QWidget *parent)
    : QDialog(parent), server(server)
{
    setWindowTitle(tr("Broadcast"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Please input the message to broadcast")));

    text_edit = new QTextEdit;
    layout->addWidget(text_edit);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    QPushButton *ok_button = new QPushButton(tr("OK"));
    hlayout->addWidget(ok_button);

    layout->addLayout(hlayout);

    setLayout(layout);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
}

void BroadcastBox::accept() {
    QDialog::accept();
    server->broadcast(text_edit->toPlainText());
}

void MainWindow::on_actionBroadcast_triggered() {
    Server *server = findChild<Server *>();
    if (server == NULL) {
        QMessageBox::warning(this, tr("Warning"), tr("Server is not started yet!"));
        return;
    }

    BroadcastBox *dialog = new BroadcastBox(server, this);
    dialog->exec();
}

void MainWindow::on_actionAcknowledgement_triggered() {
    Window *window = new Window(QString(), QSize(1000, 677), "image/system/acknowledgement.png");
    scene->addItem(window);

    Button *button = window->addCloseButton(tr("OK"));
    button->moveBy(-85, -35);
    window->setZValue(32766);
    window->shift(scene && scene->inherits("RoomScene") ? scene->width() : 0,
                  scene && scene->inherits("RoomScene") ? scene->height() : 0);

    window->appear();
}

#include <QGroupBox>
#include <QToolButton>
#include <QCommandLinkButton>
#include <QFormLayout>
#include "recorder.h"

void MainWindow::on_actionReplay_file_convert_triggered() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Please select a replay file"),
                                                    Config.value("LastReplayDir").toString(),
                                                    tr("Pure text replay file (*.txt);; Image replay file (*.png)"));

    if (filename.isEmpty())
        return;

    QFile file(filename);
    bool success = false;
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo info(filename);
        QString tosave = info.absoluteDir().absoluteFilePath(info.baseName());
        QString suffix = filename.right(4).toLower();

        if (suffix == ".txt") {
            tosave.append(".png");

            // txt to png
            Recorder::TXT2PNG(file.readAll()).save(tosave);
            success = true;
        } else if (suffix == ".png") {
            tosave.append(".txt");

            // png to txt
            QByteArray data = Recorder::PNG2TXT(filename);

            QFile tosave_file(tosave);
            if (!data.isEmpty() && tosave_file.open(QIODevice::WriteOnly)) {
                tosave_file.write(data);
                success = true;
            }
        }
    }
    if (!success)
        QMessageBox::warning(this, tr("Replay file convert"), tr("Conversion failed!"));
    else
        QMessageBox::warning(this, tr("Replay file convert"), tr("Conversion done!"));
}

void MainWindow::on_actionRecord_analysis_triggered() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString location = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Load replay record"),
                                                    location,
                                                    tr("Pure text replay file (*.txt);; Image replay file (*.png)"));

    if (filename.isEmpty()) return;

    QDialog *rec_dialog = new QDialog(this);
    rec_dialog->setWindowTitle(tr("Record Analysis"));
    rec_dialog->resize(800, 500);
    QTableWidget *table = new QTableWidget;

    RecAnalysis *record = new RecAnalysis(filename);
    QMap<QString, PlayerRecordStruct *> record_map = record->getRecordMap();
    table->setColumnCount(11);
    table->setRowCount(record_map.keys().length());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    static QStringList labels;
    if (labels.isEmpty()) {
        labels << tr("ScreenName") << tr("General") << tr("Role") << tr("Living") << tr("WinOrLose") << tr("TurnCount")
               << tr("Recover") << tr("Damage") << tr("Damaged") << tr("Kill") << tr("Designation");
    }
    table->setHorizontalHeaderLabels(labels);
    table->setSelectionBehavior(QTableWidget::SelectRows);

    int i = 0;
    foreach (PlayerRecordStruct *rec, record_map.values()) {
        QTableWidgetItem *item = new QTableWidgetItem;
        QString screen_name = Sanguosha->translate(rec->m_screenName);
        if (rec->m_statue == "robot")
            screen_name += "(" + Sanguosha->translate("robot") + ")";

        item->setText(screen_name);
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        QString generals = Sanguosha->translate(rec->m_generalName);
        if (!rec->m_general2Name.isEmpty())
            generals += "/" + Sanguosha->translate(rec->m_general2Name);
        item->setText(generals);
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        item->setText(Sanguosha->translate(rec->m_role));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;
        item->setText(rec->m_isAlive ? tr("Alive") : tr("Dead"));
        table->setItem(i, 3, item);

        item = new QTableWidgetItem;
        bool is_win = record->getRecordWinners().contains(rec->m_role)
                      || record->getRecordWinners().contains(record_map.key(rec));
        item->setText(is_win ? tr("Win") : tr("Lose"));
        table->setItem(i, 4, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_turnCount));
        table->setItem(i, 5, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_recover));
        table->setItem(i, 6, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damage));
        table->setItem(i, 7, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damaged));
        table->setItem(i, 8, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_kill));
        table->setItem(i, 9, item);

        item = new QTableWidgetItem;
        item->setText(rec->m_designation.join(", "));
        table->setItem(i, 10, item);
        i++;
    }

    table->resizeColumnsToContents();

    QLabel *label = new QLabel;
    label->setText(tr("Packages:"));

    QTextEdit *package_label = new QTextEdit;
    package_label->setReadOnly(true);
    package_label->setText(record->getRecordPackages().join(", "));

    QLabel *label_game_mode = new QLabel;
    label_game_mode->setText(tr("GameMode:") + Sanguosha->getModeName(record->getRecordGameMode()));

    QLabel *label_options = new QLabel;
    label_options->setText(tr("ServerOptions:") + record->getRecordServerOptions().join(","));

    QTextEdit *chat_info = new QTextEdit;
    chat_info->setReadOnly(true);
    chat_info->setText(record->getRecordChat());

    QLabel *table_chat_title = new QLabel;
    table_chat_title->setText(tr("Chat Information:"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(package_label);
    layout->addWidget(label_game_mode);
    layout->addWidget(label_options);
    layout->addWidget(table);
    layout->addSpacing(15);
    layout->addWidget(table_chat_title);
    layout->addWidget(chat_info);
    rec_dialog->setLayout(layout);

    rec_dialog->exec();
}

void MainWindow::on_actionView_ban_list_triggered() {
    BanlistDialog *dialog = new BanlistDialog(this, true);
    dialog->exec();
}

#include "audio.h"

void MainWindow::on_actionAbout_fmod_triggered() {
    QString content = tr("FMOD is a proprietary audio library made by Firelight Technologies");
    content.append("<p align='center'> <img src='image/logo/fmod.png' /> </p> <br/>");

    QString address = "http://www.fmod.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

#ifdef AUDIO_SUPPORT
    content.append(tr("Current versionn %1 <br/>").arg(Audio::getVersion()));
#endif

    Window *window = new Window(tr("About fmod"), QSize(500, 260));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene && scene->inherits("RoomScene") ? scene->width() : 0,
                  scene && scene->inherits("RoomScene") ? scene->height() : 0);

    window->appear();
}

#include "lua.hpp"

void MainWindow::on_actionAbout_Lua_triggered() {
    QString content = tr("Lua is a powerful, fast, lightweight, embeddable scripting language.");
    content.append("<p align='center'> <img src='image/logo/lua.png' /> </p> <br/>");

    QString address = "http://www.lua.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

    content.append(tr("Current versionn %1 <br/>").arg(LUA_RELEASE));
    content.append(LUA_COPYRIGHT);

    Window *window = new Window(tr("About Lua"), QSize(500, 585));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene && scene->inherits("RoomScene") ? scene->width() : 0,
                  scene && scene->inherits("RoomScene") ? scene->height() : 0);

    window->appear();
}

void MainWindow::on_actionAbout_GPLv3_triggered() {
    QString content = tr("The GNU General Public License is the most widely used free software license, which guarantees end users the freedoms to use, study, share, and modify the software.");
    content.append("<p align='center'> <img src='image/logo/gplv3.png' /> </p> <br/>");

    QString address = "http://gplv3.fsf.org";
    content.append(tr("Official site: <a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(address));

    Window *window = new Window(tr("About GPLv3"), QSize(500, 225));
    scene->addItem(window);

    window->addContent(content);
    window->addCloseButton(tr("OK"));
    window->setZValue(32766);
    window->shift(scene && scene->inherits("RoomScene") ? scene->width() : 0,
                  scene && scene->inherits("RoomScene") ? scene->height() : 0);

    window->appear();
}

void MainWindow::checkUpdate()
{
    static QNetworkAccessManager *qnam = new QNetworkAccessManager(this);
    reply = qnam->get(QNetworkRequest(QUrl("http://ver.qsanguosha.org/UpdateInfoForTouhouTripleSha")));
    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
}

void MainWindow::httpFinished()
{
    QString newVersionNumber, updateDate, downloadUrl;
    QStringList avaliableVersion;
    bool has_new_version = false;
    while (!reply->atEnd()) {
        QString line = reply->readLine();
        line.remove('\n');

        QStringList texts1 = line.split('=', QString::SkipEmptyParts);
        QStringList texts2 = line.split(':', QString::SkipEmptyParts);

        if (texts1.size() == 2) {
            QString key = texts1.at(0);
            QString value = texts1.at(1);
            if (key == "VersionNumber") {
                if (Sanguosha->getVersionNumber() < value)
                    has_new_version = true;
                newVersionNumber = value;
            } else if (key == "UpdateDate") {
                if (Sanguosha->getMODName() < value.right(4))
                    has_new_version = true;
                updateDate = value;
            } else if (key == "url") {
                downloadUrl = value;
            } else if (key == "avaliable") {
                avaliableVersion << value.split("|");
            }
        } else if (texts2.size() == 2) {
            QString key = texts2.at(0);
            QString value = texts2.at(1);
            if (key == "VersionNumber") {
                if (Sanguosha->getVersionNumber() < value)
                    has_new_version = true;
                newVersionNumber = value;
            } else if (key == "UpdateDate") {
                if (Sanguosha->getMODName() < value.right(4))
                    has_new_version = true;
                updateDate = value;
            }
        }
    }
    if (has_new_version) {
        setWindowTitle(tr("New Version Available") + "  " + windowTitle());
        if (downloadUrl.isEmpty() || (!avaliableVersion.contains(Sanguosha->getVersionNumber()))) {
            QMessageBox::warning(this, tr("New Version Available"),
                                 tr("There is a new version for TouhouTripleSha<br/> \
                                     The version number is %1<br/> \
                                     Update date is %2<br/> \
                                     Please download it in our QQ Group<br/> \
                                     The Group number is 221093508<br/> \
                                     Hope you enjoy this game").arg(newVersionNumber).arg(updateDate),
                                 QMessageBox::Ok, QMessageBox::Ok);
        } else {
            if (QMessageBox::question(this, tr("New Version Available"),
                                      tr("There is a new version for TouhouTripleSha<br/>"
                                         "The version number is %1<br/>"
                                         "Update date is %2<br/>"
                                         "Would you wanna download now?").arg(newVersionNumber).arg(updateDate))
                    == QMessageBox::Yes) {
                QMessageBox::warning(this, tr("Tips"),
                                     tr("The new version will be downloading<br/>"
                                        "it may takes sereval minutes if the update is big<br/>"
                                        "you can do any thing you want but DO NOT close the game<br/>"
                                        "otherwise the download will be canceled<br/>"
                                        "we will call a pop-up window if the download is finished."));
                downloadNew(downloadUrl);
            }
        }
    }
    reply->deleteLater();
    reply = NULL;
}

void MainWindow::downloadNew(QString url)
{
    QUrl url2(url);
    QFileInfo fileInfo(url2.path());
    fileName = fileInfo.fileName();
    if (fileName.isEmpty())
        fileName = "index.html";
    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("Download New Version"),
                                  tr("There already exists a file called %1 in "
                                     "the current directory. Overwrite?").arg(fileName),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                == QMessageBox::No)
            return;
        QFile::remove(fileName);
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Download New Version"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        file = NULL;
        return;
    }

    updateWindow = new UpdateDialog(this);
    updateWindow->move(width() - 110, 30);
    updateWindow->setStyleSheet("border:1px solid gray; background-color: rgba(208, 127, 108, 100)");
    updateWindow->show();

    static QNetworkAccessManager *qnam2 = new QNetworkAccessManager(this);
    reply2 = qnam2->get(QNetworkRequest(url2));
    connect(reply2, SIGNAL(finished()), this, SLOT(httpFinished2()));
    connect(reply2, SIGNAL(readyRead()), this, SLOT(httpReadyRead2()));
    connect(reply2, SIGNAL(downloadProgress(qint64,qint64)), updateWindow, SLOT(updateDataReadProgress2(qint64,qint64)));
}

void MainWindow::httpFinished2()
{
    file->flush();
    file->close();

    QVariant redirectionTarget = reply2->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply2->error()) {
        file->remove();
        QMessageBox::information(this, tr("Download New Version"),
                                 tr("Download failed: %1.")
                                 .arg(reply2->errorString()));
    } else if (!redirectionTarget.isNull()) {
        QMessageBox::warning(this, tr("Download New Version"),
                             tr("Redirect Error!"));
        return;
    } else {
#ifdef Q_OS_WIN
        if (QMessageBox::question(this, tr("Download New Version"),
                                  tr("Download Finish!<br/>"
                                     "Do you want to update now?<br/>"
                                     "Tips: The game will restart after finish update"))
                == QMessageBox::Yes) {
            QStringList cmd_lists;
            cmd_lists << "taskkill /f /IM touhoutriplesha.exe";
            cmd_lists << "7za x " + fileName + " -aoa";
            cmd_lists << "del " + fileName;
            cmd_lists << "start touhoutriplesha";
            QString system_str = "cmd /c " + cmd_lists.join("&");
            WinExec(system_str.toLatin1().data(), SW_HIDE);
        } else
#endif
            QMessageBox::warning(this, tr("Download New Version"),
                                 tr("The update package has been saved at root folder"));
    }

    reply->deleteLater();
    reply = NULL;
    delete file;
    file = NULL;
}

void MainWindow::httpReadyRead2()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (file)
        file->write(reply2->readAll());
}

UpdateDialog::UpdateDialog(QWidget *parent)
    : QFrame(parent, Qt::FramelessWindowHint)
{
    setWindowTitle("Update");
    progressBar = new QProgressBar(this);
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(progressBar);
    this->setLayout(mainLayout);
}

void UpdateDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_CurrentPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void UpdateDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        move(event->globalPos() - m_CurrentPos);
        event->accept();
    }
}

void UpdateDialog::paintEvent(QPaintEvent *)
{
}

void UpdateDialog::updateDataReadProgress2(qint64 bytesRead, qint64 totalBytes)
{
    progressBar->setMaximum(totalBytes);
    progressBar->setValue(bytesRead);
}
