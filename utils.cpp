#include "declaration.h"
#include "utils.h"

double get_detail_currenttime_milisec()
{
	double		 timeval;
	struct timeb detail_time;

	ftime(&detail_time);
	timeval = (double)detail_time.time + (double)detail_time.millitm / 1000.0;
	return(timeval);
}

void u2b(char src[], char tar[])
{
    //cout << "\nConvert code From UTF-8 to big5...";
    size_t srclen = strlen(src);
    size_t tarlen = srclen / 3 * 2 + 1;
    memset(tar, 0, tarlen);
    iconv_t cd = iconv_open("big5", "UTF-8");
    size_t error = iconv(cd, &src, &srclen, &tar, &tarlen);
    iconv_close(cd);
}

// 計算時間預測的函數
int predict_time(int length) {
    return (-0.0000651 * pow(length, 2)) + (0.1657 * length) + 2.8140;
}

char* get_unpaid_plate_nums() {
    std::string password = "car%2.0nexun!";
    std::string conninfo = "host=10.0.49.184 port=5455 dbname=pms_db user=pms_user password=" + password;
    PGconn* conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    //const char* query = "SELECT plate_num FROM access.movement WHERE paid_status IS NULL OR paid_status = '';";
    const char* query = "SELECT plate_num FROM access.movement WHERE (paid_status IS NULL OR paid_status = '') AND comments != 'exit';";
    PGresult* res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    int num_rows = PQntuples(res);
    if (num_rows == 0) {
        PQclear(res);
        PQfinish(conn);
        return strdup("");  // 無結果時回傳空字串
    }

    // 計算結果字串所需的長度
    size_t total_length = 0;
    for (int i = 0; i < num_rows; i++) {
        total_length += strlen(PQgetvalue(res, i, 0)) + 1;  // +1 計算逗號
    }
    if (num_rows > 0) {
        total_length -= 1;  // 去掉最後一個多算的逗號
    }
    total_length += 1;  // 預留 \0 空間

    // 分配足夠的記憶體
    char* result = (char*)malloc(total_length);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }
    result[0] = '\0';

    // 串接所有車牌號碼
    for (int i = 0; i < num_rows; i++) {
        if (i > 0) strcat(result, ",");  // 添加逗號
        strcat(result, PQgetvalue(res, i, 0));
    }

    PQclear(res);
    PQfinish(conn);
    return result;
}

char* get_paid_plate_nums() {
    std::string password = "car%2.0nexun!";
    std::string conninfo = "host=10.0.49.184 port=5455 dbname=pms_db user=pms_user password=" + password;
    PGconn* conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    //const char* query = "SELECT plate_num FROM access.movement WHERE paid_status = 'paid';";
    const char* query = "SELECT plate_num FROM access.movement WHERE paid_status = 'paid' AND comments != 'exit';";
    PGresult* res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    int num_rows = PQntuples(res);
    if (num_rows == 0) {
        PQclear(res);
        PQfinish(conn);
        return strdup("");  // 無結果時回傳空字串
    }

    // 計算結果字串所需的長度
    size_t total_length = 0;
    for (int i = 0; i < num_rows; i++) {
        total_length += strlen(PQgetvalue(res, i, 0)) + 1;  // +1 計算逗號
    }
    if (num_rows > 0) {
        total_length -= 1;  // 去掉最後一個多算的逗號
    }
    total_length += 1;  // 預留 \0 空間

    // 分配足夠的記憶體
    char* result = (char*)malloc(total_length);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }
    result[0] = '\0';

    // 串接所有車牌號碼
    for (int i = 0; i < num_rows; i++) {
        if (i > 0) strcat(result, ",");  // 添加逗號
        strcat(result, PQgetvalue(res, i, 0));
    }

    PQclear(res);
    PQfinish(conn);
    return result;
}

int getIpThirdOctet(const std::string& ip)
{
    std::stringstream ss(ip);
    std::string token;
    int index = 0;

    while (std::getline(ss, token, '.')) {
        if (index == 2) {
            return std::stoi(token);
        }
        index++;
    }

    // 若格式錯誤（不是合法 IPv4）
    return -1;
}

