#ifndef CompMaterialPointWidget_H
#define CompMaterialPointWidget_H

#include <QWidget>

namespace Ui {
    class CompMaterialPointWidget;
}

namespace GUI
{
    class CompMaterialPointWidget :public QWidget
    {
        Q_OBJECT;
    public:
        CompMaterialPointWidget(QWidget* parent);
        ~CompMaterialPointWidget();

        void init();
        void setName(QString name);
        QString getName();
        void getPoint(double* point);
        void setPoint(double* point);

        void setData(int pos, QVariant value);
        QVariant data(int pos);
        void setCurrentPos(int row, int clo);
        QPair<int, int> getCurrentPos();
    signals:
        ;
        void sigDeleteClicked();
        void sigPointChange();

    private:
        Ui::CompMaterialPointWidget* _ui = nullptr;
        QHash<int, QVariant> _data;
        QPair<int, int> _currentPos;
    };
}

#endif
