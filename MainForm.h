#pragma once

#include "ResourceMonitor.h"

namespace ResourceMonitor {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MainForm : public System::Windows::Forms::Form {
	private:
		Label^ cpuLabel;
		Label^ gpuLabel;
		Label^ memLabel;
		Label^ vmemLabel;
		Timer^ updateTimer;
		ResourceMonitor1* monitor;

	public:
		MainForm() {
			InitializeComponent();
			monitor = new ResourceMonitor1();	
			updateTimer = gcnew Timer();
			updateTimer->Interval = 1000; // 1 секунда
			updateTimer->Tick += gcnew EventHandler(this, &MainForm::UpdateData);
			updateTimer->Start();
		}

	protected:
		~MainForm() {
			if (components) {
				delete components;
			}
			if (monitor) {
				delete monitor;
				monitor = nullptr;
			}
		}

	private:
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code

		void InitializeComponent() {
			this->components = gcnew System::ComponentModel::Container();
			this->cpuLabel = gcnew Label();
			this->gpuLabel = gcnew Label();
			this->memLabel = gcnew Label();
			this->vmemLabel = gcnew Label();

			this->SuspendLayout();

			// CPU Label
			this->cpuLabel->Location = Point(10, 10);
			this->cpuLabel->Size = Drawing::Size(80, 20);
			this->cpuLabel->Text = "CPU: ";

			// GPU Label
			this->gpuLabel->Location = Point(10, 50);
			this->gpuLabel->Size = Drawing::Size(90, 20);
			this->gpuLabel->Text = "GPU: ";

			// MEM Label
			this->memLabel->Location = Point(10, 30);
			this->memLabel->Size = Drawing::Size(90, 20);
			this->memLabel->Text = "MEM: ";

			// VMEM
			this->vmemLabel->Location = Point(10, 70);
			this->vmemLabel->Size = Drawing::Size(110, 20);
			this->vmemLabel->Text = "VMEM: ";

			// Form
			this->Controls->Add(this->cpuLabel);
			this->Controls->Add(this->gpuLabel);
			this->Controls->Add(this->memLabel);
			this->Controls->Add(this->vmemLabel);
			this->Text = "Monitor741";
			this->Size = Drawing::Size(300, 200);
			this->ClientSize = Drawing::Size(110, 95);
			//this->BackColor = Drawing::Color::Transparent;
			this->TopMost = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->Font = (gcnew System::Drawing::Font(L"Arial", 9.0F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));

			this->ResumeLayout(false);
		}
#pragma endregion

		void UpdateData(Object^ sender, EventArgs^ e) {
			cpuLabel->Text = "CPU: " + Math::Round(monitor->GetCPUUsage()) + " %";
			gpuLabel->Text = "GPU: " + Math::Round(monitor->GetGPUUsage()) + " %";
			memLabel->Text = "MEM: " + Math::Round(monitor->GetMemoryUsage()) + " %";
			//vmemLabel->Text = "VMEM: " + Math::Round(monitor->GetVideoMemoryUsage()) + " MB";
			vmemLabel->Text = "VMEM: " + Math::Round(monitor->GetTotalBytesReceived()) + " MB";
		}

	};
}
