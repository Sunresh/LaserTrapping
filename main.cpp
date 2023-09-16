#include <thread>
#include "MainWindow.h"

int main() {
	while(true) {
		MainWindow dia;
		std::thread ddaa(&MainWindow::app, &dia);
		ddaa.join();
	};
    return 0;
}
