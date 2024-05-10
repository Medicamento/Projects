#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "plot.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QStatusBar>
#include <QtDataVisualization>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    QCheckBox *gridCheckBox;
    QCheckBox *labelsCheckBox;
    QCheckBox *labelBordersCheckBox;

    void saveSettings();

    void loadSettings(QString settingsFilePath);

    int gradientForGraph[2] = {1, 1};
    int curGraph = 1;
    int stepCountx = 50;
    int stepCountz = 50;
    int curLang = 0;

    void showSincGraph1();

    void showSincGraph2();

    void createSincWidget();

    void createGradientWidget();

    QWidget *sincWidget;
    QWidget *selectionWidget;
    QWidget *GradientWidget;
    QWidget *stepWidget;
    QWidget *rangeWidget;
    QWidget *displayOptionsWidget;
    QStatusBar *statusBar;
    QTranslator *translator;

    void applyGradientToGraph(int num);

    QLinearGradient gradient1;
    QLinearGradient gradient2;
    QtDataVisualization::Q3DSurface *graph;

    QHBoxLayout *createSliderWithLineEdit(const QString &title, int minimum,
                                          int maximum, int value, int singleStep,
                                          char axis, bool left);

    Plot *plot;

    void createSelectionWidget();

    void createGradients();

    void createGraphs();

    void createStepWidget();

    void createRangeWidget();

    void updateGraphData();

    void createDisplayOptionsWidget();

    void changeLanguageToRussian();

    void changeLanguageToEnglish();

    void update();
};

#endif // MAINWINDOW_H
