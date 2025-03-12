#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

//[STAThread]
[STAThreadAttribute]
int main(array<String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	ResourceMonitor::MainForm mainForm;
	Application::Run(% mainForm);
	return 0;
}
