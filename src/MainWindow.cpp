#include "MainWindow.h"
#include "CTSliceView.h"
#include "CTVolumeModel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSlider>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_volumeModel(new CTVolumeModel(this)),
    m_sliceViewer(new CTSliceView(this))
{
    setupUI();

    connect(m_volumeModel, &CTVolumeModel::volumeLoaded, this, &MainWindow::handleVolumeLoaded);
    connect(m_sliceSlider, &QSlider::valueChanged, this, &MainWindow::handleSliceChanged);

    connect(m_sliderWidth, &QSlider::valueChanged, this, [this](int val) {
        m_txtWidth->setText(QString::number(val));
        handleWindowingChanged();
    });
    connect(m_sliderCenter, &QSlider::valueChanged, this, [this](int val) {
        m_txtCenter->setText(QString::number(val));
        handleWindowingChanged();
    });

    connect(m_txtWidth, &QLineEdit::editingFinished, this, [this]() {
        int val = std::clamp(m_txtWidth->text().toInt(), m_sliderWidth->minimum(), m_sliderWidth->maximum());
        m_sliderWidth->setValue(val);
    });
    connect(m_txtCenter, &QLineEdit::editingFinished, this, [this]() {
        int val = std::clamp(m_txtCenter->text().toInt(), m_sliderCenter->minimum(), m_sliderCenter->maximum());
        m_sliderCenter->setValue(val);
    });

    connect(m_comboAnnotation, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleAnnotationSelection);
    connect(m_btnClearAnno, &QPushButton::clicked, m_sliceViewer, &CTSliceView::clearAnnotations);

    connect(m_sliceViewer, &CTSliceView::measurementUpdated, this, [this](double val){
        m_lblMeasurement->setText(QString("Distance: %1 px").arg(val, 0, 'f', 2));
    });

    updateUIStates();
    m_sliceViewer->updateSliceDisplay(nullptr, 0, 0);
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(14);

    QVBoxLayout *leftPanel = new QVBoxLayout();
    leftPanel->setSpacing(16);

    m_btnOpen = new QPushButton("Open CT Directory", this);
    m_btnOpen->setMinimumHeight(45);
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::handleOpenFolder);
    leftPanel->addWidget(m_btnOpen);

    QGroupBox *navGroup = new QGroupBox("Volume Navigation", this);
    QVBoxLayout *navLayout = new QVBoxLayout(navGroup);
    navLayout->setContentsMargins(12, 16, 12, 12);
    navLayout->setSpacing(10);

    m_sliceSlider = new QSlider(Qt::Horizontal, this);
    m_lblSliceInfo = new QLabel("Slice: 0 / 0", this);
    m_lblSliceInfo->setAlignment(Qt::AlignCenter);

    navLayout->addWidget(m_sliceSlider);
    navLayout->addWidget(m_lblSliceInfo);
    leftPanel->addWidget(navGroup);

    QGroupBox *windowGroup = new QGroupBox("Hounsfield Windowing", this);
    QVBoxLayout *windowLayout = new QVBoxLayout(windowGroup);
    windowLayout->setContentsMargins(12, 16, 12, 12);
    windowLayout->setSpacing(12);

    QLabel *lblWidthTitle = new QLabel("Window Width (Contrast)", this);
    QHBoxLayout *widthControlsLayout = new QHBoxLayout();
    m_sliderWidth = new QSlider(Qt::Horizontal, this);
    m_sliderWidth->setRange(1, 4000);
    m_sliderWidth->setValue(400);
    m_txtWidth = new QLineEdit("400", this);
    m_txtWidth->setFixedWidth(55);
    m_txtWidth->setAlignment(Qt::AlignCenter);
    widthControlsLayout->addWidget(m_sliderWidth);
    widthControlsLayout->addWidget(m_txtWidth);
    windowLayout->addWidget(lblWidthTitle);
    windowLayout->addLayout(widthControlsLayout);

    QLabel *lblCenterTitle = new QLabel("Window Center (Brightness)", this);
    QHBoxLayout *centerControlsLayout = new QHBoxLayout();
    m_sliderCenter = new QSlider(Qt::Horizontal, this);
    m_sliderCenter->setRange(-1024, 3072);
    m_sliderCenter->setValue(40);
    m_txtCenter = new QLineEdit("40", this);
    m_txtCenter->setFixedWidth(55);
    m_txtCenter->setAlignment(Qt::AlignCenter);
    centerControlsLayout->addWidget(m_sliderCenter);
    centerControlsLayout->addWidget(m_txtCenter);
    windowLayout->addWidget(lblCenterTitle);
    windowLayout->addLayout(centerControlsLayout);

    leftPanel->addWidget(windowGroup);

    QGroupBox *annoGroup = new QGroupBox("Annotation Engine", this);
    QVBoxLayout *annoLayout = new QVBoxLayout(annoGroup);
    annoLayout->setContentsMargins(12, 16, 12, 12);
    annoLayout->setSpacing(10);

    m_comboAnnotation = new QComboBox(this);
    m_comboAnnotation->addItem("Navigation Mode (Pan/Zoom)", static_cast<int>(AnnotationMode::None));
    m_comboAnnotation->addItem("Linear Distance Measure", static_cast<int>(AnnotationMode::Line));
    m_comboAnnotation->addItem("Region of Interest (Rect)", static_cast<int>(AnnotationMode::Rectangle));

    m_lblMeasurement = new QLabel("Distance: —", this);
    m_lblMeasurement->setAlignment(Qt::AlignCenter);
    m_btnClearAnno = new QPushButton("Clear Markings", this);

    annoLayout->addWidget(m_comboAnnotation);
    annoLayout->addWidget(m_lblMeasurement);
    annoLayout->addWidget(m_btnClearAnno);
    leftPanel->addWidget(annoGroup);

    leftPanel->addStretch();

    mainLayout->addLayout(leftPanel, 2);
    mainLayout->addWidget(m_sliceViewer, 5);

    resize(1100, 800);
    setWindowTitle("Mini Diagnostic CT Viewer Pro");

    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #1E1E1E;
        }
        QWidget {
            color: #DCDCDC;
            font-family: Arial, sans-serif;
            font-size: 12px;
        }
        QGroupBox {
            border: 1px solid #3A3A3A;
            border-radius: 6px;
            margin-top: 10px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 4px;
        }
        QLineEdit {
            background-color: #121212;
            border: 1px solid #444444;
            color: #00FFCC;
            border-radius: 3px;
            padding: 3px;
        }
        QComboBox, QPushButton {
            background-color: #2A2A2A;
            border: 1px solid #444444;
            border-radius: 4px;
            padding: 6px;
        }
        QPushButton:hover, QComboBox:hover {
            background-color: #3A3A3A;
        }
        QPushButton:disabled {
            color: #666666;
            background-color: #1C1C1C;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background: #2A2A2A;
        }
        QSlider::handle:horizontal {
            background: #00FFCC;
            width: 14px;
            margin: -5px 0;
            border-radius: 7px;
        }
    )");
}

