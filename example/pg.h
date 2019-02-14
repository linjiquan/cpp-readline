#pragma once

// C/C++
#include <string>

#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_service.h"

using namespace xma;

class PacketGenerator: public Service
{
public:
  PacketGenerator(std::string name): Service(name) {}

  void OnInit() {
    //Create the basic socket
  }


};
