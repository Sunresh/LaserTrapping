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
}
void MainWindow::app() {};

MainWindow::~MainWindow() {

}
