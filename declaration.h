#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <iostream>
#include <fcntl.h>
#include <dirent.h> 
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <vector>
#include <errno.h>
#include <iosfwd>
#include <memory>
#include <utility>
#include <iconv.h>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <string>
#include <cstring>

#include <libpq-fe.h>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <mutex>

using namespace std;
using std::string;

#include <nlohmann/json.hpp>
using json = nlohmann::json;
