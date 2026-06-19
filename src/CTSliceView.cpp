#include "CTSliceView.h"
#include <QScrollBar>
#include <algorithm>
#include <cmath>

CTSliceView::CTSliceView(QWidget *parent)
    : QGraphicsView(parent), m_scene(new QGraphicsScene(this)), m_pixmapItem(new QGraphicsPixmapItem())
{
    m_scene->addItem(m_pixmapItem);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void CTSliceView::updateSliceDisplay(const int16_t* rawData, int width, int height) {
    if (!rawData || width <= 0 || height <= 0) {
        m_currentRawData = nullptr;
        m_pixmapItem->setPixmap(QPixmap());
        m_scene->setSceneRect(0, 0, 0, 0);
        return;
    }

    m_currentRawData = rawData;
    m_imgWidth = width;
    m_imgHeight = height;
    applyWindowing();

    m_scene->setSceneRect(0, 0, width, height);
}

void CTSliceView::applyWindowing() {
    if (!m_currentRawData || m_imgWidth <= 0 || m_imgHeight <= 0) {
        m_pixmapItem->setPixmap(QPixmap());
        return;
    }

    QImage image(m_imgWidth, m_imgHeight, QImage::Format_Grayscale8);
    uchar* bits = image.bits();

    int lowBound = m_windowCenter - (m_windowWidth / 2);
    float invWidth = 255.0f / m_windowWidth;

    int totalPixels = m_imgWidth * m_imgHeight;
    for (int i = 0; i < totalPixels; ++i) {
        int16_t huValue = m_currentRawData[i];
        int transformed = static_cast<int>((huValue - lowBound) * invWidth);
        bits[i] = static_cast<uchar>(std::clamp(transformed, 0, 255));
    }

    m_pixmapItem->setPixmap(QPixmap::fromImage(image));
}

void CTSliceView::setWindowing(int windowWidth, int windowCenter) {
    m_windowWidth = std::max(1, windowWidth);
    m_windowCenter = windowCenter;
    if (m_currentRawData) {
        applyWindowing();
    }
}

void CTSliceView::setAnnotationMode(AnnotationMode mode) {
    m_annoMode = mode;
}

void CTSliceView::clearAnnotations() {
    for (auto item : m_scene->items()) {
        if (item != m_pixmapItem) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    m_activeLine = nullptr;
    m_activeRect = nullptr;
}

void CTSliceView::wheelEvent(QWheelEvent *event) {
    double scaleFactor = 1.15;
    if (event->angleDelta().y() < 0) {
        scaleFactor = 1.0 / scaleFactor;
    }
    scale(scaleFactor, scaleFactor);
}

void CTSliceView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_annoMode == AnnotationMode::None) {
            m_isPanning = true;
            m_lastPanPoint = event->pos();
            setCursor(Qt::ClosedHandCursor);
        } else {
            m_isDrawing = true;
            m_startDrawPoint = mapToScene(event->pos());

            QPen pen(Qt::cyan, 2);
            if (m_annoMode == AnnotationMode::Line) {
                m_activeLine = m_scene->addLine(QLineF(m_startDrawPoint, m_startDrawPoint), pen);
            } else if (m_annoMode == AnnotationMode::Rectangle) {
                m_activeRect = m_scene->addRect(QRectF(m_startDrawPoint, QSizeF(0, 0)), pen);
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void CTSliceView::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    } else if (m_isDrawing) {
        QPointF currentPoint = mapToScene(event->pos());

        if (m_annoMode == AnnotationMode::Line && m_activeLine) {
            m_activeLine->setLine(QLineF(m_startDrawPoint, currentPoint));
            double length = std::hypot(currentPoint.x() - m_startDrawPoint.x(), currentPoint.y() - m_startDrawPoint.y());
            emit measurementUpdated(length);
        } else if (m_annoMode == AnnotationMode::Rectangle && m_activeRect) {
            qreal x = std::min(m_startDrawPoint.x(), currentPoint.x());
            qreal y = std::min(m_startDrawPoint.y(), currentPoint.y());
            qreal w = std::abs(currentPoint.x() - m_startDrawPoint.x());
            qreal h = std::abs(currentPoint.y() - m_startDrawPoint.y());
            m_activeRect->setRect(QRectF(x, y, w, h));
            emit measurementUpdated(std::hypot(w, h));
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CTSliceView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        m_isDrawing = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}
