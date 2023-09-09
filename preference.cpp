#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "preference.h" 
using namespace std;

bool isNumeric(const std::string& str) {
	for (char c : str) {
		if (!std::isdigit(c) && c != '.' && c != '-') {
			return false;
		}
	}
	return true;
}

void updateDepositionTime(UserPreferences& userPrefs) {
	std::string input;

	std::cout << "Enter new time of deposition: ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: Time of deposition must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.time = std::stoi(input);
	}
}

void saveCSV(const std::string& filename, const std::string& itemName, const std::string& itemValue) {
	UserPreferences userPrefs;
	std::string input;

	std::vector<UserPreferences> storedP = readFromCSV(filename);
	for (const UserPreferences& prefs : storedP) {
		userPrefs.threshold = prefs.voltage;
	}

	std::cout << itemName;
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: Voltage must be a numeric value." << std::endl;
		return;
	}
	else {
		userPrefs.itemValue = std::stoi(input);
	}

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
void getPrefToCSV(const std::string& filename) {
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

std::vector<UserPreferences> readFromCSV(const std::string& filename) {
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
