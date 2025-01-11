#include <gui/screen2_screen/Screen2View.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t myQueue01Handle;
extern osMessageQueueId_t myQueue03Handle;

extern int init_temp;

bool isFanOn; // Không khởi tạo giá trị mặc định ở đây

Screen2View::Screen2View()
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    // Đồng bộ trạng thái quạt ban đầu
    uint8_t fanState;
    if (osMessageQueueGetCount(myQueue03Handle) > 0) {
        // Giả sử trạng thái ban đầu của quạt được truyền qua queue 3
        osMessageQueueGet(myQueue03Handle, &fanState, NULL, 0);
        isFanOn = (fanState == 'F'); // Nếu 'F' thì quạt đang bật, ngược lại tắt
    } else {
        isFanOn = false; // Nếu không nhận được trạng thái, mặc định là tắt
    }

    // Hiển thị trạng thái quạt ban đầu trên giao diện
    onTxt.setVisible(isFanOn);
    offTxt.setVisible(!isFanOn);
    onTxt.invalidate();
    offTxt.invalidate();

    updateTemp();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::updateTemp()
{
    // Cập nhật lại màn hình hiển thị nhiệt độ
    Unicode::snprintf(tempSettingTxtBuffer, TEMPSETTINGTXT_SIZE, "%d", init_temp);
    tempSettingTxt.invalidate();
}

void Screen2View::increaseTemp()
{
    // Nếu nhiệt độ quá thấp thì không cho thay đổi nhiệt độ nữa
    if (init_temp > -30) init_temp++;
    updateTemp();
}

void Screen2View::decreaseTemp()
{
    // Nếu nhiệt độ quá cao thì không cho giảm nữa
    if (init_temp < 50) init_temp--;
    updateTemp();
}

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();
    uint8_t res;

    if (osMessageQueueGetCount(myQueue01Handle) > 0) {
        // Nhận dữ liệu ở cảm biến ở queue 1
        osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);
        if (res <= 50) {
            Unicode::snprintf(tempTxtBuffer, TEMPTXT_SIZE, "%d", res);
            tempTxt.invalidate();
            uint8_t data;

            // Thuật toán mới:
            if (!isFanOn && res >= init_temp + 2) {
                // Bật quạt khi nhiệt độ vượt ngưỡng 2 độ và quạt đang tắt
                data = 'F';
                osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                isFanOn = true; // Cập nhật trạng thái quạt
            }
            else if (isFanOn && res <= init_temp - 2) {
                // Tắt quạt khi nhiệt độ thấp hơn ngưỡng 2 độ và quạt đang bật
                data = 'f';
                osMessageQueuePut(myQueue03Handle, &data, 0, 10);
                isFanOn = false; // Cập nhật trạng thái quạt
            }

            // Điều chỉnh trạng thái của quạt trên giao diện
            onTxt.setVisible(isFanOn);
            offTxt.setVisible(!isFanOn);
            onTxt.invalidate();
            offTxt.invalidate();
        }
    }
}
