#include "declaration.h"
#include "threads.h"
#include "led.h"

void ledDisplayTask() {
	struct led_display_type led_display_tmp;
	bool status;
	
	int retry_reset_count = 0;
	bool led_reset_ok_flag = false;
	while (!led_reset_ok_flag) {
		if (resetLed()) {
			led_reset_ok_flag = true;
			std::cout << "Reset LED OK (retry=" << retry_reset_count << ")" << std::endl;
			break;
		}
		retry_reset_count++;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	int retry_init_count = 0;
	bool led_init_ok_flag = false;
	while (!led_init_ok_flag) {
		if (initLed()) {
			led_init_ok_flag = true;
			std::cout << "Init LED OK (retry=" << retry_init_count << ")" << std::endl;
			break;
		}
		retry_init_count++;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	
	
	while (1) {
		status = false;
		/*if (!_led_set_success_flag) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}*/
		_led_thread_mutex.lock();
		if (!_led_display.empty()) {
			auto itc = _led_display.begin();

			led_display_tmp = *itc;

			_led_display.erase(itc);

			status = true;
		}
		_led_thread_mutex.unlock();
		if (status) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			ledDisplay(led_display_tmp._is_scrolling, led_display_tmp._is_chinese, led_display_tmp.line, led_display_tmp.content);
			std::cout << "LED Display: " << std::string(led_display_tmp.content) << " at line" << std::to_string(led_display_tmp.line) << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
