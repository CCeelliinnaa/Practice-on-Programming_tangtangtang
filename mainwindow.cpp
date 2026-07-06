#include "mainwindow.h"
#include <QPainter>
#include <QTimer>
#include <QMessageBox>
#include <QMouseEvent>
#include "ui_mainwindow.h"
#include <QPixmap>

mainwindow::mainwindow(QWidget *parent)
    : QWidget(parent), currentWave(0), totalWaves(10), difficulty(1), elapsedTime(0), isGameOver(false), showButtons(false), gameSpeed(1),
      selectedTower(nullptr), selectedObstacle(nullptr), isPaused(false), countdownValue(4),isVictory(false)
{
    ui = new Ui::mainwindow;
    ui->setupUi(this);
    this->setWindowTitle("Carrot_Defence保卫萝卜");
    this->setFixedSize(1200, 800);

    bgmPlayer = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);
    bgmPlayer->setAudioOutput(audioOutput);

    bgmPlayer->setSource(QUrl("qrc:/sounds/sounds/background_music.mp3"));  // Qt6 setSource
    audioOutput->setVolume(0.01);  // 1%音量，对应Qt5的setVolume(1)

    bgmPlayer->play();

    // Qt6手动循环播放（setLoops也可用，但为通用性保留手动方式）
    connect(bgmPlayer, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) {
                if (state == QMediaPlayer::StoppedState) {
                    bgmPlayer->play();
                }
            });
    mapPixmap.load(":/images/images/Background2.png");
    setFixedSize(1200, 800);
    waveTimerId = -1;
    radish = new Radish(QPoint(750, 600));
    gameOverPixmap.load(":/images/images/lose2.png");
    winPixmap.load(":/images/images/win2.png");
    winPixmap = winPixmap.scaled(1200,800);
    confirmButtonRect = QRect(479, 647, 243, 56);
    money = new Money(450);
    coinPixmap.load(":/images/images/coin.png");

    QPixmap pauseNormalPixmap(":/images/images/pause_normal.png");
    QPixmap pauseHoverPixmap(":/images/images/pause_hover.png");
    QPixmap doubleSpeedNormalPixmap(":/images/images/double_speed_normal.png");
    QPixmap doubleSpeedHoverPixmap(":/images/images/double_speed_hover.png");
    QPixmap coffeeButtonPixmap(":/images/images/cannon_button.png");
    QPixmap musicButtonPixmap(":/images/images/poop_button.png");
    QPixmap aiButtonPixmap(":/images/images/star_button.png");
    QPixmap upgradeNormalGrayPixmap(":/images/images/upgrade_normal_gray.png");
    QPixmap upgradeNormalBluePixmap(":/images/images/upgrade_normal_blue.png");
    QPixmap upgradeHoverBluePixmap(":/images/images/upgrade_hover_blue.png");
    QPixmap removeNormalPixmap(":/images/images/remove_normal.png");
    QPixmap removeHoverPixmap(":/images/images/remove_hover.png");
    QPixmap crossPixmap(":/images/images/cross.png");

    QPixmap scaledPausePixmap = pauseNormalPixmap.scaled(60, 60, Qt::KeepAspectRatio);
    QPixmap scaledPauseHoverPixmap = pauseHoverPixmap.scaled(60, 60, Qt::KeepAspectRatio);
    QPixmap scaledDoubleSpeedPixmap = doubleSpeedNormalPixmap.scaled(120, 120, Qt::KeepAspectRatio);
    QPixmap scaledDoubleSpeedHoverPixmap = doubleSpeedHoverPixmap.scaled(120, 120, Qt::KeepAspectRatio);

    buttons.append(new Button(QPoint(1000, 10), scaledPausePixmap, scaledPauseHoverPixmap, Button::PAUSE));
    buttons.append(new Button(QPoint(800, 10), scaledDoubleSpeedPixmap, scaledDoubleSpeedHoverPixmap, Button::DOUBLE_SPEED));

    towerPits = TowerPit::createTowerPits();
    QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};

    QString obstaclePaths[10] = {
        ":/images/images/buildings_01.png",
        ":/images/images/obstacle2.png",
        ":/images/images/obstacle3.png",
        ":/images/images/obstacle4.png",
        ":/images/images/obstacle5.png",
        ":/images/images/obstacle6.png",
        ":/images/images/buildings_03.png",
        ":/images/images/bulidings_02.png",
        ":/images/images/obstacle9.png",
        ":/images/images/buildings_04.png"
    };
    // 障碍物大小
    QList<QSize> obstacleSizes = {
        QSize(180, 180),
        QSize(100, 100),
        QSize(100, 100),
        QSize(100, 100),
        QSize(100, 100),
        QSize(100, 100),
        QSize(180, 180),
        QSize(180, 180),
        QSize(100, 100),
        QSize(180, 180)
    };

        int pathIndex = 0;
        for (int index : targetIndices) {
            for (TowerPit* pit : towerPits) {
                if (pit->getIndex() == index) {
                    QSize size = obstacleSizes[pathIndex];
                    Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                    obstacles.append(obstacle);
                    pit->setObstacle(obstacle);
                    pathIndex++;
                    break;
                }
            }
        }

    startWindow = new StartWindow(this);
    startWindow->show();

    difficultyWindow = new DifficultyWindow(this);
    difficultyWindow->hide();

    connect(startWindow, &StartWindow::startGame, this, &mainwindow::onStartGame);
    connect(difficultyWindow, &DifficultyWindow::difficultySelected, this, &mainwindow::onDifficultySelected);
    connect(startWindow, &StartWindow::showRules, this, &mainwindow::onShowRules);

    countdownLabel = new QLabel(this);
    countdownLabel->setGeometry(500, 300, 200, 200);
    countdownLabel->hide();
    countdownLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &mainwindow::countdown);
    countdownPixmaps[0].load(":/images/images/3.png");
    countdownPixmaps[1].load(":/images/images/2.png");
    countdownPixmaps[2].load(":/images/images/1.png");
    countdownPixmaps[3].load(":/images/images/GO.png");
    restartButtonRect = QRect(433, 459, 282, 66);
    returnButtonRect = QRect(434, 559, 280, 70);
}

