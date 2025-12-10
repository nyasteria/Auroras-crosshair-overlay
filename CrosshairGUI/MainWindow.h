#pragma once

#include <QMainWindow>
#include <QSlider>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QProcess>
#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onScaleChanged(int value);
    void onCrosshairSelected(int index);
    void onThicknessChanged(int value);
    void onStartOverlay();
    void onStopOverlay();
    void updateScaleLabel(int value);

private:
    void setupUI();
    void loadCrosshairs();
    void applySettings();
    void startOverlayProcess();
    void stopOverlayProcess();
    void sendSettingsToOverlay();

    // UI Components
    QSlider* m_scaleSlider;
    QSpinBox* m_thicknessSpinBox;
    QComboBox* m_crosshairCombo;
    QLabel* m_scaleLabel;
    QLabel* m_statusLabel;

    // Overlay process
    std::unique_ptr<QProcess> m_overlayProcess;
    bool m_overlayRunning;

    // Settings
    int m_currentScale;
    int m_currentThickness;
    QString m_currentCrosshair;
};
