#ifndef _SIGNAL_PROCESSOR_H___
#define _SIGNAL_PROCESSOR_H___
 
#include <QObject>

namespace AppFrame
{
    class FITKAbstractProgramerDriver;
}

class SignalProcessor : public QObject
{
    Q_OBJECT
public:
    explicit SignalProcessor() = default;
    virtual ~SignalProcessor() = default;


protected slots:
    void on_sendProgramDriverMessageSig(AppFrame::FITKAbstractProgramerDriver* driver, int messageType, const QString& message);

};

#endif
