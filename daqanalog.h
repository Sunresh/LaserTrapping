#include <nidaqmx.h>
#include <iostream>
using namespace std;
class MyDaq 

{
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
	void digitalOut(bool boolean, const char* digital) {//port0/line0 for shuttur
		TaskHandle taskHandle = 0;
		int32 error = 0;
		DAQmxCreateTask("", &taskHandle);
		DAQmxCreateDOChan(taskHandle, digital, "", DAQmx_Val_ChanForAllLines);
		DAQmxWriteDigitalScalarU32(taskHandle, true, 10.0, boolean, nullptr);
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}

};