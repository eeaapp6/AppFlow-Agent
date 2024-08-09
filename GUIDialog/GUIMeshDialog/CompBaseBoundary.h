#ifndef _CompBaseBoundary_H
#define _CompBaseBoundary_H

#include "GUIMeshDialogAPI.h"
#include <QLabel>
#include <QComboBox>

namespace GUI
{
   class GUIMeshDialogAPI CompBaseBoundaryLabel : public QLabel
   {
       Q_OBJECT;
   public:
       CompBaseBoundaryLabel(QWidget* parent = nullptr);
       virtual ~CompBaseBoundaryLabel();

       void setPos(int pos);
       int getPos();
   signals:
       ;
       void sigMouseMove();
   protected:
       void mouseMoveEvent(QMouseEvent *event) override;
   private:
       int _pos = -1;
   };

   class GUIMeshDialogAPI CompBaseBoundaryComboBox : public QComboBox
   {
       Q_OBJECT;
   public:
       CompBaseBoundaryComboBox(QWidget* parent = nullptr);
       virtual ~CompBaseBoundaryComboBox();

       void setPos(int pos);
       int getPos();
   signals:
       ;
       void sigMouseMove();
   protected:
       void mouseMoveEvent(QMouseEvent *event) override;
   private:
       int _pos = -1;
   };
}

#endif
