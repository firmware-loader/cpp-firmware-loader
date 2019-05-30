#include <iostream>
#include <Poco/JSON/ParseHandler.h>
#include <Poco/Environment.h>
#include <catch.hpp>
#include <clara.hpp>
#include <asio.hpp>

#include "src/commandline/parse.h"
#include "src/utils/enum_constants.h"
#include "src/json//deviceParser.h"
#include "src/json/configFinder.h"

int main(int argc, const char* argv[]) {
    Poco::JSON::ParseHandler handler;

    Parse<OSTypes::Linux>{argc, argv};
    ConfigFinder config{"atmega328p"};
    if(auto location = config.getFileLocation()) {
        std::cout << "File Location: " << location->string() << std::endl;
    }
    if(auto content = config.getFileContents()) {
        DeviceParser parser{*content};
        std::cout << "ID: " << parser.getID() << std::endl;
    }

    std::cout << Poco::Environment::osDisplayName() << std::endl;
    std::cout << Poco::Environment::processorCount() << std::endl;
    std::cout << Poco::Environment::os() << std::endl;
    return 0;
}