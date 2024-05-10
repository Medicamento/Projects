#include "mainwindow.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QShortcut>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtDataVisualization>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    graph = new QtDataVisualization::Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);
    container->setMinimumSize(800, 800);
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QTranslator *translator = new QTranslator(this);
    QMenu *fileMenu = menuBar->addMenu(tr("Settings"));

    QAction *saveAction = new QAction(tr("Save"), this);
    saveAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(saveAction);

    QAction *loadAction = new QAction(tr("Open"), this);
    loadAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(loadAction);

    connect(saveAction, &QAction::triggered, this, &MainWindow::saveSettings);
    connect(loadAction, &QAction::triggered, this,
            [this]() { loadSettings(""); });

    QToolBar *toolBar = new QToolBar(this);
    addToolBar(toolBar);
    toolBar->addAction(saveAction);
    toolBar->addAction(loadAction);

    connect(saveAction, &QAction::hovered, this, [this]() {
        statusBar->showMessage(tr("Want to save settings?"));
    });
    connect(loadAction, &QAction::hovered, this, [this]() {
        statusBar->showMessage(tr("Want to load settings?"));
    });
    connect(fileMenu, &QMenu::aboutToShow, this, [this]() {
        statusBar->showMessage(tr("Want to save or load file?"));
    });


    createGradients();
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage(tr("hello!"));
    plot = new Plot();
    QVBoxLayout *sideLayout = new QVBoxLayout();
    createSincWidget();
    createGradientWidget();
    createSelectionWidget();
    createDisplayOptionsWidget();
    createRangeWidget();
    createStepWidget();
    sideLayout->addWidget(sincWidget);
    sideLayout->addWidget(GradientWidget);
    sideLayout->addWidget(selectionWidget);
    sideLayout->addWidget(rangeWidget);
    sideLayout->addWidget(stepWidget);
    sideLayout->addWidget(displayOptionsWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(container);
    mainLayout->addLayout(sideLayout);
    showSincGraph1();
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    loadSettings(QCoreApplication::applicationDirPath() + "/settings.ini");
    QMenu *languageMenu = new QMenu(tr("Language"));

    QAction *englishAction = new QAction(tr("English"), this);
    connect(englishAction, &QAction::triggered, this, &MainWindow::changeLanguageToEnglish);

    QAction *russianAction = new QAction(tr("Русский"), this);
    connect(russianAction, &QAction::triggered, this, &MainWindow::changeLanguageToRussian);
    languageMenu->addAction(englishAction);
    languageMenu->addAction(russianAction);
    menuBar->addMenu(languageMenu);
}

MainWindow::~MainWindow() {
    const auto seriesList = graph->seriesList();
    for (auto series: seriesList) {
        graph->removeSeries(series);
        delete series;
    }
    delete graph;
    delete translator;
    delete statusBar;
    delete plot;
    delete sincWidget;
    delete GradientWidget;
    delete selectionWidget;
    delete rangeWidget;
    delete stepWidget;
    delete displayOptionsWidget;
}

void MainWindow::showSincGraph1() {
    curGraph = 1;
    const auto seriesList = graph->seriesList();
    for (auto series: seriesList) {
        graph->removeSeries(series);
        delete series;
    }

    QtDataVisualization::QSurfaceDataProxy *dataProxy =
            new QtDataVisualization::QSurfaceDataProxy();
    QtDataVisualization::QSurface3DSeries *series =
            new QtDataVisualization::QSurface3DSeries(dataProxy);

    dataProxy->resetArray(plot->curGraph1DataCopy());
    graph->addSeries(series);
    applyGradientToGraph(gradientForGraph[0]);
}

void MainWindow::showSincGraph2() {
    curGraph = 2;
    const auto seriesList = graph->seriesList();
    for (auto series: seriesList) {
        graph->removeSeries(series);
        delete series;
    }

    QtDataVisualization::QSurfaceDataProxy *dataProxy =
            new QtDataVisualization::QSurfaceDataProxy();
    QtDataVisualization::QSurface3DSeries *series =
            new QtDataVisualization::QSurface3DSeries(dataProxy);

    dataProxy->resetArray(plot->curGraph2DataCopy());
    graph->addSeries(series);
    applyGradientToGraph(gradientForGraph[1]);
}

