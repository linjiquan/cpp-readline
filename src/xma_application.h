#pragma once

namespace xma {

  class Application
  {
  public:

    Application() = default;
    ~Application() = default;

    //change the currently thread to xma thread
    static void Init();
    static void Run();
    static void Exit();
    
  private:
  };

}
