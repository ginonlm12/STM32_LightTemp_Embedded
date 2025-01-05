#include <gui/screen2_screen/Screen2View.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t myQueue01Handle;
extern osMessageQueueId_t myQueue03Handle;

int temp = 25;

Screen2View::Screen2View()
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
    updateTemp();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::updateTemp() {
	Unicode::snprintf(tempSettingTxtBuffer, TEMPSETTINGTXT_SIZE, "%d", temp);
	tempSettingTxt.invalidate();
}

void Screen2View::increaseTemp()
{
    if(temp>-30) temp++;
    updateTemp();
}


void Screen2View::decreaseTemp()
{
    if(temp<50) temp--;
    updateTemp();
}

void Screen2View::handleTickEvent()
{
	Screen2ViewBase::handleTickEvent();
	uint8_t res;
	if (osMessageQueueGetCount(myQueue01Handle) > 0){
		osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);
		if(res <= 50) {
			Unicode::snprintf(tempTxtBuffer, TEMPTXT_SIZE, "%d", res);
			tempTxt.invalidate();
			uint8_t data;
			if(temp < res) {
				data = 'F';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}else {
				data = 'f';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}
			onTxt.setVisible(temp < res);
			offTxt.setVisible(temp >= res);
			onTxt.invalidate();
			offTxt.invalidate();
		}
	}
}
