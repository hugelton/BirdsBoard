#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QProgressBar>
#include <QComboBox>
#include <QLCDNumber>
#include <QFrame>

class TockusDSP;
class PT8211DAC;
class CoreAudioEngine;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onPitchCVChanged(int value);
    void onPitchKnobChanged(int value);
    void onCV1Changed(int value);
    void onCV2Changed(int value);
    void onGatePressed();
    void onGateReleased();
    void updateDisplay();
    void onAlgorithmChanged(int index);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    void setupUI();
    void setupControlsGroup();
    void setupDisplayGroup();
    void setupAudioGroup();
    void setupMenuBar();
    void connectSignals();
    void updateParameters();
    void updateLEDDisplay();
    
    // Core components
    TockusDSP* tockusDSP;
    PT8211DAC* pt8211DAC;
    CoreAudioEngine* coreAudioEngine;
    
    // UI Components
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    
    // Control Group
    QGroupBox* controlsGroup;
    QSlider* pitchCVSlider;
    QSlider* pitchKnobSlider;
    QSlider* cv1Slider;
    QSlider* cv2Slider;
    QPushButton* gateButton;
    
    // Labels for sliders
    QLabel* pitchCVLabel;
    QLabel* pitchKnobLabel;
    QLabel* cv1Label;
    QLabel* cv2Label;
    QLabel* gateLabel;
    
    // Value displays
    QLCDNumber* pitchCVDisplay;
    QLCDNumber* pitchKnobDisplay;
    QLCDNumber* cv1Display;
    QLCDNumber* cv2Display;
    
    // Display Group
    QGroupBox* displayGroup;
    QLabel* algorithmLabel;
    QComboBox* algorithmCombo;
    QLabel* frequencyLabel;
    QLCDNumber* frequencyDisplay;
    QLabel* ledLabel;
    QFrame* ledDisplay;
    QProgressBar* envelopeDisplay;
    
    // Audio Group
    QGroupBox* audioGroup;
    QLabel* dacTHDLabel;
    QLabel* dacSNRLabel;
    QLCDNumber* dacTHDDisplay;
    QLCDNumber* dacSNRDisplay;
    QPushButton* coreAudioButton;
    QPushButton* coreAudioTestButton;
    
    // Update timer
    QTimer* updateTimer;
    
    // State variables
    bool gateState;
    int currentPitchCV;
    int currentPitchKnob;
    int currentCV1;
    int currentCV2;
    
    // Algorithm names
    static const QStringList algorithmNames;
    
    // LED colors for each algorithm
    static const QColor ledColors[8];
};

#endif // MAINWINDOW_H