#ifndef CTSLICEVIEW_H
#define CTSLICEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QWheelEvent>

enum class AnnotationMode { None, Line, Rectangle };

class CTSliceView : public QGraphicsView {
    Q_OBJECT
public:
    explicit CTSliceView(QWidget *parent = nullptr);
    ~CTSliceView() = default;

    void updateSliceDisplay(const int16_t* rawData, int width, int height);
    void setWindowing(int windowWidth, int windowCenter);
    void setAnnotationMode(AnnotationMode mode);
    void clearAnnotations();

signals:
    void measurementUpdated(double value);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void applyWindowing();

    QGraphicsScene      *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;

    const int16_t* m_currentRawData = nullptr;
    int m_imgWidth = 0;
    int m_imgHeight = 0;

    int m_windowWidth = 400;
    int m_windowCenter = 40;

    AnnotationMode m_annoMode = AnnotationMode::None;
    bool m_isPanning = false;
    bool m_isDrawing = false;
    QPoint m_lastPanPoint;
    QPointF m_startDrawPoint;

    QGraphicsLineItem *m_activeLine = nullptr;
    QGraphicsRectItem *m_activeRect = nullptr;
};

#endif