mainwindow::~mainwindow()
{
    for (Enemy* enemy : enemies) delete enemy;
    for (Tower* tower : towers) delete tower;
    for (Button* button : buttons) delete button;
    for (Obstacle* obstacle : obstacles) delete obstacle;
    for (TowerPit* pit : towerPits) delete pit;
    delete radish;
    delete money;
    delete ui;
    delete startWindow;
    delete countdownLabel;
    delete countdownTimer;
    delete bgmPlayer;
}

void mainwindow::onStartGame()
{
    startWindow->hide();
    startWindow->setAttribute(Qt::WA_TransparentForMouseEvents);
    difficultyWindow->show();
    difficultyWindow->raise();
}

void mainwindow::onDifficultySelected(int diff)
{
    difficulty = diff;
    switch (difficulty) {
    case 0: totalWaves = 8; break;   // 简单
    case 1: totalWaves = 10; break;  // 中等
    case 2: totalWaves = 12; break;  // 困难
    }
    difficultyWindow->hide();
    difficultyWindow->setAttribute(Qt::WA_TransparentForMouseEvents);
    countdownValue = 4;
    countdownLabel->show();
    Tower::setGameSpeed(gameSpeed);
    countdownTimer->start(1000);
}

void mainwindow::onShowRules()
{
    QMessageBox::information(this, "彩蛋", "恭喜你触发隐藏剧情：保送清华！后续内容敬请期待！");
}

void mainwindow::countdown()
{
    if (countdownValue > 0) {
        countdownValue--;
        update();
    } else {
        countdownTimer->stop();
        countdownLabel->hide();
        elapsedTime = 0;
        currentWave = 0;
        waveTimerId = startTimer(100);
    }
}

void mainwindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, backgroundPixmap.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    painter.drawPixmap(0, 0, mapPixmap.scaled(width(), height()));
    if (!startWindow->isVisible()) {
        painter.drawPixmap(0, 0, mapPixmap.scaled(width(), height()));
    }
    for (Enemy* enemy : enemies) {
        enemy->draw(&painter);
    }
    QPixmap pitPixmap(":/images/images/tower_pit.png");
    for (TowerPit* pit : towerPits) {
        painter.drawPixmap(pit->getPosition(), pitPixmap.scaled(80,80,Qt::KeepAspectRatio));
    }
    for (Tower* tower : towers) {
        tower->draw(&painter, enemies);
        for (Bullet* bullet : tower->getBullets()) {
            bullet->draw(&painter);
        }
    }

    radish->draw(&painter);

    for (Obstacle* obstacle : obstacles) {
        obstacle->draw(&painter);
    }
    if (countdownValue > 0 && countdownValue <= 4) {
        QPixmap countdownPixmap = countdownPixmaps[4 - countdownValue];
        painter.drawPixmap((width() - countdownPixmap.width()) / 2, (height() - countdownPixmap.height()) / 2, countdownPixmap);
    }
        if (showButtons) {
            QPixmap coffeeButtonPixmap(":/images/images/button_1.png");
            QPixmap musicButtonPixmap(":/images/images/button_4.png");
            QPixmap aiButtonPixmap(":/images/images/button_2.png");
            QPixmap teacherButtonPixmap(":/images/images/button_3.png");

            int buttonSize = 100;
            int gap = 10;
            int totalWidth = 4 * buttonSize + 3 * gap;
            int totalHeight = buttonSize;

            // 按钮位置防止超出窗口边界
            int bx = clickPos.x();
            int by = clickPos.y();
            if (bx + totalWidth > width()) bx = width() - totalWidth;
            if (bx < 0) bx = 0;
            if (by + totalHeight > height()) by = by - totalHeight;
            if (by < 0) by = 0;

            painter.drawPixmap(bx, by, coffeeButtonPixmap.scaled(buttonSize, buttonSize));
            painter.drawPixmap(bx + buttonSize + gap, by, musicButtonPixmap.scaled(buttonSize, buttonSize));
            painter.drawPixmap(bx + 2 * (buttonSize + gap), by, aiButtonPixmap.scaled(buttonSize, buttonSize));
            painter.drawPixmap(bx + 3 * (buttonSize + gap), by, teacherButtonPixmap.scaled(buttonSize, buttonSize));
        }

        if (selectedTower) {
            QPixmap upgradePixmap;
            if (money->canAfford(selectedTower->getUpgradeCost()) && selectedTower->get_level() == 1) {
                upgradePixmap = QPixmap(":/images/images/upgrade_normal_blue.png").scaled(50,50,Qt::KeepAspectRatio);
            }
            else {
                upgradePixmap = QPixmap(":/images/images/upgrade_normal_gray.png").scaled(50,50,Qt::KeepAspectRatio);
            }
            QPoint upgradePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() - upgradePixmap.height() - 20);
            painter.drawPixmap(upgradePos, upgradePixmap);

            QPixmap removePixmap;
            removePixmap=QPixmap(":/images/images/remove_normal.png").scaled(50,50,Qt::KeepAspectRatio);
            QPoint removePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() + 100);
            painter.drawPixmap(removePos, removePixmap);
        }
    QFont font;
    font.setFamily("Times New Roman");
    font.setPointSize(16);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(130, 50, QString("Money: %1").arg(money->getAmount()));

    for (Button* button : buttons) {
        button->draw(&painter);
    }
        QString waveText;
        if (isPaused) {
            waveText = "暂停中";
        } else {
            waveText = QString("%1/%2").arg(currentWave + 1).arg(totalWaves);
        }
        QFont waveFont;
        waveFont.setFamily("Ubuntu");
        waveFont.setPointSize(20);
        waveFont.setBold(true);
        painter.setFont(waveFont);
        QRect waveRect(0, 15, width(), 50);
        painter.drawText(waveRect, Qt::AlignHCenter | Qt::AlignVCenter, waveText);
        if (isVictory) {
            painter.drawPixmap(0, 0, width(), height(), winPixmap);
        }
        else if (isGameOver) {
            painter.drawPixmap(0, 0, width(), height(), gameOverPixmap);
        }
}

void mainwindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == waveTimerId) {
        if (isGameOver || isPaused) {
            return;
        }

        for (int i = 0; i < gameSpeed; i++) {
            elapsedTime += 100;

            // 第1波：0-20秒
            if (currentWave == 0 && elapsedTime <= 20000) {
                if (elapsedTime % 3000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (elapsedTime % 5000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
            }
            // 第1波结束→第2波（间隔10秒）
            else if (currentWave == 0 && elapsedTime >= 30000) {
                currentWave = 1;
                elapsedTime = 30000;
            }

            // 第2波：30-50秒
            if (currentWave == 1 && elapsedTime - 30000 <= 20000) {
                int waveTime = elapsedTime - 30000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
            }
            // 第2波结束→第3波
            else if (currentWave == 1 && elapsedTime >= 60000) {
                currentWave = 2;
                elapsedTime = 60000;
            }

            // 第3波：60-80秒
            if (currentWave == 2 && elapsedTime - 60000 <= 20000) {
                int waveTime = elapsedTime - 60000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第3波结束→第4波
            else if (currentWave == 2 && elapsedTime >= 90000) {
                currentWave = 3;
                elapsedTime = 90000;
            }

            // 第4波：90-110秒
            if (currentWave == 3 && elapsedTime - 90000 <= 20000) {
                int waveTime = elapsedTime - 90000;
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第4波结束→第5波
            else if (currentWave == 3 && elapsedTime >= 120000) {
                currentWave = 4;
                elapsedTime = 120000;
            }

            // 第5波：120-150秒
            if (currentWave == 4 && elapsedTime - 120000 <= 30000) {
                int waveTime = elapsedTime - 120000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 6000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
            }
            // 第5波结束→第6波
            else if (currentWave == 4 && elapsedTime >= 160000) {
                currentWave = 5;
                elapsedTime = 160000;
            }

            // 第6波：160-180秒
            if (currentWave == 5 && elapsedTime - 160000 <= 20000) {
                int waveTime = elapsedTime - 160000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第6波结束→第7波
            else if (currentWave == 5 && elapsedTime >= 190000) {
                currentWave = 6;
                elapsedTime = 190000;
            }

            // 第7波：190-210秒
            if (currentWave == 6 && elapsedTime - 190000 <= 20000) {
                int waveTime = elapsedTime - 190000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第7波结束→第8波
            else if (currentWave == 6 && elapsedTime >= 220000) {
                currentWave = 7;
                elapsedTime = 220000;
            }

            // 第8波：220-240秒（中等/困难）
            if (difficulty != 0 && currentWave == 7 && elapsedTime - 220000 <= 20000) {
                int waveTime = elapsedTime - 220000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 简单难度：第8波=Boss波
            if (difficulty == 0 && currentWave == 7 && elapsedTime - 220000 <= 40000) {
                int waveTime = elapsedTime - 220000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                if (waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
            }
            // 简单难度：等待Boss消灭
            else if (difficulty == 0 && currentWave == 7 && elapsedTime >= 260000) {
                // 等待Boss被消灭
            }

            // 第8波结束→第9波
            else if (currentWave == 7 && elapsedTime >= 250000) {
                currentWave = 8;
                elapsedTime = 250000;
            }

            // 第9波：250-270秒
            if (currentWave == 8 && elapsedTime - 250000 <= 20000) {
                int waveTime = elapsedTime - 250000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第9波结束→第10波
            else if (currentWave == 8 && elapsedTime >= 280000) {
                currentWave = 9;
                elapsedTime = 280000;
            }

            // 第10波：280-320秒（中等Boss，困难普通）
            if (currentWave == 9 && elapsedTime - 280000 <= 40000) {
                int waveTime = elapsedTime - 280000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                // 中等难度Boss
                if (difficulty == 1 && waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
                // 困难难度继续
            }
            // 第10波结束→第11波
            else if (currentWave == 9 && difficulty == 2 && elapsedTime >= 330000) {
                currentWave = 10;
                elapsedTime = 330000;
            }
            // 中等难度：等待Boss消灭
            else if (currentWave == 9 && difficulty != 2 && elapsedTime >= 320000) {
                // 等待Boss被消灭
            }

            // 第11波：330-370秒
            if (currentWave == 10 && elapsedTime - 330000 <= 40000) {
                int waveTime = elapsedTime - 330000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
            }
            // 第11波结束→第12波
            else if (currentWave == 10 && elapsedTime >= 380000) {
                currentWave = 11;
                elapsedTime = 380000;
            }

            // 第12波：380-420秒（最终Boss）
            if (currentWave == 11 && elapsedTime - 380000 <= 40000) {
                int waveTime = elapsedTime - 380000;
                if (waveTime % 2000 == 0) {
                    generateMonster(Enemy::Monster1);
                }
                if (waveTime % 3000 == 0) {
                    generateMonster(Enemy::Monster2);
                }
                if (waveTime % 4000 == 0) {
                    generateMonster(Enemy::Monster3);
                }
                if (waveTime % 4500 == 0) {
                    generateMonster(Enemy::Monster4);
                }
                if (waveTime % 5000 == 0) {
                    generateMonster(Enemy::Monster5);
                }
                // 困难难度Boss
                if (waveTime == 40000) {
                    generateMonster(Enemy::MonsterBoss);
                }
            }
            // 困难难度：等待Boss消灭
            else if (currentWave == 11 && elapsedTime >= 420000) {
                // 等待Boss被消灭
            }

            // 移动所有敌人
            for (auto it = enemies.begin(); it != enemies.end(); ) {
                Enemy* enemy = *it;
                enemy->move();
                if (enemy->getHealth() <= 0 || isEnemyAtEnd(enemy)) {
            // Boss被消灭且萝卜存活→胜利
            if (enemy->getType() == Enemy::MonsterBoss && enemy->getHealth() <= 0 && radish->getHealth() > 0) {
                isVictory = true;
                killTimer(waveTimerId);
                waveTimerId = -1;
            }
            if (enemy->getHealth() <= 0) {
                switch (enemy->getType()) {
                case Enemy::Monster1: money->earn(50); break;
                case Enemy::Monster2: money->earn(80); break;
                case Enemy::Monster3: money->earn(80); break;
                case Enemy::Monster4: money->earn(80); break;
                case Enemy::Monster5: money->earn(80); break;
                case Enemy::MonsterBoss: money->earn(200); break;
                }
            }
            if (isEnemyAtEnd(enemy)) {
                checkEnemyCollideWithRadish();
            }
            delete enemy;
            it = enemies.erase(it);
                } else {
                    ++it;
                }
            }

            // 炮塔攻击
            for (Tower* tower : towers) {
                if (selectedObstacle && tower->isInRange(selectedObstacle->getPosition())) {
                    tower->attackObstacle(selectedObstacle);
                } else {
                    tower->attack(enemies);
                }
                tower->updateBullets();
            }

            // 障碍物摧毁处理
            for (auto it = obstacles.begin(); it != obstacles.end(); ) {
                Obstacle* obstacle = *it;
                if (obstacle->getHealth() <= 0) {
                    if (selectedObstacle == obstacle) {
                        selectedObstacle = nullptr;
                    }
                    for (TowerPit* pit : towerPits) {
                        if (pit->getObstacle() == obstacle) {
                            pit->setObstacle(nullptr);
                            break;
                        }
                    }
                    delete obstacle;
                    it = obstacles.erase(it);
                } else {
                    ++it;
                }
            }

            if (radish->getHealth() <= 0) {
                isGameOver = true;
                killTimer(waveTimerId);
                waveTimerId = -1;
            }
        }
        update();
    }
}
void mainwindow::mousePressEvent(QMouseEvent *event)
{
    if (isVictory) {
        QPoint clickPos = event->pos();
        if (confirmButtonRect.contains(clickPos)) {
            returnToStart();
            return;
        }
        return; // 防止穿透到底层游戏逻辑
    }
    if (isGameOver) {
            QPoint clickPos = event->pos();
            if (restartButtonRect.contains(clickPos)) {
                restartGame();
                return;
            }
            if (returnButtonRect.contains(clickPos)) {
                returnToStart();
                return;
            }
            return; // 防止穿透
        }

    QPoint clickPos = event->pos();

    // 全局按钮（暂停、二倍速）
    for (Button* button : buttons) {
        if (button->isClicked(clickPos)) {
            handleButtonClick(button->getType());
            update();
            return;
        }
    }
    // ========== 优先级最高：弹出的操作按钮 ==========

    // 升级/移除按钮
    if (selectedTower) {
        QPixmap upgradePixmap;
        if (money->canAfford(selectedTower->getUpgradeCost())) {
            upgradePixmap = QPixmap(":/images/images/upgrade_normal_blue.png").scaled(50, 50, Qt::KeepAspectRatio);
        }
        else {
            upgradePixmap = QPixmap(":/images/images/upgrade_normal_gray.png").scaled(50, 50, Qt::KeepAspectRatio);
        }
        // 与paintEvent绘制位置保持一致
        QPoint upgradePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() - upgradePixmap.height() - 20);
        QRect upgradeRect(upgradePos, upgradePixmap.size());

        QPixmap removePixmap = QPixmap(":/images/images/remove_normal.png").scaled(50, 50, Qt::KeepAspectRatio);
        QPoint removePos(selectedTower->getPosition().x(), selectedTower->getPosition().y() + 100);
        QRect removeRect(removePos, removePixmap.size());

        if (upgradeRect.contains(clickPos) && money->canAfford(selectedTower->getUpgradeCost())) {
            selectedTower->upgrade();
            money->spend(selectedTower->getUpgradeCost());
            selectedTower = nullptr;
            update();
            return;
        }

        if (removeRect.contains(clickPos)) {
            money->earn(selectedTower->getSellPrice());
            for (TowerPit* pit : towerPits) {
                if (pit->getRect().contains(selectedTower->getPosition())) {
                    pit->setHasTower(false);
                    break;
                }
            }
            auto it = std::find(towers.begin(), towers.end(), selectedTower);
            if (it != towers.end()) {
                delete *it;
                towers.erase(it);
            }
            selectedTower = nullptr;
            update();
            return;
        }
    }

    // 炮塔选项按钮
    if (showButtons) {
        int buttonSize = 100;
        int gap = 10;
        int totalWidth = 4 * buttonSize + 3 * gap;
        int totalHeight = buttonSize;

        // 与paintEvent绘制位置保持一致
        int bx = this->clickPos.x();
        int by = this->clickPos.y();
        if (bx + totalWidth > width()) bx = width() - totalWidth;
        if (bx < 0) bx = 0;
        if (by + totalHeight > height()) by = by - totalHeight;
        if (by < 0) by = 0;

        QRect coffeeButtonRect(bx, by, buttonSize, buttonSize);
        QRect musicButtonRect(bx + buttonSize + gap, by, buttonSize, buttonSize);
        QRect aiButtonRect(bx + 2 * (buttonSize + gap), by, buttonSize, buttonSize);
        QRect teacherButtonRect(bx + 3 * (buttonSize + gap), by, buttonSize, buttonSize);
        bool validButtonClicked = false;

        if (coffeeButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::COFFEE))) {
                towers.push_back(new Tower(Tower::COFFEE, this->clickPos));
                money->spend(Tower::getCost(Tower::COFFEE));
                validButtonClicked = true;
            }
        }
        else if (musicButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::MUSIC))) {
                towers.push_back(new Tower(Tower::MUSIC, this->clickPos));
                money->spend(Tower::getCost(Tower::MUSIC));
                validButtonClicked = true;
            }
        }
        else if (aiButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::AI))) {
                towers.push_back(new Tower(Tower::AI, this->clickPos));
                money->spend(Tower::getCost(Tower::AI));
                validButtonClicked = true;
            }
        }
        else if (teacherButtonRect.contains(clickPos)) {
            if (money->canAfford(Tower::getCost(Tower::TEACHER))) {
                towers.push_back(new Tower(Tower::TEACHER, this->clickPos));
                money->spend(Tower::getCost(Tower::TEACHER));
                validButtonClicked = true;
            }
        }
        // 无论是否成功放置，都隐藏选项按钮
        showButtons = false;
        update();

        if (validButtonClicked) {
            for (TowerPit* pit : towerPits) {
                if (pit->getPosition() == this->clickPos) {
                    pit->setHasTower(true);
                    break;
                }
            }
            return;
        }
    }

    // ========== 普通优先级：底层元素 ==========

    for (Obstacle* obstacle : obstacles) {
        QRect obstacleRect(obstacle->getPosition().toPoint(), obstacle->getPixmap().size());
        if (obstacleRect.contains(clickPos)) {
            if (selectedObstacle == obstacle) {
                selectedObstacle->setSelected(false);
                selectedObstacle = nullptr;
            } else {
                if (selectedObstacle) {
                    selectedObstacle->setSelected(false);
                }
                selectedObstacle = obstacle;
                selectedObstacle->setSelected(true);
            }
            update();
            return;
        }
    }

    for (TowerPit* pit : towerPits) {
        if (pit->getRect().contains(clickPos) && !pit->hasTower() && !pit->hasObstacle()) {
            this->clickPos = pit->getPosition();
            showButtons = true;
            update();
            return;
        }
    }

    for (Tower* tower : towers) {
        // 固定80x80尺寸检测，避免升级后贴图尺寸变化导致检测区域错误
        QRect towerRect(tower->getPosition(), QSize(80, 80));
        if (towerRect.contains(clickPos)) {
            if (selectedTower == tower) {
                selectedTower = nullptr;
            } else {
                selectedTower = tower;
            }
            update();
            return;
        }
    }

    // 点击空白位置，取消所有选中
    if (selectedTower || selectedObstacle || showButtons) {
        if (selectedObstacle) {
            selectedObstacle->setSelected(false);
        }
        selectedTower = nullptr;
        selectedObstacle = nullptr;
        showButtons = false;
        update();
    }
}

