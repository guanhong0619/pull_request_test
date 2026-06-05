#include "declaration.h"
#include "tcp.h"
#include "led.h"
#include "utils.h"

std::string led_ip = "";
//bool		_led_set_success_flag = false;
bool		_prev_line1_is_scrolling = false;
bool		_prev_line1_is_chinese = false;
int			_prev_line1_line = 0;
char		_prev_line1_content[500] = { 0 };
bool		_prev_line2_is_scrolling = false;
bool		_prev_line2_is_chinese = false;
int			_prev_line2_line = 0;
char		_prev_line2_content[500] = { 0 };
std::vector<struct led_display_type> _led_display;
std::mutex  _led_thread_mutex;

bool initLed() {
	try {
		unsigned char LED_msg[1000];
		unsigned int  ichecksum = 0;
		unsigned char cid = 1;
		int	idx = 0;
		int packet_data_len = 18;
		int network_data_len = packet_data_len + 11;

		unsigned char LED26_packet16_2windows[] = {
			0x01, 0x02,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10,
			0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x10
		};

		// (2) set led windows size
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = network_data_len;	// len (from packet type)
		LED_msg[idx++] = 0x00;				// len
		LED_msg[idx++] = 0x00;				// reserve
		LED_msg[idx++] = 0x00;				// reserve
		LED_msg[idx++] = 0x68;				// packet type
		LED_msg[idx++] = 0x32;				// card type
		LED_msg[idx++] = cid;				// control card id
		LED_msg[idx++] = 0x7B;
		LED_msg[idx++] = 0x01;				// RR
		LED_msg[idx++] = packet_data_len;	// packet data len
		LED_msg[idx++] = 0x00;				// packet data len
		LED_msg[idx++] = 0x00;
		LED_msg[idx++] = 0x00;

		for (int i = 0; i < packet_data_len; i++) {
			LED_msg[idx++] = LED26_packet16_2windows[i];
		}
		for (int i = 8; i < idx; i++) {
			ichecksum = ichecksum + LED_msg[i];
		}
		LED_msg[idx++] = ichecksum % 256;
		LED_msg[idx++] = ichecksum >> 8;

		TCP ledTcp;
		if (!ledTcp.initTcp(led_ip, 5200, 1.0)) {
			std::cout << "[initLed] Failed to connect to LED" << std::endl;
			return false;
		}

		ledTcp.Send((char*)LED_msg, idx);
		std::this_thread::sleep_for(std::chrono::microseconds(500));

		char buf[1024] = { 0 };
		int ret = ledTcp.Recv(buf, 255);

		std::ostringstream log_stream;
		log_stream << "LED(n=" << ret << "):";
		for (int p = 0; p < ret; ++p) {
			log_stream << " " << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)(unsigned char)buf[p];
		}
		std::cout << "[initLed] " << log_stream.str() << std::endl;

		if (ret != 20) {
			std::cout << "[initLed] unexpected response length (n=" << std::to_string(ret) << ")" << std::endl;
			return false;
		}
		//FF FF FF FF 0C 00 00 00 E8 32 01 7B 00 01 00 00 00 01 98 01
		//FF FF FF FF 0C 00 00 00 E8 32 02 7B 00 01 00 00 00 01 99 01
		int p = 0;
		if (buf[p++] == 0xFF &&
			buf[p++] == 0xFF &&
			buf[p++] == 0xFF &&
			buf[p++] == 0xFF &&
			buf[p++] == 0x0C &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x00 &&
			buf[p++] == 0xE8 &&
			buf[p++] == 0x32 &&
			buf[p++] >= 0x01 &&
			buf[p++] == 0x7B &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x01 &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x00 &&
			buf[p++] == 0x01) {
			std::cout << "[initLed] success" << std::endl;
			return true;
		}
		else {
			std::cout << "[initLed] response content mismatch" << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) {
		std::cout << "[initLed] " << std::string("Exception in initLed: ") << e.what() << std::endl;
		return false;
	}
}

