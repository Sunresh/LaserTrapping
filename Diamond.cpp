#include "Diamond.h"
#include "preference.h"
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <conio.h>
#include "only.cpp"

Diamond::Diamond() { 
}

Diamond::~Diamond() {
}

void Diamond::application() {
	system("cls");
	std::vector<UserPreferences> storedPreferences = pr.readFromCSV(filename);

	// Display stored preferences
	for (const UserPreferences& prefs : storedPreferences) {
		std::cout << "\t\t" << "PZT Voltage: " << prefs.voltage << "\n";
		std::cout << "\t\t" << "Pixel Contrast: " << prefs.threshold << "\n";
		std::cout << "\t\t" << "Time for velocity: " << prefs.time << "\n";
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
			cout <<"\t\t" << i << endl;
		}
		else {
			cout << "\t" << "Press " << i << endl;
		}
	}
	char key;
	char a = 'a';
	
	while (true) {
		key = _getch();
		if (key == 'e' || key == 'E') {
			pr.getPrefToCSV(filename);
		}
		if (key == 'x' || key == 'X') {
			Depositi dia;
			std::thread ddaa(&Depositi::cAndD, &dia);
			ddaa.join();
		}

		if (key == 'd' || key == 'D') {
			Depositi dia;
			std::thread ddaa(&Depositi::dodeposition, &dia);
			ddaa.join();
		}
		if (key == 'c' || key == 'C') {
			Depositi tdd;
			std::thread threadA(&Depositi::camra, &tdd);
			threadA.join();
		}
		if (key == 'b' || key == 'B') {
			Depositi tdd;
			Depositi d;
			std::thread threadA(&Depositi::camra, &tdd);
			std::thread threadDeposition(&Depositi::dodeposition, &d);
			threadA.join();
			threadDeposition.join();

		}
		else if (key == 'i' || key == 'I') {
			//	MainWindow min;
			//	std::thread mui(&MainWindow::checking, &min);
			//	mui.join();
		}
		else {
			std::cout << "Exiting program." << std::endl;
			break;
		}
	}

}
