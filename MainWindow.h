#include <thread>
#include "depositionclass.h"
#include "preference.h"
#include <conio.h>

class MainWindow {
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;

public:
	Pref pr;
	MainWindow() {
		pr.startscreen();
		char key;
		while (true) {
			key = _getch();
			if (key == 'a') {

				break;
			}
			if (key == 's') {

				break;
			}
			if (key == 'e' || key == 'E') {
				pr.app(pr.getprefPath());
				break;
			}
			if (key == 'c') {
				Deposition diay;
				std::thread ddaia(&Deposition::application, &diay);
				ddaia.join();
				break;
			}
			if (key == ' ') {
				Pref pfe;
				//std::thread thh(&Pref::slowlyslowly, &pfe, LAST_VOLT_FILE);
				//thh.join();
				break;
			}
		}
	};
	~MainWindow() {};
	void app() {};

};