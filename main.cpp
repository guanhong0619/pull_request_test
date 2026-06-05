#include "declaration.h"
#include "main.h"
#include "led.h"
#include "utils.h"
#include "threads.h"
#include "defines.h"
#include "database.h"

int main(int argc, char *argv[])
{
	led_ip = argv[1];
	std::cout << "led ip: " << led_ip << std::endl;
	std::thread(ledDisplayTask).detach();

	std::string primary_ip   = "10.0." + std::to_string(getIpThirdOctet(led_ip)) + ".184";
	std::string secondary_ip = "10.0." + std::to_string(getIpThirdOctet(led_ip)) + ".184";
	PostgreSQL db;
	if (!db.init(primary_ip, secondary_ip)) {
		std::cout << "Init Database Connection for CommandfileFunction Failed" << std::endl;
	}

	

	int status = SHOWUNPAID;
	while (1) {

		if (status == SHOWUNPAID) {

			char* unpaid = db.getUnpaidPlateNums();
			if (unpaid != nullptr && strlen(unpaid) > 0) {
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "尚未繳費車輛");
				sendLedMessageToTask(ISSCROLLING,  ENGLISH, 2, unpaid);

				int _wait_time = predict_time(strlen(unpaid));
				std::this_thread::sleep_for(std::chrono::seconds(_wait_time));
				free(unpaid);  // 釋放記憶體
			}
			else {
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "無尚未繳費車輛");
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 2, "");
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
			sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "");
			sendLedMessageToTask(NOTSCROLLING, CHINESE, 2, "");
			
			status = SHOWPAID;
		}
		else {
			char* paid = db.getPaidPlateNums();
			if (paid != nullptr && strlen(paid) > 0) {
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "已繳費車輛");
				sendLedMessageToTask(ISSCROLLING,  ENGLISH, 2, paid);

				int _wait_time = predict_time(strlen(paid));
				std::this_thread::sleep_for(std::chrono::seconds(_wait_time));
				free(paid);  // 釋放記憶體
			}
			else {
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "無已繳費車輛");
				sendLedMessageToTask(NOTSCROLLING, CHINESE, 2, "");
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}

			sendLedMessageToTask(NOTSCROLLING, CHINESE, 1, "");
			sendLedMessageToTask(NOTSCROLLING, CHINESE, 2, "");

			status = SHOWUNPAID;
		}
	
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
}