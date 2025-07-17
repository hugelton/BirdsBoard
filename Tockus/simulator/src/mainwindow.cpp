#include "mainwindow.h"
#include "tockus_dsp.h"
#include "pt8211_dac.h"
#include "coreaudio_engine.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>

// Algorithm names for the combo box
const QStringList MainWindow::algorithmNames = {
    "BASS (808 Bass)",
    "SNARE (808 Snare)", 
    "HIHAT (808 Hi-hat)",
    "KARPLUS (Karplus-Strong)",
    "MODAL (Modal Synthesis)",
    "ZAP (ZAP Sound)",
    "CLAP (808 Clap)",
    "COWBELL (Cowbell)"
};

// LED colors for each algorithm (matching Arduino)
const QColor MainWindow::ledColors[8] = {
    QColor(255, 0, 0),     // BASS - Red
    QColor(0, 255, 0),     // SNARE - Green
    QColor(0, 0, 255),     // HIHAT - Blue
    QColor(255, 255, 0),   // KARPLUS - Yellow
    QColor(255, 0, 255),   // MODAL - Magenta
    QColor(0, 255, 255),   // ZAP - Cyan
    QColor(255, 165, 0),   // CLAP - Orange
    QColor(255, 255, 255)  // COWBELL - White
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tockusDSP(nullptr)
    , pt8211DAC(nullptr)
    , coreAudioEngine(nullptr)
    , centralWidget(nullptr)
    , mainLayout(nullptr)
    , gateState(false)
    , currentPitchCV(2048)
    , currentPitchKnob(2000)
    , currentCV1(1000)
    , currentCV2(1000)
{
    // Create core components
    tockusDSP = new TockusDSP();
    pt8211DAC = new PT8211DAC();
    coreAudioEngine = new CoreAudioEngine(this);
    
    // Initialize audio engine
    coreAudioEngine->initialize(tockusDSP, pt8211DAC);
    
    // Setup UI
    setupUI();
    connectSignals();
    
    // Setup update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateDisplay);
    updateTimer->start(33); // 30 FPS update rate for more responsive CV control
    
    // Set window properties
    setWindowTitle("Tockus Simulator v1.0");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // Initialize display
    updateParameters();
    updateDisplay();
}

MainWindow::~MainWindow() {
    if (coreAudioEngine) {
        coreAudioEngine->stopAudio();
    }
    
    delete tockusDSP;
    delete pt8211DAC;
    // coreAudioEngine is QObject child, will be deleted automatically
}

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    setupControlsGroup();
    setupDisplayGroup();
    setupAudioGroup();
    setupMenuBar();
}

