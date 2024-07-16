#include "FITKOpenFOAMMeshReader.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include "FITK_Interface/FITKInterfaceModel/FITKUnstructuredMesh.h"
#include "FITK_Interface/FITKInterfaceModel/FITKElementHex.h"

namespace Interface {


    FITKOpenFOAMMeshReader::FITKOpenFOAMMeshReader() = default;

    FITKOpenFOAMMeshReader::~FITKOpenFOAMMeshReader() = default;

    void FITKOpenFOAMMeshReader::run()
    {
        read();
    }

    void FITKOpenFOAMMeshReader::consoleMessage(int level, const QString & str)
    {
    }

    bool FITKOpenFOAMMeshReader::read()
    {
        if (_fileName.isEmpty()) return false;
        QFileInfo fileInfo(_fileName);
        if (!fileInfo.isDir()) return false;

        auto dir = fileInfo.dir();
        if (!dir.exists()) return false;

        if (!dir.exists("points") || !dir.exists("faces") || !dir.exists("owner") || !dir.exists("neighbour")) return false;

        _unstructuredMesh = new Interface::FITKUnstructuredMesh;

        if (!readPoints(dir.filePath("points"))) return false;

        if (!readFaces(dir.filePath("faces"))) return false;

        if (!readOwner(dir.filePath("owner"))) return false;

        if (!readNeighbour(dir.filePath("neighbour"))) return false;

        if (!setupCells()) return false;
        return true;
    }

    bool FITKOpenFOAMMeshReader::processLine(QString& line, bool inComment)
    {
        auto beginPos = line.indexOf("/*");
        auto endPos = line.indexOf("*/");
        auto singlePos = line.indexOf("//");
        // 当前行为注释内容
        if (inComment) {
            // 注释尚未结束
            if (endPos < 0) {
                line.clear();
                return true;
            }
            // 注释结束
            else {
                line = line.mid(endPos + 2);
                return false;
            }
        }
        else {
            // 当前行包含注释且在行内结束
            if (beginPos >= 0 && endPos >= 0) {
                line = line.mid(0, beginPos) + line.mid(endPos);
                return false;
            }
            // 当前行包含注释尚未在行内结束
            else if (beginPos >= 0) {
                line = line.mid(0, beginPos);
                return true;
            }
            // 当前行包含当行注释
            else if (singlePos >= 0) {
                line = line.mid(0, singlePos);
                return false;
            }
            else {
                return false;
            }
        }
    }

    bool FITKOpenFOAMMeshReader::readFoamFileHeader(QFile& file, FoamFileHeader& header) {
        QString line;
        bool isInt{ false };
        do
        {
            line = file.readLine().simplified();
            // 读到整数表示异常
            line.toInt(&isInt);
            if (isInt) return false;
            auto strs = line.split(' ', Qt::SkipEmptyParts);
            if (strs.size() != 2) continue;
            if (strs.at(0) == "format") {
                header.format = strs.at(1).chopped(1);
            }
            else if (strs.at(0) == "class") {
                header.clazz = strs.at(1).chopped(1);
            }
            else if (strs.at(0) == "location") {
                header.location = strs.at(1).chopped(1);
            }
            else if (strs.at(0) == "object") {
                header.object = strs.at(1).chopped(1);
            }

        } while (!line.startsWith("}"));
        return true;
    }

    bool FITKOpenFOAMMeshReader::readPointsData(QFile& file) {
        if (_unstructuredMesh == nullptr) return false;
        QString line;
        bool ok1, ok2, ok3;
        double x, y, z;
        do
        {
            line = file.readLine().simplified();
            if (line == "(") continue;
            if (line == ")") break;
            // 移除括号
            line = line.chopped(1).remove(0, 1);
            auto strs = line.split(' ', Qt::SkipEmptyParts);
            if (strs.size() != 3) continue;
            x = strs.at(0).toDouble(&ok1);
            y = strs.at(1).toDouble(&ok2);
            z = strs.at(2).toDouble(&ok3);
            if (ok1 && ok2 && ok3) {
                _unstructuredMesh->addNode(x, y, z);
            }
            else {
                return false;
            }
        } while (!file.atEnd());
        return true;
    }

