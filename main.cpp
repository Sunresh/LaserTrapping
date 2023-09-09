#include <thread>
#include "MainWindow.cpp"

int main() {
	while(true) {
		MainWindow dia;
		std::thread ddaa(&MainWindow::application, &dia);
		ddaa.join();
	};
    return 0;
}
