#ifndef CompFaceGroupWidget_H
#define CompFaceGroupWidget_H

#include <QWidget>
#include <QLineEdit>

namespace Ui {
    class CompFaceGroupWidget;
}

namespace GUI
{
    class CompFaceGroupWidget :public QWidget
    {
        Q_OBJECT;
    public:
        CompFaceGroupWidget(QWidget* parent);
        ~CompFaceGroupWidget();

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
        void sigEditNameStart();
        void sigEditNameFinish();
        void sigOkClicked();
        void sigCancelClicked();
        void sigDeleteClicked();
    private:
        Ui::CompFaceGroupWidget* _ui = nullptr;
        QHash<int, QVariant> _data;
        QPair<int, int> _currentPos;
    };
}

#endif
