#ifndef CTVOLUMEMODEL_H
#define CTVOLUMEMODEL_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSize>

struct SliceData {
    int width;
    int height;
    QVector<int16_t> rawPixelData;
};

class CTVolumeModel : public QObject {
    Q_OBJECT
public:
    explicit CTVolumeModel(QObject *parent = nullptr);
    ~CTVolumeModel() = default;

    bool loadDirectory(const QString &dirPath);
    void generateMockCTData();

    int getSliceCount() const;
    QSize getSliceDimensions() const;
    const int16_t* getSliceData(int index) const;

signals:
    void volumeLoaded();

private:
    QVector<SliceData> m_volume;
    int m_width;
    int m_height;
};

#endif
