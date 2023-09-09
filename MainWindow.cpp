#include "Diamond.h"
#include <thread>
#include "depositionclass.h"
#include "preference.h"
#include <conio.h>
#include "cameraonly.h"   

class MainWindow {
public:
	const std::string filename = "preference.csv";
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;
	std::vector<UserPreferences> storedPreferences = readFromCSV(filename);
	MainWindow() {
		system("cls");
		for (const UserPreferences& prefs : storedPreferences) {
			std::cout << "\t\t" << std::string(28, '#') << std::endl;
			std::cout << "\t\t" << "#" << std::string(26, ' ') << "#" << std::endl;
			std::cout << "\t\t" << "#  PZT Voltage: " << prefs.voltage << std::string(9, ' ') << " #" << std::endl;
			std::cout << "\t\t" << "#  Pixel Contrast: " << prefs.threshold << std::string(4, ' ') << " #" << std::endl;
			std::cout << "\t\t" << "#  Time for velocity: " << prefs.time << std::string(3, ' ') << " #" << std::endl;
			std::cout << "\t\t" << "#  left: " << prefs.left << std::string(13, ' ') << " #" << std::endl;
			std::cout << "\t\t" << "#  top: " << prefs.top << std::string(13, ' ') << " #" << std::endl;
			std::cout << "\t\t" << "#" << std::string(26, ' ') << "#" << std::endl;
			std::cout << "\t\t" << std::string(28, '#') << std::endl;

		}
		for (const UserPreferences& prefs : storedPreferences) {
			pztvolt = prefs.voltage;
			thContrast = prefs.threshold;
			durationInSeconds = prefs.time;
		}
		string str[] = {
			" ",
			"'e' for edit, you want to change the above parameter.",
			" ",
			"c to start ccd camera.",
			"m to start deposition.",
			"b to start camera and deposition.",
			" ",
		};
		for (string i : str) {
			if (i.size() < 6) {
				cout << "\t\t\t" << i << endl;
			}
			else {
				cout << "\t\t" << "Press " << i << endl;
			}
		}
		char key;
		while (true) {
			key = _getch();
			if (key == 'e' || key == 'E') {
				getPrefToCSV(filename);
				break;
			}
			if (key == 'c') {
				CameraOnly dia;
				std::thread ddaa(&CameraOnly::application, &dia);
				ddaa.join();
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
				DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
				DAQmxClearTask(task1);
				DAQmxClearTask(task2);
				break;
			}
		}
	}
	void application() {};
};
