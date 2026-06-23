#pragma once
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    QListWidget* sidebarList;
    QStackedWidget* pagesWidget;

    void setupUI();
    void applyStyles();
};