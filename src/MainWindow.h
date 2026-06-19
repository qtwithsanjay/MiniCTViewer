#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "CTSliceView.h"

class CTVolumeModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void handleOpenFolder();
    void handleVolumeLoaded();
    void handleSliceChanged(int value);
    void handleWindowingChanged();
    void handleAnnotationSelection(int index);

private:
    void setupUI();
    void updateUIStates();

    CTVolumeModel *m_volumeModel;
    CTSliceView   *m_sliceViewer;

    QPushButton   *m_btnOpen;
    QSlider       *m_sliceSlider;
    QLabel        *m_lblSliceInfo;

    QSlider       *m_sliderWidth;
    QLineEdit     *m_txtWidth;
    QSlider       *m_sliderCenter;
    QLineEdit     *m_txtCenter;

    QComboBox     *m_comboAnnotation;
    QLabel        *m_lblMeasurement;
    QPushButton   *m_btnClearAnno;
};

#endif
