#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QFile>
#include <QTextStream>
#include <windows.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_overlayRunning(false)
    , m_currentScale(100)
    , m_currentThickness(3)
    , m_currentCrosshair("osupink.png")
{
    setWindowTitle("Aurora Crosshair Controller");
    setGeometry(100, 100, 500, 400);

    setupUI();
    loadCrosshairs();

    // Try to connect to existing overlay process
    HWND hWnd = FindWindowW(L"AuroraCrosshair", nullptr);
    if (hWnd)
    {
        m_overlayRunning = true;
        m_statusLabel->setText("Overlay: Running");
        m_statusLabel->setStyleSheet("color: green;");
    }
    else
    {
        m_statusLabel->setText("Overlay: Not running");
        m_statusLabel->setStyleSheet("color: red;");
    }
}

MainWindow::~MainWindow()
{
    stopOverlayProcess();
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Status label
    m_statusLabel = new QLabel("Overlay: Not running");
    mainLayout->addWidget(m_statusLabel);

    // Crosshair Selection Group
    QGroupBox* crosshairGroup = new QGroupBox("Crosshair Selection", this);
    QVBoxLayout* crosshairLayout = new QVBoxLayout();

    QLabel* crosshairLabel = new QLabel("Select Crosshair:");
    m_crosshairCombo = new QComboBox();
    connect(m_crosshairCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCrosshairSelected);

    crosshairLayout->addWidget(crosshairLabel);
    crosshairLayout->addWidget(m_crosshairCombo);
    crosshairGroup->setLayout(crosshairLayout);
    mainLayout->addWidget(crosshairGroup);

    // Scale Control Group
    QGroupBox* scaleGroup = new QGroupBox("Crosshair Scale", this);
    QVBoxLayout* scaleLayout = new QVBoxLayout();

    QHBoxLayout* scaleSliderLayout = new QHBoxLayout();
    QLabel* scaleMinLabel = new QLabel("50%");
    m_scaleSlider = new QSlider(Qt::Horizontal);
    m_scaleSlider->setMinimum(50);
    m_scaleSlider->setMaximum(300);
    m_scaleSlider->setValue(100);
    m_scaleSlider->setTickPosition(QSlider::TicksBelow);
    m_scaleSlider->setTickInterval(50);
    QLabel* scaleMaxLabel = new QLabel("300%");

    connect(m_scaleSlider, &QSlider::valueChanged, this, &MainWindow::onScaleChanged);
    connect(m_scaleSlider, &QSlider::valueChanged, this, &MainWindow::updateScaleLabel);

    scaleSliderLayout->addWidget(scaleMinLabel);
    scaleSliderLayout->addWidget(m_scaleSlider);
    scaleSliderLayout->addWidget(scaleMaxLabel);

    m_scaleLabel = new QLabel("Scale: 100%");
    m_scaleLabel->setAlignment(Qt::AlignCenter);
    m_scaleLabel->setStyleSheet("font-weight: bold;");

    scaleLayout->addLayout(scaleSliderLayout);
    scaleLayout->addWidget(m_scaleLabel);
    scaleGroup->setLayout(scaleLayout);
    mainLayout->addWidget(scaleGroup);

    // Thickness Control Group
    QGroupBox* thicknessGroup = new QGroupBox("Crosshair Thickness (Fallback)", this);
    QVBoxLayout* thicknessLayout = new QVBoxLayout();

    QHBoxLayout* thicknessControlLayout = new QHBoxLayout();
    QLabel* thicknessLabel = new QLabel("Thickness (pixels):");
    m_thicknessSpinBox = new QSpinBox();
    m_thicknessSpinBox->setMinimum(1);
    m_thicknessSpinBox->setMaximum(10);
    m_thicknessSpinBox->setValue(3);

    connect(m_thicknessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onThicknessChanged);

    thicknessControlLayout->addWidget(thicknessLabel);
    thicknessControlLayout->addWidget(m_thicknessSpinBox);
    thicknessControlLayout->addStretch();

    thicknessLayout->addLayout(thicknessControlLayout);
    thicknessGroup->setLayout(thicknessLayout);
    mainLayout->addWidget(thicknessGroup);

    // Button Group
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* startButton = new QPushButton("Start Overlay");
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartOverlay);

    QPushButton* stopButton = new QPushButton("Stop Overlay");
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopOverlay);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch();
    centralWidget->setLayout(mainLayout);
}

