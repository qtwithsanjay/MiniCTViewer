#include "CTVolumeModel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QImage>
#include <cmath>

CTVolumeModel::CTVolumeModel(QObject *parent) : QObject(parent), m_width(512), m_height(512) {}

bool CTVolumeModel::loadDirectory(const QString &dirPath) {
    if (dirPath.isEmpty()) {
        generateMockCTData();
        emit volumeLoaded();
        return true;
    }

    QDir dir(dirPath);
    QStringList filters;
    filters << "*.dcm" << "*.DCM" << "*.raw" << "*.ima" << "*.png" << "*.PNG";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Name);

    QFileInfoList fileList = dir.entryInfoList();
    if (fileList.isEmpty()) {
        generateMockCTData();
        emit volumeLoaded();
        return true;
    }

    m_volume.clear();
    m_width = 512;
    m_height = 512;
    int pixelCount = m_width * m_height;

    for (const QFileInfo &fileInfo : fileList) {
        QString suffix = fileInfo.suffix().toLower();
        SliceData slice;
        slice.width = m_width;
        slice.height = m_height;
        slice.rawPixelData.resize(pixelCount);

        if (suffix == "png") {
            QImage img(fileInfo.absoluteFilePath());
            if (!img.isNull()) {
                QImage grayImg = img.convertToFormat(QImage::Format_Grayscale8);
                if (grayImg.width() != m_width || grayImg.height() != m_height) {
                    grayImg = grayImg.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                }

                const uchar* bits = grayImg.constBits();
                for (int i = 0; i < pixelCount; ++i) {
                    slice.rawPixelData[i] = static_cast<int16_t>((bits[i] * 4) - 200);
                }
                m_volume.append(slice);
            }
            continue;
        }

        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) continue;

        QByteArray fileData = file.readAll();
        file.close();

        int jpegOffset = fileData.indexOf("\xFF\xD8");
        const unsigned char pixelTag[] = { 0xE0, 0x7F, 0x10, 0x00 };
        int tagIndex = fileData.indexOf(QByteArray(reinterpret_cast<const char*>(pixelTag), 4));

        if (jpegOffset != -1) {
            int jpegEndOffset = fileData.indexOf("\xFF\xD9", jpegOffset);
            QByteArray jpegData;
            if (jpegEndOffset != -1) {
                int jpegLength = (jpegEndOffset + 2) - jpegOffset;
                jpegData = fileData.mid(jpegOffset, jpegLength);
            } else {
                jpegData = fileData.mid(jpegOffset);
            }

            QImage img;
            if (img.loadFromData(jpegData, "JPEG")) {
                QImage grayImg = img.convertToFormat(QImage::Format_Grayscale8);
                if (grayImg.width() != m_width || grayImg.height() != m_height) {
                    grayImg = grayImg.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                }

                const uchar* bits = grayImg.constBits();
                for (int i = 0; i < pixelCount; ++i) {
                    slice.rawPixelData[i] = static_cast<int16_t>((bits[i] * 4) - 200);
                }
            }
        }
        else if (tagIndex != -1) {
            int pixelDataOffset = tagIndex + 12;
            if (pixelDataOffset + (pixelCount * (int)sizeof(int16_t)) <= fileData.size()) {
                const int16_t* srcPixels = reinterpret_cast<const int16_t*>(fileData.constData() + pixelDataOffset);
                for (int i = 0; i < pixelCount; ++i) {
                    slice.rawPixelData[i] = srcPixels[i];
                }
            }
        }
        else {
            if (fileData.size() >= pixelCount * (int)sizeof(int16_t)) {
                memcpy(slice.rawPixelData.data(), fileData.constData() + (fileData.size() - pixelCount * sizeof(int16_t)), pixelCount * sizeof(int16_t));
            }
        }

        m_volume.append(slice);
    }

    emit volumeLoaded();
    return true;
}

void CTVolumeModel::generateMockCTData() {
    m_volume.clear();
    m_width = 512;
    m_height = 512;
    int totalSlices = 60;

    for (int z = 0; z < totalSlices; ++z) {
        SliceData slice;
        slice.width = m_width;
        slice.height = m_height;
        slice.rawPixelData.resize(m_width * m_height);

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                float dx = x - m_width / 2.0f;
                float dy = y - m_height / 2.0f;
                float r = std::sqrt(dx*dx + dy*dy);

                int16_t val = -1000;

                if (r < 180.0f) {
                    val = 0;
                    if (r > 160.0f && r < 175.0f) {
                        val = 1100;
                    }
                    float internalDist1 = std::sqrt(std::pow(dx - 40, 2) + std::pow(dy - (z * 2 - 60), 2));
                    if (internalDist1 < 35.0f) {
                        val = 300;
                    }
                    float internalDist2 = std::sqrt(std::pow(dx + 50, 2) + std::pow(dy + 30, 2));
                    if (internalDist2 < 45.0f) {
                        val = -500;
                    }
                }
                slice.rawPixelData[y * m_width + x] = val;
            }
        }
        m_volume.append(slice);
    }
}

int CTVolumeModel::getSliceCount() const {
    return m_volume.size();
}

QSize CTVolumeModel::getSliceDimensions() const {
    return QSize(m_width, m_height);
}

const int16_t* CTVolumeModel::getSliceData(int index) const {
    if (index < 0 || index >= m_volume.size()) return nullptr;
    return m_volume[index].rawPixelData.constData();
}
