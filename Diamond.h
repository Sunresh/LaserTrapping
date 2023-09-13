#ifndef DIAMOND_H
#define DIAMOND_H

#include <string>
#include <vector>
#include <iostream>
#include "preference.h"

using namespace std;

class Diamond {
private:
	const std::string filename = "diamond.csv";
	Pref pr;
public:
	Diamond();
	~Diamond();
	const std::string str[2][2] = { {"evolt", "Enter the Electrophoretic voltage:"}, {"pvolt", "Enter the PZT voltage:"} };
	void application();
};

#endif // DIAMOND_H