    bool FITKOpenFOAMMeshReader::readPoints(const QString &points)
    {
        if (_unstructuredMesh == nullptr) return false;

        QFile pointsFile(points);

        if (!pointsFile.exists()) return false;

        if (!pointsFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

        QString line;
        bool inComment{ false };
        bool isInt{ false };
        while (!pointsFile.atEnd())
        {
            line = pointsFile.readLine();
            inComment = processLine(line, inComment);
            line = line.simplified();
            if (line.isEmpty()) continue;
            // 读取文件头信息
            if (line.startsWith("FoamFile")) {
                FoamFileHeader header;
                if (!readFoamFileHeader(pointsFile, header)) return false;
                // 只支持文本格式
                if (header.format != "ascii") return false;
            }

            // 读取点数据
            _pointsNum = line.toInt(&isInt);
            if (isInt) {
                if (!readPointsData(pointsFile)) return false;
            }
        }
        pointsFile.close();
        return true;
    }

    bool FITKOpenFOAMMeshReader::readFacesData(QFile& file) {
        QString line;
        bool ok;
        int i = -1;
        do
        {
            line = file.readLine().simplified();
            if (line == "(") continue;
            if (line == ")") break;

            auto strs = line.chopped(1).split('(', Qt::SkipEmptyParts);
            if (strs.size() != 2) continue;

            QList<QString> indexStrs = strs.at(1).split(' ', Qt::SkipEmptyParts);
            QList<int> indexs{};
            for (auto str : indexStrs)
            {
                auto val = str.toInt(&ok);
                if (ok) indexs.append(val);
                else return false;
            }
            if (QString("%1").arg(indexs.size()) != strs.at(0)) {
                return false;
            }

            _faces[++i] = indexs;
        } while (!file.atEnd());

        if (_faces.size() != _facesNum) return false;
        return true;
    }

    bool FITKOpenFOAMMeshReader::readFaces(const QString &faces)
    {
        QFile facesFile(faces);

        if (!facesFile.exists()) return false;

        if (!facesFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

        QString line;
        bool inComment{ false };
        bool isInt{ false };
        while (!facesFile.atEnd())
        {
            line = facesFile.readLine();
            inComment = processLine(line, inComment);
            line = line.simplified();
            if (line.isEmpty()) continue;
            // 读取文件头信息
            if (line.startsWith("FoamFile")) {
                FoamFileHeader header;
                if (!readFoamFileHeader(facesFile, header)) return false;
                // 只支持文本格式
                if (header.format != "ascii") return false;
            }

            // 读取面数据
            _facesNum = line.toInt(&isInt);
            if (isInt) {
                _faces.resize(_facesNum);
                if (!readFacesData(facesFile)) return false;
            }
        }
        facesFile.close();

        return true;
    }

    bool FITKOpenFOAMMeshReader::readOwnerData(QFile& file) {
        QString line;
        bool ok;
        int faceIndex = -1;
        do
        {
            line = file.readLine().simplified();
            if (line == "(") continue;
            if (line == ")") break;

            auto ownerIndex = line.toInt(&ok);
            if (ok) {
                _owner[ownerIndex].append(++faceIndex);
            }
            else {
                return false;
            }
        } while (!file.atEnd());
        return true;
    }

    bool FITKOpenFOAMMeshReader::readOwner(const QString &owner)
    {
        QFile ownerFile(owner);

        if (!ownerFile.exists()) return false;

        if (!ownerFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

        QString line;
        bool inComment{ false };
        bool isInt{ false };
        int ownerNum{};
        while (!ownerFile.atEnd())
        {
            line = ownerFile.readLine();
            inComment = processLine(line, inComment);
            line = line.simplified();
            if (line.isEmpty()) continue;
            // 读取文件头信息
            if (line.startsWith("FoamFile")) {
                FoamFileHeader header;
                if (!readFoamFileHeader(ownerFile, header)) return false;
                // 只支持文本格式
                if (header.format != "ascii") return false;
            }

            // 读取Owner数据
            ownerNum = line.toInt(&isInt);
            if (isInt) {
                if (!readOwnerData(ownerFile)) return false;
            }
        }
        ownerFile.close();

        return true;
    }

    bool FITKOpenFOAMMeshReader::readNeighbourData(QFile& file)
    {
        QString line;
        bool ok;
        int faceIndex = -1;
        do
        {
            line = file.readLine().simplified();
            if (line == "(") continue;
            if (line == ")") break;

            auto neighbourIndex = line.toInt(&ok);
            if (ok) {
                _neighbour[++faceIndex] = neighbourIndex;
                _tempNeighbour[neighbourIndex].append(faceIndex);
            }
            else {
                return false;
            }
        } while (!file.atEnd());
        return true;
    }

    bool FITKOpenFOAMMeshReader::readNeighbour(const QString &neighbour)
    {
        QFile neighbourFile(neighbour);

        if (!neighbourFile.exists()) return false;

        if (!neighbourFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

        QString line;
        bool inComment{ false };
        bool isInt{ false };
        int neighbourNum{};
        while (!neighbourFile.atEnd())
        {
            line = neighbourFile.readLine();
            inComment = processLine(line, inComment);
            line = line.simplified();
            if (line.isEmpty()) continue;
            // 读取文件头信息
            if (line.startsWith("FoamFile")) {
                FoamFileHeader header;
                if (!readFoamFileHeader(neighbourFile, header)) return false;
                // 只支持文本格式
                if (header.format != "ascii") return false;
            }

            // 读取Owner数据
            neighbourNum = line.toInt(&isInt);
            if (isInt) {
                if (!readNeighbourData(neighbourFile)) return false;
            }
        }
        neighbourFile.close();

        return true;
    }

    bool FITKOpenFOAMMeshReader::getHex8IdsByFace(QList<int> face, int inIdPrev, int inIdNext, int& outIdPrev, int& outIdNext)
    {
        int idNextPos = face.indexOf(inIdNext);
        int idPrevPos = face.indexOf(inIdPrev);
        // id1必须在id2后面，且必须相邻
        if (idPrevPos == 0 && idNextPos == 1) {
            outIdNext = face.at(3);
            outIdPrev = face.at(2);
        }
        else if (idPrevPos == 1 && idNextPos == 2) {
            outIdNext = face.at(0);
            outIdPrev = face.at(3);
        }
        else if (idPrevPos == 2 && idNextPos == 3) {
            outIdNext = face.at(1);
            outIdPrev = face.at(0);
        }
        else if (idPrevPos == 3 && idNextPos == 0) {
            outIdNext = face.at(2);
            outIdPrev = face.at(1);
        }
        return true;
    }

    Interface::FITKAbstractElement* FITKOpenFOAMMeshReader::setupHex8Cell(int eleIndex, QVector<QList<int>> points) {

        auto faceNum = points.size();
        if (faceNum < 3 || faceNum > 6) return nullptr;

        // 假定第一个面为底面
        QList<int> bottomFace = points.at(0);
        if (bottomFace.size() != 4) return nullptr;


        int id0 = bottomFace.at(0);
        int id1 = bottomFace.at(1);
        int id2 = bottomFace.at(2);
        int id3 = bottomFace.at(3);

        int id4{ -1 }, id5{ -1 }, id6{ -1 }, id7{ -1 };

        QList<int> leftFace{};
        QList<int> rightFace{};
        QList<int> frontFace{};
        QList<int> backFace{};

        // 只有三个面不足以确定所有顶点，需要添加邻面
        if (faceNum < 5) {
            auto faces = _tempNeighbour[eleIndex];
            for (auto faceIndex : faces) {
                auto face = _faces[faceIndex];
                if (face.size() != 4) return nullptr;
                points.append({ face.at(3), face.at(2), face.at(1), face.at(0) });
            }
        }
        faceNum = points.size();

        for (int i = 1; i < faceNum; ++i) {
            auto face = points.at(i);
            if (face.contains(id0) && face.contains(id1)) {
                leftFace = std::move(face);
            }
            else if (face.contains(id1) && face.contains(id2)) {
                backFace = std::move(face);
            }
            else if (face.contains(id2) && face.contains(id3)) {
                rightFace = std::move(face);
            }
            else if (face.contains(id3) && face.contains(id0)) {
                frontFace = std::move(face);
            }
        }

        // 分析面确定顶面的节点顺序
        if (!leftFace.empty())
        {
            getHex8IdsByFace(leftFace, id1, id0, id4, id7);
        }
        if (!rightFace.empty())
        {
            getHex8IdsByFace(rightFace, id3, id2, id6, id5);
        }
        if ((id4 == -1 || id5 == -1) && !frontFace.empty())
        {
            getHex8IdsByFace(frontFace, id0, id3, id5, id4);
        }
        if ((id6 == -1 || id7 == -1) && !backFace.empty())
        {
            getHex8IdsByFace(backFace, id2, id1, id7, id6);
        }

        // 只有三个face的单元缺一个顶点，需要从neighbour中读取
        if (id4 == -1 || id5 == -1 || id6 == -1 || id7 == -1)
        {
            return nullptr;
        }
        // OpenFOAM的face节点是按右手螺旋定则排序的
        auto hex8Cell = new Interface::FITKElementHex8;
        hex8Cell->setNodeID({ id0, id3, id2, id1, id4, id5, id6, id7 });
        return hex8Cell;
    }

    Interface::FITKAbstractElement* FITKOpenFOAMMeshReader::setupCell(int eleIndex, QList<int> faces) {
        Interface::FITKAbstractElement* cell{};

        // 单元包含面的个数
        auto facesNum = faces.size();

        // 包含每个面的点索引的面数组
        QVector<QList<int>> points{};

        // 包含每个面的点数的面数组
        QVector<int> pointNumOfFace{};

        // 遍历面
        for (auto fIndex : faces) {
            auto pointsInFace = _faces[fIndex];
            points.append(pointsInFace);

            pointNumOfFace.append(pointsInFace.size());
        }

        // 面数组里面单个面中点数的最大值和最小值
        int minPointNum = pointNumOfFace.at(0);
        int maxPointNum = minPointNum;

        for (int i = 1, n = 0; i < facesNum; ++i) {
            n = pointNumOfFace.at(i);
            if (minPointNum > n) minPointNum = n;
            if (maxPointNum < n) maxPointNum = n;
        }

        // 六面体单元(最少有3个面，最多有6个面)
        if (facesNum > 2 && facesNum < 7 && minPointNum == 4 && maxPointNum == 4) {
            cell = setupHex8Cell(eleIndex, points);
        }
        else {
            return nullptr;
        }

        return cell;
    }

    bool FITKOpenFOAMMeshReader::setupCells() {
        if (_unstructuredMesh == nullptr) return false;

        auto eleNum = _owner.size();
        for (int i = 0; i < eleNum; ++i)
        {
            auto cell = setupCell(i, _owner[i]);
            if (cell == nullptr) {
                return false;
            }
            _unstructuredMesh->appendElement(cell);
        }
        return true;
    }
}
