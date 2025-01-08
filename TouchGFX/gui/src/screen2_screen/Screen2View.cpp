#include <gui/screen2_screen/Screen2View.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t myQueue01Handle;
extern osMessageQueueId_t myQueue03Handle;

extern int init_temp;

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
	// cập nhật lại màn hình hiển thị nhiệt độ
	Unicode::snprintf(tempSettingTxtBuffer, TEMPSETTINGTXT_SIZE, "%d", init_temp);
	tempSettingTxt.invalidate();
}

void Screen2View::increaseTemp()
{
	// nếu nhiệt độ quá thấp thì không cho thay đổi nhiệt độ nữa
    if(init_temp>-30) init_temp++;
    updateTemp();
}


void Screen2View::decreaseTemp()
{
	// nếu nhiệt độ quá cao thì không cho giảm nữa
    if(init_temp<50) init_temp--;
    updateTemp();
}

void Screen2View::handleTickEvent()
{
	Screen2ViewBase::handleTickEvent();
	uint8_t res;
	if (osMessageQueueGetCount(myQueue01Handle) > 0){
		// nhận dữ liệu ở cảm biến ở queue 1
		osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);
		if(res <= 50) {
			Unicode::snprintf(tempTxtBuffer, TEMPTXT_SIZE, "%d", res);
			tempTxt.invalidate();
			uint8_t data;
			// so sánh nhiệt độ hiện tạo với nhiệt độ
			if(init_temp < res) {
				data = 'F';
				// gửi tín hiệu để bật quạt
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}else {
				data = 'f';
				// gửi tín hiệu tắt quạt
				osMessageQueuePut(myQueue03Handle, &data, 0, 10);
			}
			// điều chỉnh lại trạng thái của quạt
			onTxt.setVisible(init_temp < res);
			offTxt.setVisible(init_temp >= res);
			onTxt.invalidate();
			offTxt.invalidate();
		}
	}
}
