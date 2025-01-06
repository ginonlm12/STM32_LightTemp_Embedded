#include <gui/screen3_screen/Screen3View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t myQueue02Handle;
extern osMessageQueueId_t myQueue03Handle;
//extern int lux;
int lux = 50;
bool toggle = true;
int currentHour = 7;
int currentMinute = 30;
int currentSecond = 0;
uint32_t initial_seconds = currentHour * 3600 + currentMinute * 60;


Screen3View::Screen3View()
{
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();
    updateLux();
    settingMode.forceState(toggle);
    settingMode.invalidate();
    updateView(!toggle);
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}


void Screen3View::updateLux() {
	Unicode::snprintf(luxSettingTxtBuffer, LUXSETTINGTXT_SIZE, "%d", lux);
	luxSettingTxt.invalidate();
}

void Screen3View::increaseLux()
{
    if(lux<300) lux++;
    updateLux();
}


void Screen3View::decreaseLux()
{
    if(lux>=0) lux--;
    updateLux();
}

void Screen3View::showSetting()
{
   updateView(toggle);
   toggle = !toggle;
}

void Screen3View::updateTime() {
	Unicode::snprintf(timeTxtBuffer, TIMETXT_SIZE, "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);
	timeTxt.invalidate();
}

void Screen3View::updateView(bool isToggle)
{
   increaseBtn.setVisible(isToggle);
   decreaseBtn.setVisible(isToggle);
   luxSettingTxt.setVisible(isToggle);
   increaseBtn.invalidate();
   decreaseBtn.invalidate();
   luxSettingTxt.invalidate();
}

void Screen3View::handleTickEvent()
{
	Screen3ViewBase::handleTickEvent();
	uint8_t res;
	if (osMessageQueueGetCount(myQueue02Handle) > 0){
		osMessageQueueGet(myQueue02Handle, &res, NULL, osWaitForever);
		Unicode::snprintf(luxTxtBuffer, LUXTXT_SIZE, "%d", res);
		luxTxt.invalidate();
		uint8_t data;
		if(toggle) {
			if(currentHour > 5 && currentHour < 18) {
				data = 'l';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}else {
				data = 'L';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}
			onTxt.setVisible(!(currentHour > 5 && currentHour < 18));
			offTxt.setVisible(currentHour > 5 && currentHour < 18);
			onTxt.invalidate();
			offTxt.invalidate();
		}else {
			if(lux > res) {
				data = 'L';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}else {
				data = 'l';
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}
			onTxt.setVisible(lux > res);
			offTxt.setVisible(lux <= res);
			onTxt.invalidate();
			offTxt.invalidate();
		}
	}
	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_freq = 1000;
	uint32_t total_seconds =  initial_seconds + tick_count / tick_freq;
	currentSecond = total_seconds % 60;
	currentMinute = (total_seconds / 60) % 60;
	currentHour = (total_seconds / 3600) % 24;
	updateTime();
}
