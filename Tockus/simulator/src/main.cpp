#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Tockus Simulator");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("BirdsBoards");
    app.setOrganizationDomain("birdsboards.com");
    
    // Set a modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Dark theme palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Additional styling for synthesizer look
    app.setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 5px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #FFF;
        }
        
        QSlider::groove:horizontal {
            border: 1px solid #999;
            height: 8px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #B1B1B1, stop:1 #c4c4c4);
            margin: 2px 0;
            border-radius: 4px;
        }
        
        QSlider::handle:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #b4b4b4, stop:1 #8f8f8f);
            border: 1px solid #5c5c5c;
            width: 18px;
            margin: -2px 0;
            border-radius: 3px;
        }
        
        QSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #d4d4d4, stop:1 #afafaf);
        }
        
        QPushButton {
            background-color: #404040;
            border: 2px solid #555;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            min-width: 80px;
        }
        
        QPushButton:hover {
            background-color: #505050;
            border-color: #777;
        }
        
        QPushButton:pressed {
            background-color: #303030;
            border-color: #333;
        }
        
        QPushButton#gateButton {
            background-color: #8B0000;
            border-color: #A52A2A;
        }
        
        QPushButton#gateButton:hover {
            background-color: #A52A2A;
        }
        
        QPushButton#gateButton:pressed {
            background-color: #FF4444;
        }
        
        QLCDNumber {
            background-color: #000;
            color: #0F0;
            border: 2px solid #333;
            border-radius: 4px;
        }
        
        QComboBox {
            background-color: #404040;
            border: 2px solid #555;
            border-radius: 4px;
            padding: 4px 8px;
            min-width: 120px;
        }
        
        QComboBox:hover {
            border-color: #777;
        }
        
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 15px;
            border-left-width: 1px;
            border-left-color: #555;
            border-left-style: solid;
            border-top-right-radius: 3px;
            border-bottom-right-radius: 3px;
        }
        
        QProgressBar {
            border: 2px solid #555;
            border-radius: 4px;
            text-align: center;
            background-color: #000;
        }
        
        QProgressBar::chunk {
            background-color: #0F0;
            border-radius: 2px;
        }
    )");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    qDebug() << "Tockus Simulator started";
    
    return app.exec();
}