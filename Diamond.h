#ifndef DIAMOND_H
#define DIAMOND_H

#include <string>
#include <vector>
#include <iostream>
#include "preference.h"

using namespace std;

class Diamond {
public:
	static const std::string filename;
	Pref pr;
	Diamond();
	~Diamond();
	void initialSetup();
	virtual void app();
};

#endif // DIAMOND_H