void MainWindow::createSincWidget() {
    sincWidget = new QWidget(this);
    sincWidget->setMaximumWidth(150);
    sincWidget->setMaximumHeight(100);
    QRadioButton *sinc1 = new QRadioButton(tr("graph1"), sincWidget);
    QRadioButton *sinc2 = new QRadioButton(tr("graph2"), sincWidget);

    QGroupBox *groupBox = new QGroupBox(tr("Plot"), sincWidget);

    QVBoxLayout *groupLayout = new QVBoxLayout();
    groupLayout->addWidget(sinc1);
    groupLayout->addWidget(sinc2);
    groupBox->setLayout(groupLayout);

    QVBoxLayout *sincLayout = new QVBoxLayout();
    sincLayout->addWidget(groupBox);
    sincWidget->setLayout(sincLayout);


    connect(sinc1, &QRadioButton::clicked, this, &MainWindow::showSincGraph1);
    connect(sinc2, &QRadioButton::clicked, this, &MainWindow::showSincGraph2);
    sinc1->setChecked(true);
}

void MainWindow::createSelectionWidget() {
    selectionWidget = new QWidget(this);
    selectionWidget->setMaximumWidth(150);
    selectionWidget->setMaximumHeight(100);

    QRadioButton *selection1 = new QRadioButton(tr("On"), selectionWidget);
    QRadioButton *selection2 = new QRadioButton(tr("Off"), selectionWidget);

    QGroupBox *groupBox = new QGroupBox(tr("Point Selection"), selectionWidget);

    QVBoxLayout *groupLayout = new QVBoxLayout();
    groupLayout->addWidget(selection1);
    groupLayout->addWidget(selection2);
    groupBox->setLayout(groupLayout);

    QVBoxLayout *selectionLayout = new QVBoxLayout();
    selectionLayout->addWidget(groupBox);
    selectionWidget->setLayout(selectionLayout);


    connect(selection1, &QRadioButton::clicked, this, [this]() {
        graph->setSelectionMode(
                QtDataVisualization::QAbstract3DGraph::SelectionItem);
        QObject::connect(graph->seriesList().at(0),
                         &QtDataVisualization::QSurface3DSeries::itemLabelChanged,
                         this, [this](const QString &label) {
                    statusBar->showMessage(tr("Selected point: ") + label);
                });
    });

    connect(selection2, &QRadioButton::clicked, this, [this]() {
        graph->setSelectionMode(
                QtDataVisualization::QAbstract3DGraph::SelectionNone);
        statusBar->showMessage("");
    });
    selection2->setChecked(true);
}

void MainWindow::createGradientWidget() {
    GradientWidget = new QWidget(this);
    GradientWidget->setMaximumWidth(150);
    GradientWidget->setMaximumHeight(150);
    QPushButton *Gradient1 = new QPushButton(tr("Gradient1"), GradientWidget);
    QPushButton *Gradient2 = new QPushButton(tr("Gradient2"), GradientWidget);

    QGroupBox *groupBox = new QGroupBox(tr("Gradient"), GradientWidget);

    QVBoxLayout *groupLayout = new QVBoxLayout();
    groupLayout->addWidget(Gradient1);
    groupLayout->addWidget(Gradient2);
    groupBox->setLayout(groupLayout);
    QVBoxLayout *GradientLayout = new QVBoxLayout();
    GradientLayout->addWidget(groupBox);
    GradientWidget->setLayout(GradientLayout);
    Gradient1->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, "
                             "y2:0, stop:0 cyan, stop:0.5 green, stop:1 red);");
    Gradient2->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 red, "
            "stop:0.5 orange, stop:1 yellow);");

    connect(Gradient1, &QRadioButton::clicked, this,
            [this]() { applyGradientToGraph(1); });
    connect(Gradient2, &QRadioButton::clicked, this,
            [this]() { applyGradientToGraph(2); });
    Gradient1->setChecked(true);
}