void mainwindow::generateMonster(int type)
{
    Enemy* enemy = new Enemy(static_cast<Enemy::Type>(type));
    int healthBonus = 0;
    switch (currentWave) {
    case 0: case 1: healthBonus = 0; break;
    case 2: healthBonus = 50; break;
    case 3: healthBonus = 100; break;
    case 4: healthBonus = 150; break;
    case 5: healthBonus = 180; break;
    case 6: healthBonus = 210; break;
    case 7: healthBonus = 250; break;
    case 8: healthBonus = 300; break;
    case 9: healthBonus = 300; break;
    case 10: healthBonus = 300; break;
    case 11: healthBonus = 300; break;
    }
    // 简单难度血量-40%（Boss除外）
    if (difficulty == 0 && type != Enemy::MonsterBoss) {
        healthBonus = healthBonus * 60 / 100;
    }
    // 困难难度血量+20%，11/12波额外+10%
    if (difficulty == 2) {
        healthBonus = healthBonus * 120 / 100;
        if (currentWave >= 10) {
            healthBonus = healthBonus * 110 / 100;
        }
    }
    if (healthBonus > 0) {
        enemy->addHealth(enemy->getHealth() * healthBonus / 100);
    }
    enemies.append(enemy);
}

bool mainwindow::isEnemyAtEnd(Enemy* enemy)
{
    QPointF endPoint = enemy->getPathPoints().last();
    return enemy->getPosition() == endPoint;
}

