#include <QTest>
#include <QApplication>
#include <iostream>
#include "TestOFMeshReader.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
#if true

    QString filepath{};
    if (argc > 1) filepath = QString::fromLatin1(argv[1]);
    else {
        std::string str;
        while (str.length() == 0)
        {
            printf("Enter the full path to the polyMesh folder: \n");
            std::getline(std::cin, str);
        }
        filepath = QString::fromStdString(str);
    }

    auto test = new FITKTest::TestOFMeshReader(filepath);
    test->testRun();
#else
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
#endif // 
    return app.exec();

}