bool resetLed() {
	try {
		unsigned char LED26_reset[] = {
		0xFF, 0xFF, 0xFF, 0xFF,
		0x08, 0x00, 0x00, 0x00,
		0x68, 0x32, 0xFF, 0x2D,
		0x01, 0x00, 0xC7, 0x01
		};
		TCP ledTcp;
		if (!ledTcp.initTcp(led_ip, 5200, 1.0f)) {
			std::cout << "[resetLed] Failed to reset LED" << std::endl;
			return false;
		}

		ledTcp.Send((char*)LED26_reset, sizeof(LED26_reset));
		std::this_thread::sleep_for(std::chrono::microseconds(500));

		char buf[1024] = { 0 };
		int n = ledTcp.Recv(buf, 255);
		if (n > 0) {
			std::ostringstream log_stream;
			log_stream << "LED Reset Response (n=" << n << "):";
			for (int p = 0; p < n; ++p) {
				log_stream << " " << std::hex << std::uppercase << std::setw(2)
					<< std::setfill('0') << (int)(unsigned char)buf[p];
			}
			std::cout << "[resetLed] " << log_stream.str() << std::endl;
			return true;
		}
		else {
			std::cout << "[resetLed] No response received after LED reset command" << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) {
		std::cout << "[resetLed] " << std::string("Exception in resetLed: ") << e.what() << std::endl;
		return false;
	}
}

void sendLedMessageToTask(bool isScrolling, bool isChinese, int line, char* content) {
	/*if (!_led_set_success_flag) {
		return;
	}*/
	if (line == 1) {
		char _tmp_content[500] = { 0 };
		strcpy(_tmp_content, content);
		if (isScrolling == _prev_line1_is_scrolling &&
			isChinese == _prev_line1_is_chinese &&
			line == _prev_line1_line &&
			strcmp(_tmp_content, _prev_line1_content) == 0) {
			return;
		}
	}
	else if (line == 2) {
		char _tmp_content[500] = { 0 };
		strcpy(_tmp_content, content);
		if (isScrolling == _prev_line2_is_scrolling &&
			isChinese == _prev_line2_is_chinese &&
			line == _prev_line2_line &&
			strcmp(_tmp_content, _prev_line2_content) == 0) {
			return;
		}
	}

	led_display_type led_display_type;
	led_display_type._is_scrolling = isScrolling;
	led_display_type._is_chinese = isChinese;
	led_display_type.line = line;
	memset(led_display_type.content, 0, sizeof(led_display_type.content));
	strcpy(led_display_type.content, content);

	/*update previous display info*/
	if (line == 1) {
		_prev_line1_is_scrolling = isScrolling;
		_prev_line1_is_chinese = isChinese;
		_prev_line1_line = line;
		strcpy(_prev_line1_content, content);
		// _prev_line1_content      = content;
	}
	else if (line == 2) {
		_prev_line2_is_scrolling = isScrolling;
		_prev_line2_is_chinese = isChinese;
		_prev_line2_line = line;
		strcpy(_prev_line2_content, content);
		// _prev_line2_content = content;
	}

	_led_thread_mutex.lock();
	_led_display.push_back(led_display_type);
	_led_thread_mutex.unlock();
}

void ledDisplay(bool isScrolling, bool isChinese, int line, char* content) {
	try {
		// === 根據內容長度自動決定是否 scrolling ===
		if (content != nullptr) {
			int char_count = 0;

			if (isChinese) {
				isScrolling = (strlen(content) > 7);
			}
			else {
				// English：1 byte = 1 字
				isScrolling = (strlen(content) > 12);
			}
		}

		unsigned char LED_msg[1000];
		unsigned int  ichecksum = 0;
		unsigned char packet_data[500];
		unsigned char cid = 1;
		int packet_data_len = 0;

		if (!isChinese) {
			int p = 0;
			packet_data[p++] = 0x12;	// display pure text (for english)
			packet_data[p++] = (line == 1) ? 0x00 : 0x01;
			packet_data[p++] = (isScrolling) ? 0x0b : 0x00;
			packet_data[p++] = 0x01;	// alignment: 1 => horizontal
			packet_data[p++] = 0x01;	// speed:	  1 => faster 
			packet_data[p++] = 0xff;	// stay time: time hi byte
			packet_data[p++] = 0xff;	// stay time: time lo byte
			packet_data[p++] = 0x02;	// font size: 0 => x8 2 => x16
			packet_data[p++] = 0xff;	// color R:   255
			packet_data[p++] = 0xff;	// color G:   255
			packet_data[p++] = 0xff;	// color B:   255
			int len = strlen(content);
			for (int i = 0; i < len; i++) {
				packet_data[p++] = content[i];
			}
			packet_data[p++] = '\0';
			packet_data_len = p;
		}
		else {
			char big5_data[100] = { 0 };
			u2b(content, big5_data);
			if (strlen(big5_data) == 0) {
				strcpy(big5_data, content); // fallback to original
			}
			int p = 0;
			packet_data[p++] = 0x02;	// display chinese text (3 bytes)
			packet_data[p++] = (line == 1) ? 0x00 : 0x01;
			packet_data[p++] = (isScrolling) ? 0x0b : 0x00;
			packet_data[p++] = 0x01;	// alignment: 1 => horizontal
			packet_data[p++] = 0x04;	// speed:	  1 => faster 
			packet_data[p++] = 0xff;	// stay time: time hi byte
			packet_data[p++] = 0xff;	// stay time: time lo byte
			for (int i = 0; i < strlen(big5_data); i++) {
				if (i % 2 == 0) packet_data[p++] = 0x12;
				packet_data[p++] = big5_data[i];	// 12 bytes (8x3) 0x12 (red and x16) 0xB5 0xE7 (GB)
			}
			packet_data[p++] = '\0';
			packet_data[p++] = '\0';
			packet_data[p++] = '\0';
			packet_data_len = p;
		}

		int idx = 0;
		int network_data_len = packet_data_len + 11;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = 0xFF;
		LED_msg[idx++] = network_data_len;	// len (from packet type)
		LED_msg[idx++] = 0x00;				// len
		LED_msg[idx++] = 0x00;				// reserve
		LED_msg[idx++] = 0x00;				// reserve
		LED_msg[idx++] = 0x68;				// packet type
		LED_msg[idx++] = 0x32;				// card type
		LED_msg[idx++] = cid;					// control card id
		LED_msg[idx++] = 0x7B;
		LED_msg[idx++] = 0x01;
		LED_msg[idx++] = packet_data_len;	//length(Lbyte)
		LED_msg[idx++] = 0x00;				//length(Hbyte)
		LED_msg[idx++] = 0x00;				// packet number
		LED_msg[idx++] = 0x00;				// total

		for (int i = 0; i < packet_data_len; i++) {
			LED_msg[idx++] = packet_data[i];
		}

		for (int i = 8; i < idx; i++) {
			ichecksum += LED_msg[i];
		}

		LED_msg[idx++] = ichecksum % 256;
		LED_msg[idx++] = ichecksum >> 8;

		//避免失敗，多傳一次
		for (int i = 0; i < 2; i++) {
			TCP ledTcp;
			if (!ledTcp.initTcp(led_ip, 5200, 1.0f)) {
				std::cout << "Failed to connect to LED controller on attempt " << std::to_string(i + 1) << std::endl;
				continue;
			}

			ledTcp.Send(reinterpret_cast<char*>(LED_msg), idx);
			std::cout << "LED display packet sent (attempt " << std::to_string(i + 1) << ", bytes=" << std::to_string(idx) << ")" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << std::string("Exception in ledDisplay: ") << e.what() << std::endl;
	}
}
