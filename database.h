#pragma once
#include "declaration.h"

class PostgreSQL {
public:
    PostgreSQL();

    ~PostgreSQL();

    bool init(const std::string& primary_ip, const std::string& secondary_ip);

    bool connectToHost(const std::string& host_ip);
    // for Insert, Update, Delete
    bool executeComm(const std::string& sqlComm);
    // for Select
    int  executeComm(const std::string& sqlComm, json& resultJson, bool logflag);

    bool isConnected();
    bool ensureConnection();
    bool reConnect();

    char* getUnpaidPlateNums();
    char* getPaidPlateNums();

private:
    PGconn* _conn;
    std::string _connect_info;

    std::string _primary_ip;
    std::string _secondary_ip;
    bool _using_primary = true;

    static char* joinComma(const std::vector<std::string>& plates);
};
