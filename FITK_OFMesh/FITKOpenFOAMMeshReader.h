/**
 * @file   FITKOpenFOAMMeshReader.h
 * @brief  OpenFOAM网格文件读取类
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-07-17
 */
#ifndef FITKOPENFOAMMESHREADER_H
#define FITKOPENFOAMMESHREADER_H

#include "FITKOFMeshAPI.h"
#include "FITK_Interface/FITKInterfaceIO/FITKAbstractIO.h"

class QFile;

namespace Interface {
    class FITKUnstructuredMesh;
    class FITKAbstractElement;

    /**
     * @brief  OpenFOAM文件头信息
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-07-17
     */
    struct FoamFileHeader {
        QString format{};
        QString clazz{};
        QString location{};
        QString object{};
    };
    /**
     * @brief  OpenFOAM网格文件读取类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-07-17
     */
    class FITKOFMeshAPI FITKOpenFOAMMeshReader : public FITKAbstractIO
    {
    public:
        /**
         * @brief  构造函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        FITKOpenFOAMMeshReader();
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        ~FITKOpenFOAMMeshReader() override;
        /**
         * @brief  重写线程启动函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        void run() override;
        /**
         * @brief 打印控制台消息
         * @param[i] level 打印级别 1 normal 2 warning 3error
         * @param[i] str 消息
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        void consoleMessage(int level, const QString &str) override;

    protected:
        /**
         * @brief  读取网格文件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool read();
        /**
         * @brief  处理行数据
         * @param  line 行数据
         * @param  inComment 上一次读取完行数据是否在注释中
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool processLine(QString& line, bool inComment = false);
        /**
         * @brief  读取OpenFOAM文件头信息
         * @param  file 文件
         * @param[out]  header 头信息结构体
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readFoamFileHeader(QFile& file, FoamFileHeader& header);
        /**
         * @brief  读取points文件中的点数据
         * @param  file 文件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readPointsData(QFile& file);
        /**
         * @brief  读取points文件
         * @param  points points文件路径
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readPoints(const QString &points);
        /**
         * @brief  读取faces文件中的面数据
         * @param  file 文件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readFacesData(QFile& file);
        /**
         * @brief  读取faces文件
         * @param  points faces文件路径
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readFaces(const QString &faces);
        /**
         * @brief  读取owner文件中的所属关系数据
         * @param  file 文件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readOwnerData(QFile& file);
        /**
         * @brief  读取owner文件
         * @param  points owner文件路径
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readOwner(const QString &owner);
        /**
         * @brief  读取neighbour文件中的相邻关系数据
         * @param  file 文件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readNeighbourData(QFile& file);
        /**
         * @brief  读取neighbour文件
         * @param  points neighbour文件路径
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool readNeighbour(const QString &owner);
        /**
         * @brief  计算hex8单元的节点信息
         * @param  face 面（节点列表）
         * @param  inIdPrev 已知节点1（按OpenFOAM右手法则排序在前）
         * @param  inIdNext 已知节点2（按OpenFOAM右手法则排序在后）
         * @param[out]  outIdPrev 未知节点1（按OpenFOAM右手法则排序在前）
         * @param[out]  outIdNext 未知节点2（按OpenFOAM右手法则排序在后）
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        bool getHex8IdsByFace(QList<int> face, int inIdPrev, int inIdNext, int& outIdPrev, int& outIdNext);
        /**
         * @brief  装配hex8单元
         * @param  eleIndex 单元索引
         * @param  points 每个面上点的列表
         * @return 装配完的hex8单元或者空指针
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        Interface::FITKAbstractElement* setupHex8Cell(int eleIndex, QVector<QList<int>> points);
        /**
         * @brief  装配单元
         * @param  eleIndex 单元索引
         * @param  faces 面索引列表
         * @return 装配完的单元或空指针
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
        Interface::FITKAbstractElement* setupCell(int eleIndex, QList<int> faces);
        /**
         * @brief  装配单元
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-07-17
         */
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
