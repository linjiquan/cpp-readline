#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "xma_status.h"

namespace xma {

    class Shell {
        public:

            using Args = std::vector<std::string>;
            using ShellFunc = std::function<int(const Args &)>;
			
			//以后需要扩展命令，增加参数与帮助说明信息
			using RegisteredCommands = std::map<std::string, ShellFunc>;

            Shell(std::string prompt);
            ~Shell() = default;

			static Shell & Instance() {
				static Shell _shell(">>>");
				return _shell;
			}
			
			void Init();
			void Run();

			//void RegisterCommand(ShellCommand & command);
            void RegisterCommand(const std::string & command, ShellFunc function);
            void SetPrompt(const std::string & prompt);
            std::string GetPrompt() const;
            XmaStatus ExecCommand(const std::string & command);
            XmaStatus ExecFile(const std::string & filename);
            XmaStatus Readline();
			
        private:			
			static char **CommandCompletion (const char *text, int start, int end);
			static char * CommandGenerator(const char * text, int state);
			
		private:
			std::string _prompt;
			
			static RegisteredCommands _commands;
			
    };
}

