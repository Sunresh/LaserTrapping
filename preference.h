#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <string>
#include <vector>

struct UserPreferences {
	std::string name;
	double left;
	double top;
	double threshold;
	int voltage;
	int time;
	std::string favoriteColor;
};

void getPrefToCSV(const std::string& filename);
std::vector<UserPreferences> readFromCSV(const std::string& filename);
bool isNumeric(const std::string& str);



#endif // PREFERENCE_H
