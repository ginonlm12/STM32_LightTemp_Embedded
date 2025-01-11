#include <gui/screen3_screen/Screen3View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "cmsis_os.h"
#include <cstdio>


extern osMessageQueueId_t myQueue02Handle;
extern osMessageQueueId_t myQueue03Handle;
extern int init_lux;
extern int init_toggle;

int currentHour = 7;
int currentMinute = 30;
int currentSecond = 0;
uint32_t initial_seconds = currentHour * 3600 + currentMinute * 60;

bool isLightOn = false; // Biến theo dõi trạng thái đèn LED

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

void Screen3View::updateLux()
{
    Unicode::snprintf(luxSettingTxtBuffer, LUXSETTINGTXT_SIZE, "%d", init_lux);
    luxSettingTxt.invalidate();
}

void Screen3View::increaseLux()
{
    if (init_lux < 300) init_lux++;
    updateLux();
}

void Screen3View::decreaseLux()
{
    if (init_lux >= 0) init_lux--;
    updateLux();
}

void Screen3View::showSetting()
{
    updateView(init_toggle);
    init_toggle = !init_toggle; // Đảo giá trị của init_toggle
}

void Screen3View::updateTime()
{
    // Cập nhật lại thời gian hiện thị trên màn hình
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

    if (osMessageQueueGetCount(myQueue02Handle) > 0) {
        // Nhận giá trị ánh sáng từ queue2
        osMessageQueueGet(myQueue02Handle, &res, NULL, osWaitForever);
        Unicode::snprintf(luxTxtBuffer, LUXTXT_SIZE, "%d", res);
        luxTxt.invalidate();

        uint8_t data;

        printf("init_toggle: %d\n", init_toggle);
        if (!init_toggle) {
            // Chế độ thủ công: xử lý với nút bật/tắt
            onOffBtn.setVisible(true); // Hiển thị nút bật/tắt
            increaseBtn.setVisible(false); // Ẩn nút tăng
            decreaseBtn.setVisible(false); // Ẩn nút giảm
            luxSettingTxt.setVisible(false); // Ẩn text setting

            onOffBtn.invalidate();
            increaseBtn.invalidate();
            decreaseBtn.invalidate();
            luxSettingTxt.invalidate();

            // Logic xử lý nút onOffBtn
            if (onOffBtn.getState()) {
                if (!isLightOn) {
                    data = 'L'; // Bật đèn
                    osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                    isLightOn = true;
                }
            } else {
                if (isLightOn) {
                    data = 'l'; // Tắt đèn
                    osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                    isLightOn = false;
                }
            }

            // Cập nhật trạng thái LED trên giao diện
            onTxt.setVisible(isLightOn);
            offTxt.setVisible(!isLightOn);
            onTxt.invalidate();
            offTxt.invalidate();
        } else {
            // Chế độ tự động: Kiểm tra mức sáng hiện tại với mức sáng cài đặt
            onOffBtn.setVisible(false); // Ẩn nút bật/tắt
            increaseBtn.setVisible(true); // Hiển thị nút tăng
            decreaseBtn.setVisible(true); // Hiển thị nút giảm
            luxSettingTxt.setVisible(true); // Hiển thị text setting
            onOffBtn.invalidate();
            increaseBtn.invalidate();
            decreaseBtn.invalidate();
            luxSettingTxt.invalidate();

            // Logic kiểm tra mức sáng
            if (!isLightOn && res <= init_lux - 4) {
                // Bật đèn nếu ánh sáng thấp hơn mức cài đặt ít nhất 4 đơn vị
                data = 'L';
                osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                isLightOn = true;
            } else if (isLightOn && res >= init_lux + 4) {
                // Tắt đèn nếu ánh sáng cao hơn hoặc bằng mức cài đặt cộng 4 đơn vị
                data = 'l';
                osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                isLightOn = false;
            }

            // Cập nhật trạng thái LED trên giao diện
            onTxt.setVisible(isLightOn);
            offTxt.setVisible(!isLightOn);
            onTxt.invalidate();
            offTxt.invalidate();

        }
    }

    // Lấy tổng giây thời gian từ lúc hoạt động đến giờ để cập nhật lại thời gian
    uint32_t tick_count = osKernelGetTickCount();
    uint32_t tick_freq = 1000;
    uint32_t total_seconds = initial_seconds + tick_count / tick_freq;
    currentSecond = total_seconds % 60;
    currentMinute = (total_seconds / 60) % 60;
    currentHour = (total_seconds / 3600) % 24;
    updateTime();
}


