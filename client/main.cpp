
// C/C++
#include <iostream>
#include <string>
#include <thread>


// Internal
#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "client_process.h"

using namespace xma;

///----------------------------------Test---------------------------------------
int main()
{
  xma::Application::Init();


  xma::Shell &c = xma::Shell::Instance();

  c.RegisterCommand("Test", "Test", [](const std::vector<std::string> &) -> int {
    std::cout << "This is a test command" << std::endl;
    return 0;
  });

  std::unique_ptr<TcpClientProcess> client = std::unique_ptr<TcpClientProcess>(new TcpClientProcess());

  xma::Application::Run();
  
  return 0;
}
