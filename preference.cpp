#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "preference.h"
#include <conio.h>

using namespace std;

bool Pref::isNumeric(const std::string& str) {
	for (char c : str) {
		if (!std::isdigit(c) && c != '.' && c != '-') {
			return false;
		}
	}
	return true;
}

void Pref::getUserInput(const std::string& fieldName, int& field) {
	std::string input;
	std::cout << "Enter " << fieldName << ": ";
	std::cin >> input;

	if (!isNumeric(input)) {
		std::cerr << "Error: " << fieldName << " must be a numeric value." << std::endl;
		return;
	}
	field = std::stoi(input);
}
void Pref::getUserInput(const std::string& fieldName, double& field) {
	std::string input;
	std::cout << "Enter " << fieldName << ": ";
	std::cin >> input;

	if (!isNumeric(input)) {
		std::cerr << "Error: " << fieldName << " must be a numeric value." << std::endl;
		return;
	}
	field = std::stod(input);
}

void Pref::getPrefToCSV(const std::string& filename) {
	UserPreferences userPrefs;
	std::string input;

	std::cout << "Enter electrophoretic voltage: ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: Voltage must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.voltage = std::stoi(input);
	}

	std::cout << "Enter threshold value for laser spot contrast: ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: Threshold value for laser spot contrast must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.threshold = std::stod(input);
	}

	std::cout << "Enter time of deposition: ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: time of deposition must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.time = std::stoi(input);
	}

	std::cout << "Enter spot from left (default:358): ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: time of deposition must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.left = std::stoi(input);
	}

	std::cout << "Enter spot from Top  (default:238): ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: time of deposition must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.top = std::stoi(input);
	}


	// Open the CSV file for writing
	//std::ofstream outFile(filename, std::ios::app); // 'app' for appending
	// Open the CSV file for writing, using "trunc" mode to create a new file
	std::ofstream outFile(filename, std::ios::trunc);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	// Write the preferences to the CSV file
	outFile
		<< userPrefs.voltage
		<< ","
		<< userPrefs.threshold
		<< ","
		<< userPrefs.time
		<< ","
		<< userPrefs.left
		<< ","
		<< userPrefs.top
		<< "\n";
	outFile.close();
}

std::vector<UserPreferences> Pref::readFromCSV(const std::string& filename) {
	try{
		std::vector<UserPreferences> preferences;
		std::ifstream inFile(filename);
		if (!inFile.is_open()) {
			std::cerr << "Error opening file for reading." << std::endl;
			getPrefToCSV(filename);
			return preferences;
		}
		std::string line;
		while (std::getline(inFile, line)) {
			std::istringstream iss(line);
			UserPreferences userPrefs;
			std::string volt, th, timed,left,top;
			if (std::getline(iss, volt, ',') &&
				std::getline(iss, th, ',') &&
				std::getline(iss, timed, ',') &&
				std::getline(iss, left, ',') &&
				std::getline(iss, top, ',')) {
				// Convert ageStr to an integer
				try {
					userPrefs.voltage = std::stoi(volt);
					userPrefs.threshold = std::stod(th);
					userPrefs.time = std::stoi(timed); 
					userPrefs.left = std::stod(left);
					userPrefs.top = std::stoi(top);
				}
				catch (const std::invalid_argument& e) {
					std::cerr << "Error converting age to an integer: " << e.what() << std::endl;
					continue;
				}
				preferences.push_back(userPrefs);
			}
		}
		inFile.close();
		return preferences;
		}
	catch (int a) {
	
	};
}

void Pref::saveCSV(const std::string& filename, const UserPreferences& userPrefs) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}

	// Write the preferences to the file with commas
	outFile
		<< userPrefs.voltage << ","
		<< userPrefs.threshold << ","
		<< userPrefs.time << ","
		<< userPrefs.left << ","
		<< userPrefs.top;
	outFile.close();
}

bool Pref::loadCSV(const std::string& filename, UserPreferences& userPrefs) {
	std::ifstream inFile(filename);
	if (!inFile.is_open()) {
		return false; // File doesn't exist or cannot be opened; use default values
	}

	std::string line;
	std::getline(inFile, line);

	// Split the line into values using commas
	std::istringstream ss(line);
	std::string value;
	getline(ss, value, ',');
	userPrefs.voltage = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.threshold = std::stod(value);
	getline(ss, value, ',');
	userPrefs.time = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.left = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.top = std::stoi(value);

	inFile.close();
	return true;
}

void Pref::app(const std::string& filename) {
	UserPreferences userPrefs;
	// Load preferences from a file or create it with default values
	if (!loadCSV(filename, userPrefs)) {
		std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
	}
	else {
		loadCSV(filename, userPrefs);
	}

	char key;
	while (true) {
		system("cls");

		std::cout << "\t\t" << std::string(28, '#') << std::endl;
		std::cout << "\t\t" << "#" << std::string(26, ' ') << "#" << std::endl;
		std::cout << "\t\t" << "#  PZT Voltage: " << userPrefs.voltage << std::string(9, ' ') << " #" << std::endl;
		std::cout << "\t\t" << "#  Pixel Contrast: " << userPrefs.threshold << std::string(4, ' ') << " #" << std::endl;
		std::cout << "\t\t" << "#  Time for velocity: " << userPrefs.time << std::string(3, ' ') << " #" << std::endl;
		std::cout << "\t\t" << "#  left: " << userPrefs.left << std::string(13, ' ') << " #" << std::endl;
		std::cout << "\t\t" << "#  top: " << userPrefs.top << std::string(13, ' ') << " #" << std::endl;
		std::cout << "\t\t" << "#" << std::string(26, ' ') << "#" << std::endl;
		std::cout << "\t\t" << std::string(28, '#') << std::endl;

		

		std::cout << "'z' to set Voltage" << std::endl;
		std::cout << "'x' for Threshold" << std::endl;
		std::cout << "'c' for Time" << std::endl;
		std::cout << "'v' for Left" << std::endl;
		std::cout << "'b' for Top" << std::endl;
		key = _getch(); // Use getchar to read a character
		switch (key) {
		case 'z':
			getUserInput("electrophoretic voltage", userPrefs.voltage);
			break;
		case 'x':
			getUserInput("threshold value for laser spot contrast", userPrefs.threshold);
			break;
		case 'c':
			getUserInput("time of deposition", userPrefs.time);
			break;
		case 'v':
			getUserInput("spot from left", userPrefs.left);
			break;
		case 'b':
			getUserInput("spot from top", userPrefs.top);
			break;
		default:
			continue; // Continue the loop for unknown keys
		}

		saveCSV(filename, userPrefs);
		break;
	}
}
