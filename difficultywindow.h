#ifndef DIFFICULTYWINDOW_H
#define DIFFICULTYWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class DifficultyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DifficultyWindow(QWidget *parent = nullptr);
    ~DifficultyWindow();

signals:
    void difficultySelected(int difficulty); // 0=简单, 1=中等, 2=困难

private slots:
    void onEasyClicked();
    void onMediumClicked();
    void onHardClicked();

private:
    QPushButton *easyButton;
    QPushButton *mediumButton;
    QPushButton *hardButton;
    QLabel *titleLabel;
    QLabel *backgroundLabel;
};

#endif // DIFFICULTYWINDOW_H
