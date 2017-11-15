#include "ManagedELInterop.h"

using namespace System; 
using namespace System::Threading;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

public delegate void PostCommandDelegate(Button^ btn, String^ cmd);

void PostCommand(Button^ btn, String^ cmd)
{
    btn->Text = cmd;
    btn->PerformClick();
}

void ManagedELInterop::SendCommand(const TCHAR* command, int cmd_handle)
{
    String^ mSettings = gcnew String(command);
    Button^ btn = (Button^)Control::FromHandle((IntPtr)cmd_handle);

    array<Object^>^ params = gcnew array<Object^>(2);
    params[0] = btn;
    params[1] = mSettings;
    
	btn->BeginInvoke(gcnew PostCommandDelegate(PostCommand), params);
}

