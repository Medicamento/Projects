#include "plot.h"
#include <iostream>

Plot::Plot() {
    size = 10;
    curGraph1Data = new QtDataVisualization::QSurfaceDataArray;
    curGraph2Data = new QtDataVisualization::QSurfaceDataArray;
    generateSincData1(50, 50);
    generateSincData2(50, 50);
}

void Plot::generateSincData1(int rowCount, int columnCount) {
    curGraph1Data->clear();
    curGraph1Data->reserve(rowCount);

    double xStep = size * 2 / rowCount;
    double zStep = size * 2 / columnCount;
    double x = -size;

    for (int i = 0; i < rowCount; ++i) {
        QtDataVisualization::QSurfaceDataRow *newRow =
                new QtDataVisualization::QSurfaceDataRow(columnCount);
        double z = -size;

        for (int j = 0; j < columnCount; ++j) {
            double distanceFromZero = std::sqrt(x * x + z * z);
            double sincValue;
            if (distanceFromZero != 0)
                sincValue = size * std::sin(distanceFromZero) / distanceFromZero;
            else
                sincValue = size;

            (*newRow)[j].setPosition(QVector3D(z, sincValue, x));
            z += zStep;
        }

        curGraph1Data->append(newRow);
        x += xStep;
    }
}

void Plot::generateSincData2(int rowCount, int columnCount) {
    curGraph2Data->clear();
    curGraph2Data->reserve(rowCount);

    double xStep = size * 2 / rowCount;
    double zStep = size * 2 / columnCount;
    double x = -size;

    for (int i = 0; i < rowCount; ++i) {
        QtDataVisualization::QSurfaceDataRow *newRow =
                new QtDataVisualization::QSurfaceDataRow(columnCount);
        double z = -size;
        double sincx = (x == 0) ? 1 : std::sin(x) / x;

        for (int j = 0; j < columnCount; ++j) {
            double sincz = (z == 0) ? 1 : std::sin(z) / z;
            double sincValue = size * sincx * sincz;
            (*newRow)[j].setPosition(QVector3D(z, sincValue, x));
            z += zStep;
        }

        curGraph2Data->append(newRow);
        x += xStep;
    }
}

QtDataVisualization::QSurfaceDataArray *Plot::curGraph1DataCopy() {
    QtDataVisualization::QSurfaceDataArray *newDataArray =
            new QtDataVisualization::QSurfaceDataArray();

    for (int i = 0; i < curGraph1Data->size(); ++i) {
        QVector <QtDataVisualization::QSurfaceDataItem> *newItem =
                new QVector<QtDataVisualization::QSurfaceDataItem>(
                        *curGraph1Data->at(i));
        newDataArray->append(newItem);
    }

    return newDataArray;
}

QtDataVisualization::QSurfaceDataArray *Plot::curGraph2DataCopy() {
    QtDataVisualization::QSurfaceDataArray *newDataArray =
            new QtDataVisualization::QSurfaceDataArray();

    for (int i = 0; i < curGraph1Data->size(); ++i) {
        QVector <QtDataVisualization::QSurfaceDataItem> *newItem =
                new QVector<QtDataVisualization::QSurfaceDataItem>(
                        *curGraph2Data->at(i));
        newDataArray->append(newItem);
    }

    return newDataArray;
}

void Plot::changeData(int rowCount, int columnCount) {
    generateSincData1(rowCount, columnCount);
    generateSincData2(rowCount, columnCount);
}