void MainWindow::loadCrosshairs()
{
    // Find crosshairs in resources folder
    QString resourcePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    resourcePath += "/Crosshair overlay/AuroraCrosshairDX/resources/crosshairs";

    QDir crosshairDir(resourcePath);
    if (!crosshairDir.exists())
    {
        m_statusLabel->setText("Error: Crosshairs folder not found!");
        return;
    }

    QStringList filters;
    filters << "*.png";
    QStringList crosshairs = crosshairDir.entryList(filters, QDir::Files);

    if (crosshairs.isEmpty())
    {
        m_statusLabel->setText("Warning: No PNG crosshairs found");
        m_crosshairCombo->addItem("No crosshairs found");
        return;
    }

    for (const QString& crosshair : crosshairs)
    {
        m_crosshairCombo->addItem(crosshair);
    }

    // Set default to osupink if it exists
    int index = m_crosshairCombo->findText("osupink.png");
    if (index >= 0)
    {
        m_crosshairCombo->setCurrentIndex(index);
    }
}

void MainWindow::onScaleChanged(int value)
{
    m_currentScale = value;
    sendSettingsToOverlay();
}

void MainWindow::updateScaleLabel(int value)
{
    m_scaleLabel->setText(QString("Scale: %1%").arg(value));
}

void MainWindow::onCrosshairSelected(int index)
{
    if (index >= 0)
    {
        m_currentCrosshair = m_crosshairCombo->itemText(index);
        sendSettingsToOverlay();
    }
}

void MainWindow::onThicknessChanged(int value)
{
    m_currentThickness = value;
    sendSettingsToOverlay();
}

void MainWindow::onStartOverlay()
{
    startOverlayProcess();
}

void MainWindow::onStopOverlay()
{
    stopOverlayProcess();
}

void MainWindow::startOverlayProcess()
{
    if (m_overlayRunning)
    {
        m_statusLabel->setText("Overlay: Already running");
        return;
    }

    // Find the overlay executable
    QString overlayExePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    overlayExePath += "/Crosshair overlay/AuroraCrosshairDX/x64/Release/AuroraCrosshairDX.exe";

    QFileInfo exeFile(overlayExePath);
    if (!exeFile.exists())
    {
        m_statusLabel->setText("Error: Overlay executable not found!");
        return;
    }

    m_overlayProcess = std::make_unique<QProcess>(this);
    m_overlayProcess->start(overlayExePath);

    if (m_overlayProcess->waitForStarted(3000))
    {
        m_overlayRunning = true;
        m_statusLabel->setText("Overlay: Running");
        m_statusLabel->setStyleSheet("color: green;");
    }
    else
    {
        m_statusLabel->setText("Error: Failed to start overlay");
        m_statusLabel->setStyleSheet("color: red;");
    }
}

void MainWindow::stopOverlayProcess()
{
    if (!m_overlayRunning)
    {
        m_statusLabel->setText("Overlay: Not running");
        return;
    }

    // Find and kill the overlay window
    HWND hWnd = FindWindowW(L"AuroraCrosshair", nullptr);
    if (hWnd)
    {
        PostMessageW(hWnd, WM_QUIT, 0, 0);
    }

    if (m_overlayProcess)
    {
        m_overlayProcess->waitForFinished(2000);
        if (m_overlayProcess->state() == QProcess::Running)
        {
            m_overlayProcess->kill();
        }
    }

    m_overlayRunning = false;
    m_statusLabel->setText("Overlay: Stopped");
    m_statusLabel->setStyleSheet("color: red;");
}

void MainWindow::sendSettingsToOverlay()
{
    // Write configuration to file that overlay reads
    QString docPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString configPath = docPath + "/Crosshair overlay/overlay_config.txt";

    QFile configFile(configPath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&configFile);
        out << "scale=" << (m_currentScale / 100.0) << "\n";
        out << "crosshair=" << m_currentCrosshair << "\n";
        configFile.close();
    }
}