void MainWindow::setupControlsGroup() {
    controlsGroup = new QGroupBox("Control Voltage Inputs", this);
    QGridLayout* controlsLayout = new QGridLayout(controlsGroup);
    
    // PITCH CV slider
    pitchCVLabel = new QLabel("PITCH CV:", this);
    pitchCVSlider = new QSlider(Qt::Horizontal, this);
    pitchCVSlider->setRange(PITCH_CV_MIN, PITCH_CV_MAX);
    pitchCVSlider->setValue(currentPitchCV);
    pitchCVSlider->setTickPosition(QSlider::TicksBelow);
    pitchCVSlider->setTickInterval(500);
    pitchCVSlider->setMinimumHeight(40);
    pitchCVSlider->setStyleSheet("QSlider::groove:horizontal { height: 12px; background: #444; border-radius: 6px; } QSlider::handle:horizontal { background: #FFF; border: 2px solid #CCC; width: 20px; margin: -4px 0; border-radius: 10px; }");
    pitchCVDisplay = new QLCDNumber(4, this);
    pitchCVDisplay->display(currentPitchCV);
    
    controlsLayout->addWidget(pitchCVLabel, 0, 0);
    controlsLayout->addWidget(pitchCVSlider, 0, 1);
    controlsLayout->addWidget(pitchCVDisplay, 0, 2);
    
    // PITCH KNOB slider
    pitchKnobLabel = new QLabel("PITCH KNOB:", this);
    pitchKnobSlider = new QSlider(Qt::Horizontal, this);
    pitchKnobSlider->setRange(PITCH_KNOB_MIN, PITCH_KNOB_MAX);
    pitchKnobSlider->setValue(currentPitchKnob);
    pitchKnobSlider->setTickPosition(QSlider::TicksBelow);
    pitchKnobSlider->setTickInterval(500);
    pitchKnobSlider->setMinimumHeight(40);
    pitchKnobSlider->setStyleSheet("QSlider::groove:horizontal { height: 12px; background: #444; border-radius: 6px; } QSlider::handle:horizontal { background: #FFF; border: 2px solid #CCC; width: 20px; margin: -4px 0; border-radius: 10px; }");
    pitchKnobDisplay = new QLCDNumber(4, this);
    pitchKnobDisplay->display(currentPitchKnob);
    
    controlsLayout->addWidget(pitchKnobLabel, 1, 0);
    controlsLayout->addWidget(pitchKnobSlider, 1, 1);
    controlsLayout->addWidget(pitchKnobDisplay, 1, 2);
    
    // CV1 slider (Algorithm selection)
    cv1Label = new QLabel("CV1 (Algorithm):", this);
    cv1Slider = new QSlider(Qt::Horizontal, this);
    cv1Slider->setRange(CV1_MIN, CV1_MAX);
    cv1Slider->setValue(currentCV1);
    cv1Slider->setTickPosition(QSlider::TicksBelow);
    cv1Slider->setTickInterval(250);
    cv1Slider->setMinimumHeight(40);
    cv1Slider->setStyleSheet("QSlider::groove:horizontal { height: 12px; background: #444; border-radius: 6px; } QSlider::handle:horizontal { background: #FFF; border: 2px solid #CCC; width: 20px; margin: -4px 0; border-radius: 10px; }");
    cv1Display = new QLCDNumber(4, this);
    cv1Display->display(currentCV1);
    
    controlsLayout->addWidget(cv1Label, 2, 0);
    controlsLayout->addWidget(cv1Slider, 2, 1);
    controlsLayout->addWidget(cv1Display, 2, 2);
    
    // CV2 slider (Algorithm parameter)
    cv2Label = new QLabel("CV2 (Parameter):", this);
    cv2Slider = new QSlider(Qt::Horizontal, this);
    cv2Slider->setRange(CV2_MIN, CV2_MAX);
    cv2Slider->setValue(currentCV2);
    cv2Slider->setTickPosition(QSlider::TicksBelow);
    cv2Slider->setTickInterval(250);
    cv2Slider->setMinimumHeight(40);
    cv2Slider->setStyleSheet("QSlider::groove:horizontal { height: 12px; background: #444; border-radius: 6px; } QSlider::handle:horizontal { background: #FFF; border: 2px solid #CCC; width: 20px; margin: -4px 0; border-radius: 10px; }");
    cv2Display = new QLCDNumber(4, this);
    cv2Display->display(currentCV2);
    
    controlsLayout->addWidget(cv2Label, 3, 0);
    controlsLayout->addWidget(cv2Slider, 3, 1);
    controlsLayout->addWidget(cv2Display, 3, 2);
    
    // Gate button
    gateLabel = new QLabel("GATE:", this);
    gateButton = new QPushButton("TRIGGER", this);
    gateButton->setObjectName("gateButton");
    gateButton->setCheckable(false);
    gateButton->setMinimumHeight(60);
    gateButton->setStyleSheet("font-weight: bold; font-size: 16px; background-color: #4CAF50; color: white; border-radius: 8px;");
    
    controlsLayout->addWidget(gateLabel, 4, 0);
    controlsLayout->addWidget(gateButton, 4, 1, 1, 2);
    
    // Set column stretch
    controlsLayout->setColumnStretch(1, 1);  // Sliders stretch
    controlsLayout->setColumnStretch(2, 0);  // Displays fixed width
    
    mainLayout->addWidget(controlsGroup);
}

