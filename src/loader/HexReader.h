//
// Created by sebastian on 05.06.19.
//

#pragma once


#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "../../includes/intelhexclass.h"
#include "../json/ConfigManager.h"
#include "../units/Byte.h"
#include "DataSendManager.h"

class HexReader {
public:
    using byte = CustomDataTypes::ComputerScience::byte;
    HexReader(const std::string& fileLocation, const byte& maxSize);
	void writeToStream(DataSendManager& manager);
private:
    intelhex hex;
};

DataSendManager& operator<<(DataSendManager& sender, const HexReader& reader);

