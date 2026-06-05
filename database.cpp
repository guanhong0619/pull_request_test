#include "database.h"
#include "defines.h"

PostgreSQL::PostgreSQL() : _conn(nullptr) {
	// 建構函數不做任何事
}

PostgreSQL::~PostgreSQL() {
	if (_conn != nullptr) {
		PQfinish(_conn);
		std::cout << "Securely terminate database connection" << std::endl;
	}
}

bool PostgreSQL::init(const std::string& primary_ip, const std::string& secondary_ip) {
	_primary_ip = primary_ip;
	_secondary_ip = secondary_ip;
	_using_primary = true;
	if (connectToHost(_primary_ip)) {
		_using_primary = true;
		std::cout << "Connected to primary database(" + _primary_ip + ")" << std::endl;
		return true;
	}

	std::cout << "Primary IP(" << _primary_ip << ") failed, trying secondary IP(" << _secondary_ip << ")..." << std::endl;

	if (connectToHost(_secondary_ip)) {
		_using_primary = false;
		return true;
	}

	std::cout << "Both primary and secondary IP failed." << std::endl;
	return false;
}

bool PostgreSQL::connectToHost(const std::string& host_ip) {
	_connect_info = "host=" + host_ip +
		" dbname=" + DB_NAME +
		" user=" + DB_USER +
		" password=" + DB_PASSWORD +
		" port=" + std::to_string(DB_PORT);

	if (DB_TIMEOUT_SECONDS > 0)
		_connect_info += " connect_timeout=" + std::to_string(DB_TIMEOUT_SECONDS);

	_conn = PQconnectdb(_connect_info.c_str());
	if (PQstatus(_conn) != CONNECTION_OK) {
		std::cout << "[connectToHost] Connection to DB(" << host_ip << ") failed: " << PQerrorMessage(_conn) << std::endl;
		PQfinish(_conn);
		_conn = nullptr;
		return false;
	}

	std::cout << "[connectToHost] Connected to DB(" << host_ip << ") OK" << std::endl;
	return true;
}

bool PostgreSQL::isConnected() {
	return _conn != nullptr && PQstatus(_conn) == CONNECTION_OK;
}

bool PostgreSQL::ensureConnection() {

	if (isConnected()) {
		return true;
	}

	std::cout << "[ensureConnection] Database Lost connection. Attempting to reconnect..." << std::endl;

	return reConnect();
}

bool PostgreSQL::reConnect() {

	if (_conn != nullptr) {
		PQfinish(_conn);
		_conn = nullptr;
	}

	//  _conn = PQconnectdb(_connect_info.c_str());
	//  if (PQstatus(_conn) == CONNECTION_OK) {
	//      logger.write_log(LogLevel::INFO, "Reconnected to database OK!");
	//      // std::cout << "Reconnected to database successfully!" << std::endl;
	//      return true;
	//  }
	//  else {
		  //logger.write_log(LogLevel::WARNING, std::string("Reconnection to database failed: ") + PQerrorMessage(_conn));
	//      _conn = nullptr;
	//      return false;
	//  }

	std::string try_ip = _using_primary ? _primary_ip : _secondary_ip;
	if (connectToHost(try_ip)) {
		return true;
	}

	// 換另一個 IP 再試一次
	_using_primary = !_using_primary;
	try_ip = _using_primary ? _primary_ip : _secondary_ip;

	std::cout << "[reConnect] Database IP Switching to " << try_ip << " and retrying..." << std::endl;

	return connectToHost(try_ip);
}

// for Insert, Update, Delete
bool PostgreSQL::executeComm(const std::string& sqlComm) {

	std::cout << "Executing SQL: " << sqlComm << std::endl;

	/*確認資料庫連線是否成功*/
	if (!ensureConnection()) {
		return false;
	}

	PGresult* res = PQexec(_conn, sqlComm.c_str());
	/* 執行指令完全失敗 */
	if (res == nullptr) {
		std::cout << "Execution returned NULL: " << std::string(PQerrorMessage(_conn) ? : "Unknown") << std::endl;
		return false;
	}

	ExecStatusType status = PQresultStatus(res);
	/* 執行成功 */
	if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
		PQclear(res);
		return true;
	}
	else {
		/*logger.write_log(LogLevel::WARNING, std::string("Query execution failed: ") + PQerrorMessage(_conn));
		PQclear(res);
		return false;*/
		if (status == PGRES_FATAL_ERROR) {
			std::cout << "Fatal SQL error: " << std::string(PQerrorMessage(_conn)) << std::endl;
			return false;
		}
		else if (status == PGRES_NONFATAL_ERROR) {
			std::cout << "Non-fatal SQL warning: " << std::string(PQerrorMessage(_conn)) << std::endl;
			return false;
		}
		else {
			std::cout << "Unknown query error: " << std::string(PQerrorMessage(_conn)) << std::endl;
			return false;
		}
	}
}