void MainWindow::setupDisplayGroup() {
    displayGroup = new QGroupBox("Status Display", this);
    QGridLayout* displayLayout = new QGridLayout(displayGroup);
    
    // Algorithm display
    algorithmLabel = new QLabel("Algorithm:", this);
    algorithmCombo = new QComboBox(this);
    algorithmCombo->addItems(algorithmNames);
    algorithmCombo->setCurrentIndex(0);
    
    displayLayout->addWidget(algorithmLabel, 0, 0);
    displayLayout->addWidget(algorithmCombo, 0, 1);
    
    // Frequency display
    frequencyLabel = new QLabel("Frequency (Hz):", this);
    frequencyDisplay = new QLCDNumber(6, this);
    frequencyDisplay->display(60.0);
    
    displayLayout->addWidget(frequencyLabel, 1, 0);
    displayLayout->addWidget(frequencyDisplay, 1, 1);
    
    // LED display
    ledLabel = new QLabel("Algorithm LED:", this);
    ledDisplay = new QFrame(this);
    ledDisplay->setFrameStyle(QFrame::Box | QFrame::Raised);
    ledDisplay->setMinimumSize(40, 40);
    ledDisplay->setMaximumSize(40, 40);
    ledDisplay->setStyleSheet("background-color: red; border: 2px solid #333; border-radius: 20px;");
    
    displayLayout->addWidget(ledLabel, 2, 0);
    displayLayout->addWidget(ledDisplay, 2, 1);
    
    // Envelope display
    QLabel* envelopeLabel = new QLabel("Envelope:", this);
    envelopeDisplay = new QProgressBar(this);
    envelopeDisplay->setRange(0, 100);
    envelopeDisplay->setValue(0);
    envelopeDisplay->setTextVisible(true);
    envelopeDisplay->setFormat("%p%");
    
    displayLayout->addWidget(envelopeLabel, 3, 0);
    displayLayout->addWidget(envelopeDisplay, 3, 1);
    
    mainLayout->addWidget(displayGroup);
}

void MainWindow::setupAudioGroup() {
    audioGroup = new QGroupBox("Audio & DAC Status", this);
    QVBoxLayout* audioLayout = new QVBoxLayout(audioGroup);
    
    // Create a simple horizontal layout for CoreAudio buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // CoreAudio buttons only
    coreAudioButton = new QPushButton("Start CoreAudio", this);
    coreAudioButton->setMinimumHeight(50);
    coreAudioButton->setMinimumWidth(150);
    coreAudioButton->setStyleSheet("font-weight: bold; font-size: 14px; background-color: #000066; color: white;");
    coreAudioButton->setEnabled(true);  // Explicitly enable
    
    coreAudioTestButton = new QPushButton("CoreAudio Test 440Hz", this);
    coreAudioTestButton->setMinimumHeight(50);
    coreAudioTestButton->setMinimumWidth(150);
    coreAudioTestButton->setStyleSheet("font-weight: bold; font-size: 14px; background-color: #006600; color: white;");
    coreAudioTestButton->setEnabled(true);  // Explicitly enable
    
    buttonLayout->addWidget(coreAudioButton);
    buttonLayout->addWidget(coreAudioTestButton);
    
    audioLayout->addLayout(buttonLayout);
    
    // DAC displays
    QHBoxLayout* dacLayout = new QHBoxLayout();
    
    dacTHDLabel = new QLabel("DAC THD (%):", this);
    dacTHDDisplay = new QLCDNumber(5, this);
    dacTHDDisplay->display(0.08);
    
    dacSNRLabel = new QLabel("DAC SNR (dB):", this);
    dacSNRDisplay = new QLCDNumber(5, this);
    dacSNRDisplay->display(91.0);
    
    dacLayout->addWidget(dacTHDLabel);
    dacLayout->addWidget(dacTHDDisplay);
    dacLayout->addWidget(dacSNRLabel);
    dacLayout->addWidget(dacSNRDisplay);
    
    audioLayout->addLayout(dacLayout);
    
    mainLayout->addWidget(audioGroup);
    
    // Add helpful text
    QLabel* instructionLabel = new QLabel("使い方: 1) Start CoreAudio ボタンを押す  2) TRIGGER ボタンまたはスペースキーでドラムを鳴らす  3) CV1でアルゴリズムを変更", this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("color: #666; font-size: 11px; margin: 5px;");
    audioLayout->addWidget(instructionLabel);
    
    // Add stretch to push everything to the top
    mainLayout->addStretch();
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    
    QAction* exitAction = fileMenu->addAction("&Exit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Audio menu for testing
    QMenu* audioMenu = menuBar->addMenu("&Audio");
    
    QAction* coreAudioTestAction = audioMenu->addAction("CoreAudio Test (440Hz)");
    connect(coreAudioTestAction, &QAction::triggered, [this]() {
        if (coreAudioEngine) {
            coreAudioEngine->startTestTone();
            statusBar()->showMessage("CoreAudio test tone started - You should hear 440Hz");
        }
    });
    
    QAction* stopCoreAudioAction = audioMenu->addAction("Stop CoreAudio Test");
    connect(stopCoreAudioAction, &QAction::triggered, [this]() {
        if (coreAudioEngine) {
            coreAudioEngine->stopTestTone();
            statusBar()->showMessage("CoreAudio test stopped");
        }
    });
    
    audioMenu->addSeparator();
    
    QAction* startCoreAudioAction = audioMenu->addAction("Start CoreAudio Tockus");
    connect(startCoreAudioAction, &QAction::triggered, [this]() {
        if (coreAudioEngine) {
            coreAudioEngine->startAudio();
        }
    });
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Tockus Simulator",
            "Tockus Simulator v1.0\n\n"
            "A desktop simulator for the Tockus drum synthesizer.\n"
            "Features 8 authentic drum algorithms with real-time\n"
            "parameter control and PT8211 DAC simulation.\n\n"
            "BirdsBoards Project\n"
            "Built with Qt6 and C++");
    });
    
    // Status bar
    statusBar()->showMessage("Ready - Click 'Start Audio' to begin");
}

