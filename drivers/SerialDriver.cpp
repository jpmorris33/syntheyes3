#include "SerialDriver.hpp"

bool SerialDriver::open_read(const char *port, int baud) {return false;}
bool SerialDriver::open_write(const char *port, int baud) {return false;}
void SerialDriver::close() {}
int SerialDriver::read(char *buffer, int maxlen) {return 0;}
int SerialDriver::write(const char *msg) {return 0;}
