#include <nidaqmx.h>
#include <iostream>

using namespace std;
class MyDaq {
private:
	bool isStart;
	
public:
	MyDaq() : isStart(true){
	
	}
	void start(TaskHandle task1= nullptr, const char dev0[64]="dev0",const double voltage = 0) {
		DAQmxCreateTask("", &task1);
		DAQmxCreateAOVoltageChan(task1, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
		DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
		DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
		DAQmxClearTask(task1);
	}
	void digitalOut(TaskHandle task= nullptr, const char* digital = "Dev2/port0/line0", bool boolean= true) {
		int32 error = 0;
		if (task == nullptr) {
			DAQmxCreateTask("", &task);
		}
		DAQmxCreateDOChan(task, digital, "", DAQmx_Val_ChanForAllLines);
		uInt32 data = boolean ? 1 : 0;
		DAQmxWriteDigitalU32(task, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &data, nullptr, nullptr);
		DAQmxStopTask(task);
		DAQmxClearTask(task);
		if (DAQmxFailed(error)) {
			char errBuff[2048] = { '\0' };
			DAQmxGetExtendedErrorInfo(errBuff, 2048);
			printf("DAQmx Error: %s\n", errBuff);
			if (task != nullptr) {
				DAQmxStopTask(task);
				DAQmxClearTask(task);
			}
		}
	}

};