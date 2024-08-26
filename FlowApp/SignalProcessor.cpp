#include "SignalProcessor.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"

void SignalProcessor::on_sendProgramDriverMessageSig(AppFrame::FITKAbstractProgramerDriver* driver, int messageType, const QString& message)
{
    AppFrame::FITKMessageInfo(message);
}

