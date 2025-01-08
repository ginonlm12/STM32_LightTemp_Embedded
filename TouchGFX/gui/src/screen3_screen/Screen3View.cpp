#include <gui/screen3_screen/Screen3View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t myQueue02Handle;
extern osMessageQueueId_t myQueue03Handle;
//extern int lux;
extern int init_lux;
extern int init_toggle;
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
    settingMode.forceState(init_toggle);
    settingMode.invalidate();
    updateView(!init_toggle);
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}


void Screen3View::updateLux() {
	Unicode::snprintf(luxSettingTxtBuffer, LUXSETTINGTXT_SIZE, "%d", init_lux);
	luxSettingTxt.invalidate();
}

void Screen3View::increaseLux()
{
    if(init_lux<300) init_lux++;
    updateLux();
}


void Screen3View::decreaseLux()
{
    if(init_lux>=0) init_lux--;
    updateLux();
}

void Screen3View::showSetting()
{
   updateView(init_toggle);
   init_toggle = !init_toggle;
}

void Screen3View::updateTime() {
	// cập nhật lại thời gian hiện thị trên màn hình
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
		// nhận giá trị ánh sáng từ queue2
		osMessageQueueGet(myQueue02Handle, &res, NULL, osWaitForever);
		Unicode::snprintf(luxTxtBuffer, LUXTXT_SIZE, "%d", res);
		luxTxt.invalidate();
		uint8_t data;
		if(init_toggle) {
			// kiểm tra nếu setting theo chế dộ mặc định
			// nếu bây giờ là từ 5h sáng đến 17h thì tắt đèn
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
			// kiểm tra mức sáng hiện tại với mức sáng setting
			if(init_lux > res) {
				data = 'L';
				// gửi tín hiệu bật LED
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}else {
				data = 'l';
				// gửi tín hiệu tắt LED
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}
			//cập nhật lại trạng thái led trên màn hình
			onTxt.setVisible(init_lux > res);
			offTxt.setVisible(init_lux <= res);
			onTxt.invalidate();
			offTxt.invalidate();
		}
	}
	//lấy tổng giây thời gian từ lúc hoạt động đến giờ để cập nhật lại thời gian
	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_freq = 1000;
	uint32_t total_seconds =  initial_seconds + tick_count / tick_freq;
	currentSecond = total_seconds % 60;
	currentMinute = (total_seconds / 60) % 60;
	currentHour = (total_seconds / 3600) % 24;
	updateTime();
}
