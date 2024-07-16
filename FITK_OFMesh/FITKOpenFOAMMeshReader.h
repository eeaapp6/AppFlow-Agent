#ifndef FITKOPENFOAMMESHREADER_H
#define FITKOPENFOAMMESHREADER_H

#include "FITKOFMeshAPI.h"
#include "FITK_Interface/FITKInterfaceIO/FITKAbstractIO.h"

class QFile;

namespace Interface {
    class FITKUnstructuredMesh;
    class FITKAbstractElement;

    struct FoamFileHeader {
        QString format{};
        QString clazz{};
        QString location{};
        QString object{};
    };

    class FITKOFMeshAPI FITKOpenFOAMMeshReader : public FITKAbstractIO
    {
    public:
        FITKOpenFOAMMeshReader();
        ~FITKOpenFOAMMeshReader() override;

        void run() override;
        void consoleMessage(int level, const QString &str) override;

    protected:
        bool read();
        bool processLine(QString& line, bool inComment = false);
        bool readFoamFileHeader(QFile& file, FoamFileHeader& header);
        bool readPointsData(QFile& file);
        bool readPoints(const QString &points);
        bool readFacesData(QFile& file);
        bool readFaces(const QString &faces);
        bool readOwnerData(QFile& file);
        bool readOwner(const QString &owner);
        bool readNeighbourData(QFile& file);
        bool readNeighbour(const QString &owner);
        bool getHex8IdsByFace(QList<int> face, int inIdPrev, int inIdNext, int& outIdPrev, int& outIdNext);
        Interface::FITKAbstractElement* setupHex8Cell(int eleIndex, QVector<QList<int>> points);
        Interface::FITKAbstractElement* setupCell(int eleIndex, QList<int> faces);
        bool setupCells();

    private:
        /**
         * @brief  内部网格数据
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        FITKUnstructuredMesh* _unstructuredMesh{};
        /**
         * @brief  节点总数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        int _pointsNum{};
        /**
         * @brief  face总数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        int _facesNum{};
        /**
         * @brief  face列表<节点索引列表>
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        QVector<QList<int>> _faces{};
        /**
         * @brief  owner哈希<所属单元owner索引，face索引列表>
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        QMap<int, QList<int>> _owner{};
        /**
         * @brief  邻居单元<face索引, 邻居单元索引>
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        QMap<int, int> _neighbour{};
        /**
         * @brief  邻居单元<单元索引, face索引列表>
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-16
         */
        QMap<int, QList<int>> _tempNeighbour{};

    };
}
#endif // FITKOPENFOAMMESHREADER_H