// for Select
int PostgreSQL::executeComm(const std::string& sqlComm, json& resultJson, bool logflag) {

	if (logflag) {
		std::cout << "Executing SQL: " << sqlComm << std::endl;
	}

	if (!ensureConnection()) {
		//if(logflag) logger.write_log(LogLevel::INFO, sqlComm + " result: -1");
		return -1;
	}

	PGresult* res = PQexec(_conn, sqlComm.c_str());

	if (res == nullptr) {
		std::cout << "Execution returned NULL: " << std::string(PQerrorMessage(_conn) ? : "Unknown") << std::endl;
		return -1;
	}

	ExecStatusType status = PQresultStatus(res);

	if (status == PGRES_TUPLES_OK) {
		int rows = PQntuples(res);
		int cols = PQnfields(res);

		for (int i = 0; i < rows; ++i) {
			json rowJson;
			for (int j = 0; j < cols; ++j) {
				/*const char* value = PQgetvalue(res, i, j);
				const char* fieldName = PQfname(res, j);
				rowJson[fieldName] = value;*/
				const char* fieldName = PQfname(res, j);
				if (PQgetisnull(res, i, j)) {
					rowJson[fieldName] = nullptr;
				}
				else {
					rowJson[fieldName] = PQgetvalue(res, i, j);
				}
			}
			resultJson.push_back(rowJson);
		}

		PQclear(res);
		if (logflag) {
			std::cout << "result: " << std::to_string(rows) << " row(s) returned." << std::endl;
		}
		return rows;
	}
	else {
		/*if (logflag) logger.write_log(LogLevel::WARNING, std::string("Query execution failed: ") + PQerrorMessage(_conn));
		PQclear(res);
		if (logflag) logger.write_log(LogLevel::INFO, sqlComm + " result: -1");*/
		if (status == PGRES_FATAL_ERROR) {
			std::cout << "Fatal SQL error: " << std::string(PQerrorMessage(_conn)) << std::endl;
		}
		else if (status == PGRES_NONFATAL_ERROR) {
			if (logflag) {
				std::cout << "Non-fatal SQL warning: " << std::string(PQerrorMessage(_conn)) << std::endl;
			}
		}
		else {
			if (logflag) {
				std::cout << "Unknown query error: " << std::string(PQerrorMessage(_conn)) << std::endl;
			}
		}
		PQclear(res);
		return -1;
		//_mutex.unlock();
	}
}

char* PostgreSQL::getUnpaidPlateNums() {
	const std::string sql = "SELECT plate_num FROM access.movement WHERE (paid_status IS NULL OR paid_status = '') AND comments != 'exit';";

	json result;
	int rows = this->executeComm(sql, result, false);
	if (rows < 0) {
		std::cout << "[get_unpaid_plate_nums] Query failed." << std::endl;
		return nullptr;
	}

	std::vector<std::string> plates;
	plates.reserve((rows > 0) ? (size_t)rows : 0);

	for (int i = 0; i < rows; i++) {
		if (!result[i].contains("plate_num") || result[i]["plate_num"].is_null()) continue;
		plates.push_back(result[i]["plate_num"].get<std::string>());
	}

	char* out = joinComma(plates);
	if (!out) {
		std::cout << "[get_unpaid_plate_nums] Memory allocation failed." << std::endl;
		return nullptr;
	}
	return out;
}

 char* PostgreSQL::getPaidPlateNums() {
	 const std::string sql = "SELECT plate_num FROM access.movement WHERE paid_status = 'paid' AND comments != 'exit';";

	 json result;
	 int rows = this->executeComm(sql, result, false);
	 if (rows < 0) {
		 std::cout << "[get_paid_plate_nums] Query failed." << std::endl;
		 return nullptr;
	 }

	 std::vector<std::string> plates;
	 plates.reserve((rows > 0) ? (size_t)rows : 0);

	 for (int i = 0; i < rows; ++i) {
		 if (!result[i].contains("plate_num") || result[i]["plate_num"].is_null()) continue;
		 plates.push_back(result[i]["plate_num"].get<std::string>());
	 }

	 char* out = joinComma(plates);
	 if (!out) {
		 std::cout << "[get_paid_plate_nums] Memory allocation failed." << std::endl;
		 return nullptr;
	 }
	 return out;
}

 char* PostgreSQL::joinComma(const std::vector<std::string>& plates)
 {
	 // 行為跟你原本一致：無資料回傳空字串（malloc 一個 "\0"）
	 if (plates.empty()) {
		 char* out = (char*)malloc(1);
		 if (!out) return nullptr;
		 out[0] = '\0';
		 return out;
	 }

	 // 計算總長度：sum(len) + (n-1 comma) + 1 null
	 size_t total = 1; // '\0'
	 for (const auto& s : plates) total += s.size();
	 total += (plates.size() - 1);

	 char* out = (char*)malloc(total);
	 if (!out) return nullptr;

	 char* p = out;
	 for (size_t i = 0; i < plates.size(); ++i) {
		 if (i > 0) *p++ = ',';
		 const std::string& s = plates[i];
		 if (!s.empty()) {
			 std::memcpy(p, s.data(), s.size());
			 p += s.size();
		 }
	 }
	 *p = '\0';
	 return out;
 }

