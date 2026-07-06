#include "difficultywindow.h"
#include <QPixmap>

DifficultyWindow::DifficultyWindow(QWidget *parent) : QWidget(parent)
{
    setFixedSize(1200, 800);

    // 半透明深色背景
    backgroundLabel = new QLabel(this);
    QPixmap bgPixmap(1200, 800);
    bgPixmap.fill(QColor(0, 0, 0, 180));
    backgroundLabel->setPixmap(bgPixmap);
    backgroundLabel->setGeometry(0, 0, 1200, 800);

    // 标题
    titleLabel = new QLabel("选择难度", this);
    QFont titleFont("Microsoft YaHei", 36, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(400, 180, 400, 60);

    // 简单难度按钮
    easyButton = new QPushButton("简  单", this);
    easyButton->setGeometry(450, 300, 300, 80);
    easyButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-size: 24px; "
        "font-family: 'Microsoft YaHei'; border: none; border-radius: 10px; }"
        "QPushButton:hover { background-color: #66BB6A; }"
    );
    connect(easyButton, &QPushButton::clicked, this, &DifficultyWindow::onEasyClicked);

    // 中等难度按钮
    mediumButton = new QPushButton("中  等", this);
    mediumButton->setGeometry(450, 410, 300, 80);
    mediumButton->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; font-size: 24px; "
        "font-family: 'Microsoft YaHei'; border: none; border-radius: 10px; }"
        "QPushButton:hover { background-color: #FFB74D; }"
    );
    connect(mediumButton, &QPushButton::clicked, this, &DifficultyWindow::onMediumClicked);

    // 困难难度按钮
    hardButton = new QPushButton("困  难", this);
    hardButton->setGeometry(450, 520, 300, 80);
    hardButton->setStyleSheet(
        "QPushButton { background-color: #F44336; color: white; font-size: 24px; "
        "font-family: 'Microsoft YaHei'; border: none; border-radius: 10px; }"
        "QPushButton:hover { background-color: #EF5350; }"
    );
    connect(hardButton, &QPushButton::clicked, this, &DifficultyWindow::onHardClicked);
}

DifficultyWindow::~DifficultyWindow()
{
}

void DifficultyWindow::onEasyClicked()
{
    emit difficultySelected(0);
}

void DifficultyWindow::onMediumClicked()
{
    emit difficultySelected(1);
}

void DifficultyWindow::onHardClicked()
{
    emit difficultySelected(2);
}
