#pragma once

struct led_display_type {
	bool _is_scrolling;
	bool _is_chinese;
	int  line;
	char content[500];
};


extern std::string led_ip;
//extern bool _led_set_success_flag;
extern bool _prev_line1_is_scrolling;
extern bool _prev_line1_is_chinese;
extern int  _prev_line1_line;
extern char _prev_line1_content[500];
extern bool _prev_line2_is_scrolling;
extern bool _prev_line2_is_chinese;
extern int  _prev_line2_line;
extern char	_prev_line2_content[500];
extern std::vector<struct led_display_type> _led_display;
extern std::mutex  _led_thread_mutex;

bool initLed();

bool resetLed();

void sendLedMessageToTask(bool isScrolling, bool isChinese, int line, char* content);

void ledDisplay(bool isScrolling, bool isChinese, int line, char* content);