void MainWindow::createGradients() {
    gradient1.setColorAt(0.0, QColor(Qt::cyan));
    gradient1.setColorAt(0.5, QColor(Qt::green));
    gradient1.setColorAt(1.0, QColor(Qt::red));

    gradient2.setColorAt(0.0, QColor(Qt::red));
    gradient2.setColorAt(0.5, QColor(QColorConstants::Svg::orange));
    gradient2.setColorAt(1.0, QColor(Qt::yellow));
}

void MainWindow::applyGradientToGraph(int num) {
    gradientForGraph[curGraph - 1] = num;
    QLinearGradient gradient;
    if (num == 1)
        gradient = gradient1;
    else
        gradient = gradient2;
    QtDataVisualization::Q3DTheme *theme = graph->activeTheme();
    theme->setBaseGradients(QList<QLinearGradient>() << gradient);
    theme->setColorStyle(QtDataVisualization::Q3DTheme::ColorStyleRangeGradient);

    graph->activeTheme()->setBaseGradients(theme->baseGradients());
    graph->activeTheme()->setColorStyle(theme->colorStyle());
}

void MainWindow::createRangeWidget() {

    rangeWidget = new QWidget(this);
    rangeWidget->setMaximumWidth(150);
    rangeWidget->setMaximumHeight(120);

    QVBoxLayout *layout = new QVBoxLayout(rangeWidget);

    QHBoxLayout *sliderLayout1 =
            createSliderWithLineEdit("X:", 0, 10, 10, 1, 'x', 0);
    QHBoxLayout *sliderLayout2 =
            createSliderWithLineEdit("X:", -10, 0, -10, 1, 'x', 1);
    QHBoxLayout *sliderLayout3 =
            createSliderWithLineEdit("Z:", 0, 10, 10, 1, 'z', 0);
    QHBoxLayout *sliderLayout4 =
            createSliderWithLineEdit("Z:", -10, 0, -10, 1, 'z', 1);

    layout->addLayout(sliderLayout1);
    layout->addLayout(sliderLayout2);
    layout->addLayout(sliderLayout3);
    layout->addLayout(sliderLayout4);

    rangeWidget->setLayout(layout);
}

QHBoxLayout *MainWindow::createSliderWithLineEdit(const QString &title,
                                                  int minimum, int maximum,
                                                  int value, int singleStep,
                                                  char axis, bool left) {
    QHBoxLayout *layout = new QHBoxLayout;

    QLabel *titleLabel = new QLabel(title);

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(minimum);
    slider->setMaximum(maximum);
    slider->setValue(value);
    slider->setSingleStep(singleStep);

    QLineEdit *valueLineEdit = new QLineEdit;
    valueLineEdit->setAlignment(Qt::AlignCenter);
    valueLineEdit->setText(QString::number(value));
    valueLineEdit->setMaximumWidth(25);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLineEdit);
    layout->addWidget(slider);
    connect(slider, &QSlider::valueChanged,
            [this, valueLineEdit, axis, left](int value) {
                valueLineEdit->setText(QString::number(value));
                if (axis == 'x') {
                    if (value > 0 || (value == 0 && left == 0)) {
                        graph->axisX()->setMax(value);
                    } else {
                        graph->axisX()->setMin(value);
                    }
                }
                if (axis == 'z') {
                    if (value > 0 || (value == 0 && left == 0)) {
                        graph->axisZ()->setMax(value);
                    } else {
                        graph->axisZ()->setMin(value);
                    }
                }
            });

    connect(valueLineEdit, &QLineEdit::textChanged,
            [slider](const QString &text) {
                bool ok;
                int value = text.toInt(&ok);
                if (ok) {
                    slider->setValue(value);
                }
            });

    return layout;
}

