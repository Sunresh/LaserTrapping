#include <thread>
#include "depositionclass.h"
#include "preference.h"
#include <conio.h>

class MainWindow {
public:
	Pref pr;
	MainWindow() {
		pr.startscreen();
		pr.menu();
		MyDaq qq;
		char key;
		while (true) {
			key = _getch();
			if (key == 'a') {
				qq.digitalOut(nullptr, "Dev2/port0/line0", 1);
				break;
			}
			if (key == 's') {
				qq.digitalOut(nullptr, "Dev2/port0/line0", 0);
				break;
			}
			if (key == 'e' || key == 'E') {
				pr.app(PREF_FILE);
				break;
			}
			if (key == 'c') {
				Deposition diay;
				//diay.setfwidth(900);
				//diay.setfheight(500);
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
				Pref pfe;
				std::thread thh(&Pref::slowlyslowly, &pfe, LAST_VOLT_FILE);
				thh.join();
				break;
			}
		}
	};
	~MainWindow() {};
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;
	void app() {};
};