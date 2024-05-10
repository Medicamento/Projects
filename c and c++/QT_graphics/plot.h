#ifndef PLOT_H
#define PLOT_H

#include <QtDataVisualization>

class Plot {
public:
    Plot();

    ~Plot() {}

    QtDataVisualization::QSurfaceDataArray *curGraph1DataCopy();

    QtDataVisualization::QSurfaceDataArray *curGraph2DataCopy();

    void changeData(int rowCount, int columnCount);

private:
    void generateSincData1(int rowCount, int columnCount);

    void generateSincData2(int rowCount, int columnCount);

    QtDataVisualization::QSurfaceDataArray *curGraph1Data;
    QtDataVisualization::QSurfaceDataArray *curGraph2Data;
    double size;
};

#endif // PLOT_H
