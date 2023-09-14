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
class Pref{
public:
	std::vector<UserPreferences> readFromCSV(const std::string& filename);
	bool isNumeric(const std::string& str);
	void getUserInput(const std::string& fieldName, int& field);
	void getUserInput(const std::string& fieldName, double& field);
	void saveCSV(const std::string& filename, const UserPreferences& userPrefs);
	bool loadCSV(const std::string& filename, UserPreferences& userPrefs);
	void app(const std::string& filename);

};

#endif // PREFERENCE_H