void mainwindow::checkEnemyCollideWithRadish()
{
    for (Enemy* enemy : enemies) {
        if (isEnemyAtEnd(enemy)) {
            switch (enemy->getType()) {
            case Enemy::Monster1: radish->takeDamage(0.5); break;
            case Enemy::Monster2:
            case Enemy::Monster3:
            case Enemy::Monster4:
            case Enemy::Monster5: radish->takeDamage(1); break;
            case Enemy::MonsterBoss: radish->takeDamage(10); break;
            }
        }
    }
}

void mainwindow::createTower(Tower::Type type)
{
    if (money->canAfford(Tower::getCost(type))) {
        towers.append(new Tower(type, clickPos));
        money->spend(Tower::getCost(type));
    }
}
void mainwindow::handleButtonClick(Button::ButtonType type)
{
    switch (type) {
    case Button::PAUSE:
        if (waveTimerId != -1) {
            killTimer(waveTimerId);
            waveTimerId = -1;
            isPaused = true;
        } else {
            waveTimerId = startTimer(gameSpeed == 1 ? 100 : 50);
            isPaused = false;
        }
        update();
        break;
    case Button::DOUBLE_SPEED:
        gameSpeed = (gameSpeed == 1) ? 2 : 1;
        Tower::setGameSpeed(gameSpeed);
        if (waveTimerId != -1) {
            killTimer(waveTimerId);
            waveTimerId = startTimer(gameSpeed == 1 ? 100 : 50);
        }
        update();
        break;
    case Button::CLICK_OBSTACLE: break;
    case Button::TOWER_UPGRADE: break;
    case Button::TOWER_REMOVE: break;
    case Button::RESTART: break;
    case Button::RETURN: break;
    }
}
void mainwindow::restartGame() {
    isGameOver = false;
    isVictory = false;
    currentWave = 0;
    elapsedTime = 0;
    gameSpeed = 1;
    Tower::setGameSpeed(1);
    radish->resetHealth();
    money->reset(450);

    for (Button* btn : buttons) {
        if (btn->getType() == Button::DOUBLE_SPEED) {
            btn->setActive(false);
            break;
        }
    }

    for (Enemy* enemy : enemies) delete enemy;
    enemies.clear();
    for (Tower* tower : towers) delete tower;
    towers.clear();
    for (Obstacle* obstacle : obstacles) delete obstacle;
    obstacles.clear();
    QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};
    QString obstaclePaths[10] = {
        ":/images/images/buildings_01.png",
        ":/images/images/obstacle2.png",
        ":/images/images/obstacle3.png",
        ":/images/images/obstacle4.png",
        ":/images/images/obstacle5.png",
        ":/images/images/obstacle6.png",
        ":/images/images/buildings_03.png",
        ":/images/images/bulidings_02.png",
        ":/images/images/obstacle9.png",
        ":/images/images/buildings_04.png"
    };
    QList<QSize> obstacleSizes = {
        QSize(180, 180), QSize(100, 100), QSize(100, 100), QSize(100, 100),
        QSize(100, 100), QSize(100, 100), QSize(180, 180), QSize(180, 180),
        QSize(100, 100), QSize(180, 180)
    };
    int pathIndex = 0;
    for (int index : targetIndices) {
        for (TowerPit* pit : towerPits) {
            if (pit->getIndex() == index) {
                QSize size = obstacleSizes[pathIndex];
                Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                obstacles.append(obstacle);
                pit->setObstacle(obstacle);
                pathIndex++;
                break;
            }
        }
    }
    for (TowerPit* pit : towerPits) {
        pit->setHasTower(false);
    }

    if (waveTimerId != -1) killTimer(waveTimerId);
    waveTimerId = startTimer(100);
    update();
}

