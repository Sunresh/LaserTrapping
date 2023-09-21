#include <thread>
#include "depositionclass.h"
#include "preference.h"
#include <conio.h>
#include "MainWindow.h"

MainWindow::MainWindow() {

	pr.startscreen();
	pr.menu();
	char key;
	while (true) {
		key = _getch();
		if (key == 'e' || key == 'E') {
			pr.app(PREF_FILE);
			break;
		}
		if (key == 'c') {
			Deposition diay;
			std::thread ddaia(&Deposition::camera, &diay);
			ddaia.join();
			break;
		}
		if (key == 'k') {
			Deposition dia;
			std::thread ddaa(&Deposition::application, &dia);
			ddaa.join();
			break;
		}
		if (key == ' ') {
			const char* dev0 = "Dev2/ao0";
			const char* dev1 = "Dev2/ao1";
			double v1 = 0.0;
			TaskHandle task1 = nullptr;
			TaskHandle task2 = nullptr;
			DAQmxCreateTask("", &task1);
			DAQmxCreateTask("", &task2);
			DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
			DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
			DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
			for (double i = 1; i < VOLTAGE;i++) {
				v1 -= VOLTAGE / i;
				DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
			}
			DAQmxClearTask(task1);
			DAQmxClearTask(task2);
			break;
		}
	}
}
void MainWindow::app() {};

MainWindow::~MainWindow() {

}