void MainWindow::handleOpenFolder() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Medical CT Dataset Folder");
    if (!path.isEmpty()) {
        m_volumeModel->loadDirectory(path);
    }
}

void MainWindow::handleVolumeLoaded() {
    int totalSlices = m_volumeModel->getSliceCount();

    if (totalSlices <= 0) {
        m_lblSliceInfo->setText("Slice: 0 / 0");
        m_sliceViewer->updateSliceDisplay(nullptr, 0, 0);
        updateUIStates();
        return;
    }

    m_sliceSlider->setRange(0, totalSlices - 1);
    m_sliceSlider->setValue(totalSlices / 2);
    updateUIStates();
    handleSliceChanged(m_sliceSlider->value());
}

void MainWindow::handleSliceChanged(int value) {
    int totalSlices = m_volumeModel->getSliceCount();
    if (totalSlices == 0) return;

    m_lblSliceInfo->setText(QString("Slice: %1 / %2").arg(value + 1).arg(totalSlices));
    QSize dim = m_volumeModel->getSliceDimensions();

    m_sliceViewer->updateSliceDisplay(m_volumeModel->getSliceData(value), dim.width(), dim.height());
    m_sliceViewer->viewport()->update();
}

void MainWindow::handleWindowingChanged() {
    m_sliceViewer->setWindowing(m_sliderWidth->value(), m_sliderCenter->value());
}

void MainWindow::handleAnnotationSelection(int index) {
    AnnotationMode mode = static_cast<AnnotationMode>(m_comboAnnotation->itemData(index).toInt());
    m_sliceViewer->setAnnotationMode(mode);
}

void MainWindow::updateUIStates() {
    bool hasData = m_volumeModel->getSliceCount() > 0;

    m_sliceSlider->setEnabled(hasData);
    m_sliderWidth->setEnabled(hasData);
    m_txtWidth->setEnabled(hasData);
    m_sliderCenter->setEnabled(hasData);
    m_txtCenter->setEnabled(hasData);
    m_comboAnnotation->setEnabled(hasData);
    m_btnClearAnno->setEnabled(hasData);
}