void MainWindow::createStepWidget() {
    stepWidget = new QWidget(this);
    stepWidget->setMaximumWidth(150);
    stepWidget->setMaximumHeight(180);

    QVBoxLayout *layout = new QVBoxLayout(stepWidget);

    QGroupBox *groupBox = new QGroupBox(tr("Steps:"));
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QLabel *slider1Label = new QLabel(tr("X:"));
    QSlider *slider1 = new QSlider(Qt::Horizontal);
    slider1->setMinimum(10);
    slider1->setMaximum(200);
    slider1->setValue(50);
    slider1->setSingleStep(1);

    QLineEdit *valueLineEdit1 = new QLineEdit;
    valueLineEdit1->setAlignment(Qt::AlignCenter);
    valueLineEdit1->setText(QString::number(slider1->value()));
    valueLineEdit1->setMaximumWidth(25);

    groupLayout->addWidget(slider1Label);
    groupLayout->addWidget(slider1);
    groupLayout->addWidget(valueLineEdit1);

    QLabel *slider2Label = new QLabel(tr("Z:"));
    QSlider *slider2 = new QSlider(Qt::Horizontal);
    slider2->setMinimum(10);
    slider2->setMaximum(200);
    slider2->setValue(50);
    slider2->setSingleStep(1);

    QLineEdit *valueLineEdit2 = new QLineEdit;
    valueLineEdit2->setAlignment(Qt::AlignCenter);
    valueLineEdit2->setText(QString::number(slider2->value()));
    valueLineEdit2->setMaximumWidth(25);

    groupLayout->addWidget(slider2Label);
    groupLayout->addWidget(slider2);
    groupLayout->addWidget(valueLineEdit2);

    groupBox->setLayout(groupLayout);

    layout->addWidget(groupBox);

    connect(slider1, &QSlider::valueChanged, [valueLineEdit1, this](int value) {
        valueLineEdit1->setText(QString::number(value));
        stepCountx = value;
        plot->changeData(stepCountx, stepCountz);
        if (curGraph == 1)
            showSincGraph1();
        else
            showSincGraph2();
    });

    connect(valueLineEdit1, &QLineEdit::textChanged,
            [slider1](const QString &text) {
                bool ok;
                int value = text.toInt(&ok);
                if (ok) {
                    slider1->setValue(value);
                }
            });

    connect(slider2, &QSlider::valueChanged, [valueLineEdit2, this](int value) {
        valueLineEdit2->setText(QString::number(value));
        stepCountz = value;
        plot->changeData(stepCountx, stepCountz);
        if (curGraph == 1)
            showSincGraph1();
        else
            showSincGraph2();
    });

    connect(valueLineEdit2, &QLineEdit::textChanged,
            [slider2](const QString &text) {
                bool ok;
                int value = text.toInt(&ok);
                if (ok) {
                    slider2->setValue(value);
                }
            });

    stepWidget->setLayout(layout);
}

void MainWindow::createDisplayOptionsWidget() {
    displayOptionsWidget = new QWidget(this);
    displayOptionsWidget->setMaximumWidth(100);
    displayOptionsWidget->setMaximumHeight(100);

    gridCheckBox = new QCheckBox(tr("grid"), displayOptionsWidget);
    labelsCheckBox = new QCheckBox(tr("label"), displayOptionsWidget);
    labelBordersCheckBox = new QCheckBox(tr("boardes"), displayOptionsWidget);

    QVBoxLayout *layout = new QVBoxLayout(displayOptionsWidget);
    layout->addWidget(gridCheckBox);
    layout->addWidget(labelsCheckBox);
    layout->addWidget(labelBordersCheckBox);
    displayOptionsWidget->setLayout(layout);

    connect(gridCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        graph->activeTheme()->setGridEnabled(state == Qt::Checked);
    });

    connect(labelsCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        graph->activeTheme()->setLabelBackgroundEnabled(state == Qt::Checked);
        if (state != Qt::Checked) {
            graph->axisX()->setLabelFormat(" ");
            graph->axisY()->setLabelFormat(" ");
            graph->axisZ()->setLabelFormat(" ");
        } else {
            graph->axisX()->setLabelFormat("%.1f");
            graph->axisY()->setLabelFormat("%.1f");
            graph->axisZ()->setLabelFormat("%.1f");
        }
        labelBordersCheckBox->setEnabled(state == Qt::Checked);
    });

    connect(labelBordersCheckBox, &QCheckBox::stateChanged, this,
            [this](int state) {
                graph->activeTheme()->setLabelBorderEnabled(state == Qt::Checked);
            });
}

