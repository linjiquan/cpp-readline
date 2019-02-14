#pragma once

// C/C++
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>

// Internal
#include "xma_internal.h"
#include "xma_status.h"

namespace xma {

    using ShellFunc     = std::function<int(const std::vector<std::string> &)>;
    using ShellFuncArgs = const std::vector<std::string>;

    class Shell {
        public:
            struct ShellCommand {
                std::string name;
                std::string help;
                ShellFunc func;
            };

      //以后需要扩展命令，增加参数与帮助说明信息
      using RegisteredCommands = std::map<std::string, ShellCommand *>;

      Shell(std::string &&prompt);
      virtual ~Shell();

      static Shell & Instance() {
        static Shell _shell(">>>");
        return _shell;
      }
      
      void Init();
      void Run();

      void RegisterCommand(ShellCommand & command);
            //void RegisterCommand(const std::string & command, ShellFunc function);
            void RegisterCommand(const std::string & command, const std::string & help, ShellFunc function);
            void SetPrompt(const std::string & prompt);
            std::string GetPrompt() const;
            XmaStatus ExecCommand(const std::string & command);
            XmaStatus ExecFile(const std::string & filename);
            XmaStatus Readline();
      
    private:      
      static char **CommandCompletion (const char *text, int start, int end);
      static char * CommandGenerator(const char * text, int state);
      
    private:
      std::mutex _mutex;
      std::string _prompt;
      
      static RegisteredCommands _commands;
    };
}