void MainWindow::connectSignals() {
    // Slider connections
    connect(pitchCVSlider, &QSlider::valueChanged, this, &MainWindow::onPitchCVChanged);
    connect(pitchKnobSlider, &QSlider::valueChanged, this, &MainWindow::onPitchKnobChanged);
    connect(cv1Slider, &QSlider::valueChanged, this, &MainWindow::onCV1Changed);
    connect(cv2Slider, &QSlider::valueChanged, this, &MainWindow::onCV2Changed);
    
    // Gate button connections
    connect(gateButton, &QPushButton::pressed, this, &MainWindow::onGatePressed);
    connect(gateButton, &QPushButton::released, this, &MainWindow::onGateReleased);
    
    // Algorithm combo box
    connect(algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    
    
    // CoreAudio engine connections
    connect(coreAudioButton, &QPushButton::clicked, [this]() {
        if (coreAudioEngine->isAudioActive()) {
            coreAudioEngine->stopAudio();
        } else {
            coreAudioEngine->startAudio();
        }
    });
    
    connect(coreAudioTestButton, &QPushButton::clicked, [this]() {
        if (coreAudioEngine) {
            coreAudioEngine->startTestTone();
        }
    });
    
    connect(coreAudioEngine, &CoreAudioEngine::audioStarted, [this]() {
        coreAudioButton->setText("Stop CoreAudio");
        coreAudioButton->setStyleSheet("font-weight: bold; font-size: 12px; background-color: #CC0000;");
        statusBar()->showMessage("CoreAudio active");
    });
    
    connect(coreAudioEngine, &CoreAudioEngine::audioStopped, [this]() {
        coreAudioButton->setText("CoreAudio");
        coreAudioButton->setStyleSheet("font-weight: bold; font-size: 12px; background-color: #000066;");
        statusBar()->showMessage("CoreAudio stopped");
    });
    
    connect(coreAudioEngine, &CoreAudioEngine::audioError, [this](const QString& error) {
        QMessageBox::critical(this, "CoreAudio Error", error);
        statusBar()->showMessage("CoreAudio error: " + error);
    });
}

void MainWindow::onPitchCVChanged(int value) {
    currentPitchCV = value;
    pitchCVDisplay->display(value);
    updateParameters();
}

void MainWindow::onPitchKnobChanged(int value) {
    currentPitchKnob = value;
    pitchKnobDisplay->display(value);
    updateParameters();
}

void MainWindow::onCV1Changed(int value) {
    currentCV1 = value;
    cv1Display->display(value);
    updateParameters();
}

void MainWindow::onCV2Changed(int value) {
    currentCV2 = value;
    cv2Display->display(value);
    updateParameters();
}

void MainWindow::onGatePressed() {
    gateState = true;
    gateButton->setStyleSheet("background-color: #FF4444; font-weight: bold; font-size: 16px; color: white; border-radius: 8px;");
    updateParameters();
}

void MainWindow::onGateReleased() {
    gateState = false;
    gateButton->setStyleSheet("font-weight: bold; font-size: 16px; background-color: #4CAF50; color: white; border-radius: 8px;");
    updateParameters();
}

void MainWindow::onAlgorithmChanged(int index) {
    // Update CV1 slider to match algorithm selection
    if (index >= 0 && index < NUM_ALGORITHMS) {
        float algorithmValue = (float)index / (NUM_ALGORITHMS - 1);
        int cv1Value = CV1_MIN + (int)(algorithmValue * (CV1_MAX - CV1_MIN));
        
        cv1Slider->setValue(cv1Value);
        // onCV1Changed will be called automatically
    }
}

void MainWindow::updateParameters() {
    if (!tockusDSP) return;
    
    // Convert slider values to normalized 0-1 range
    float pitchNorm = (float)(currentPitchCV - PITCH_CV_MIN) / (PITCH_CV_MAX - PITCH_CV_MIN);
    float cv1Norm = (float)(currentCV1 - CV1_MIN) / (CV1_MAX - CV1_MIN);
    float cv2Norm = (float)(currentCV2 - CV2_MIN) / (CV2_MAX - CV2_MIN);
    
    // Update DSP parameters
    tockusDSP->setParameters(pitchNorm, cv1Norm, cv2Norm, gateState);
}

void MainWindow::updateDisplay() {
    if (!tockusDSP || !pt8211DAC) return;
    
    // Update algorithm combo box if needed
    int currentAlgo = tockusDSP->getCurrentAlgorithm();
    if (algorithmCombo->currentIndex() != currentAlgo) {
        algorithmCombo->setCurrentIndex(currentAlgo);
    }
    
    // Update frequency display
    float freq = tockusDSP->getCurrentFrequency();
    frequencyDisplay->display(freq);
    
    // Update LED display
    updateLEDDisplay();
    
    // Update envelope display with actual envelope amplitude
    bool triggerActive = tockusDSP->isTriggerActive();
    if (triggerActive) {
        float envAmp = tockusDSP->getEnvelopeAmplitude();
        int envPercent = (int)(envAmp * 100.0f);
        envelopeDisplay->setValue(envPercent);
    } else {
        envelopeDisplay->setValue(0);
    }
    
    // Update DAC statistics
    dacTHDDisplay->display(pt8211DAC->getCurrentTHD() * 100.0); // Convert to percentage
    dacSNRDisplay->display(pt8211DAC->getCurrentSNR());
}

void MainWindow::updateLEDDisplay() {
    if (!tockusDSP) return;
    
    int algorithm = tockusDSP->getCurrentAlgorithm();
    QColor ledColor = ledColors[algorithm];
    
    // Brighten LED during trigger
    if (tockusDSP->isTriggerActive()) {
        // Make LED brighter during trigger
        ledColor = ledColor.lighter(150);
    }
    
    QString styleSheet = QString(
        "background-color: rgb(%1, %2, %3); "
        "border: 2px solid #333; "
        "border-radius: 20px;"
    ).arg(ledColor.red()).arg(ledColor.green()).arg(ledColor.blue());
    
    ledDisplay->setStyleSheet(styleSheet);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        onGatePressed();
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        onGateReleased();
        event->accept();
    } else {
        QMainWindow::keyReleaseEvent(event);
    }
}