void MainWindow::saveSettings() {
    QString folderPath = QFileDialog::getExistingDirectory(
            nullptr, tr("Choose folder"), "", QFileDialog::ShowDirsOnly);

    if (!folderPath.isEmpty()) {
        QString filePath = folderPath + "/settings.ini";
        QSettings settings(filePath, QSettings::IniFormat);
        settings.setValue("Graph", curGraph);
        settings.setValue("Selection",
                          (graph->selectionMode() ==
                           QtDataVisualization::QAbstract3DGraph::SelectionNone)
                          ? 0
                          : 1);
        settings.setValue("Gradient", gradientForGraph[curGraph - 1]);
        settings.setValue("GradientForOther", gradientForGraph[2 - curGraph]);
        settings.setValue("RangeXMin", graph->axisX()->min());
        settings.setValue("RangeXMax", graph->axisX()->max());
        settings.setValue("RangeZMin", graph->axisZ()->min());
        settings.setValue("RangeZMax", graph->axisZ()->max());
        settings.setValue("StepCountX", stepCountx);
        settings.setValue("StepCountZ", stepCountz);
        settings.setValue("grid", gridCheckBox->checkState());
        settings.setValue("lables", labelsCheckBox->checkState());
        settings.setValue("boarsed", labelBordersCheckBox->checkState());
    }
}

void MainWindow::loadSettings(QString settingsFilePath) {
    QString fileName = settingsFilePath;
    if (!QFile::exists(fileName)) {

        fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Choose file"), "",
                                                "INI Files (*.ini)");
    }
    if (!fileName.isEmpty()) {
        QSettings settings(fileName, QSettings::IniFormat);
        curGraph = settings.value("Graph", 1).toInt();
        graph->setSelectionMode(
                static_cast<QtDataVisualization::QAbstract3DGraph::SelectionFlag>(
                        settings
                                .value("Selection",
                                       QtDataVisualization::QAbstract3DGraph::SelectionNone)
                                .toInt()));
        gradientForGraph[curGraph - 1] = settings.value("Gradient", 1).toInt();
        graph->axisX()->setMin(settings.value("RangeXMin", -10).toDouble());
        graph->axisX()->setMax(settings.value("RangeXMax", 10).toDouble());
        graph->axisZ()->setMin(settings.value("RangeZMin", -10).toDouble());
        graph->axisZ()->setMax(settings.value("RangeZMax", 10).toDouble());
        stepCountx = settings.value("StepCountX", 50).toInt();
        stepCountz = settings.value("StepCountZ", 50).toInt();
        Qt::CheckState gridState = static_cast<Qt::CheckState>(
                settings.value("grid", Qt::Checked).toInt());
        Qt::CheckState labelsState = static_cast<Qt::CheckState>(
                settings.value("labels", Qt::Checked).toInt());
        Qt::CheckState bordersState = static_cast<Qt::CheckState>(
                settings.value("borders", Qt::Checked).toInt());

        gridCheckBox->setChecked(gridState);
        labelsCheckBox->setChecked(labelsState);
        labelBordersCheckBox->setChecked(bordersState);
        plot->changeData(stepCountx, stepCountz);
        if (curGraph == 1)
            showSincGraph1();
        else
            showSincGraph2();
        applyGradientToGraph(gradientForGraph[curGraph - 1]);
    } else {
        curGraph = 1;
        graph->setSelectionMode(
                QtDataVisualization::QAbstract3DGraph::SelectionNone);
        gradientForGraph[curGraph - 1] = 1;
        graph->axisX()->setMin(-10);
        graph->axisX()->setMax(10);
        graph->axisZ()->setMin(-10);
        graph->axisZ()->setMax(10);
        stepCountx = 50;
        stepCountz = 50;
        plot->changeData(stepCountx, stepCountz);
        gridCheckBox->setChecked(true);
        labelsCheckBox->setChecked(true);
        labelBordersCheckBox->setChecked(true);
        showSincGraph1();
        applyGradientToGraph(gradientForGraph[curGraph - 1]);
    }
}

void MainWindow::changeLanguageToEnglish() {

}

void MainWindow::changeLanguageToRussian() {

}


