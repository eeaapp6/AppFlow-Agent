#ifndef FaceGroupWidget_H
#define FaceGroupWidget_H

#include <QWidget>

namespace Ui {
    class FaceGroupWidget;
}

namespace GUI
{
    class FaceGroupWidget :public QWidget
    {
        Q_OBJECT;
    public:
        FaceGroupWidget(QWidget* parent);
        ~FaceGroupWidget();

        void init();

        void setName(QString name);
        QString getName();

        void setData(int pos, QVariant value);
        QVariant data(int pos);

        void setSelect(bool type);

        void setCurrentPos(int row, int clo);
        QPair<int, int> getCurrentPos();
    signals:
        ;
        void sigOkClicked();
        void sigCancelClicked();
        void sigDeleteClicked();
    private:
        Ui::FaceGroupWidget* _ui = nullptr;
        QHash<int, QVariant> _data;
        QPair<int, int> _currentPos;
    };
}

#endif