void mainwindow::returnToStart()
{
    isGameOver = false;
    isVictory = false;
    currentWave = 0;
    elapsedTime = 0;
    gameSpeed = 1;
    Tower::setGameSpeed(1);

    for (Button* btn : buttons) {
        if (btn->getType() == Button::DOUBLE_SPEED) {
            btn->setActive(false);
            break;
        }
    }

    radish->resetHealth();
    money->reset(450);

    qDeleteAll(enemies);
    enemies.clear();
    qDeleteAll(towers);
    towers.clear();
    qDeleteAll(obstacles);
    obstacles.clear();
    {
        QList<int> targetIndices = {9, 15, 17,19,20,22,25, 33, 34,38};
        QString obstaclePaths[10] = {
            ":/images/images/buildings_01.png",
            ":/images/images/obstacle2.png",
            ":/images/images/obstacle3.png",
            ":/images/images/obstacle4.png",
            ":/images/images/obstacle5.png",
            ":/images/images/obstacle6.png",
            ":/images/images/buildings_03.png",
            ":/images/images/bulidings_02.png",
            ":/images/images/obstacle9.png",
            ":/images/images/buildings_04.png"
        };
        QList<QSize> obstacleSizes = {
            QSize(180, 180), QSize(100, 100), QSize(100, 100), QSize(100, 100),
            QSize(100, 100), QSize(100, 100), QSize(180, 180), QSize(180, 180),
            QSize(100, 100), QSize(180, 180)
        };
        int pathIndex = 0;
        for (int index : targetIndices) {
            for (TowerPit* pit : towerPits) {
                if (pit->getIndex() == index) {
                    QSize size = obstacleSizes[pathIndex];
                    Obstacle* obstacle = new Obstacle(pit->getPosition(), obstaclePaths[pathIndex], size.width(), size.height());
                    obstacles.append(obstacle);
                    pit->setObstacle(obstacle);
                    pathIndex++;
                    break;
                }
            }
        }
    }
    for (TowerPit* pit : towerPits) {
        pit->setHasTower(false);
    }

    selectedTower = nullptr;
    selectedObstacle = nullptr;
    showButtons = false;
    isPaused = false;
    difficulty = 1; // 重置为中等
    totalWaves = 10;

    if (waveTimerId != -1) {
        killTimer(waveTimerId);
        waveTimerId = -1;
    }
    countdownTimer->stop();

    // 不能调用this->hide()，否则Qt会连带隐藏子窗口startWindow
    startWindow->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    startWindow->show();
    startWindow->raise();
    difficultyWindow->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    difficultyWindow->hide();
    update();
}