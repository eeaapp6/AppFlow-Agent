#include <QTest>
#include <QApplication>
#include "TestOFMeshReader.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    //待测试的类对象
    QList<QObject*> testClass;

    testClass << new FITKTest::TestOFMeshReader;


    //执行测试，将会测试实例中的全部槽函数
    for (QObject* testObj : testClass)
    {
        if (testObj == nullptr) continue;

        QTest::qExec(testObj, argc, argv);
        delete testObj;
    }

    return app.exec();

}
