#include "xma_shell.hpp"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include <cstdlib>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>

namespace xma {
	
	Shell::RegisteredCommands Shell::_commands;
	
    Shell::Shell(std::string &&prompt)
    {
		_prompt = prompt;
		
        rl_attempted_completion_function = &Shell::CommandCompletion;

        // 默认命令
        RegisterCommand("help", "show this help", [this](const Argv &){
            auto commands = this->_commands;
            std::cout << " Available commands are:\n";
			
            for ( auto & command : commands ) 
				std::cout << "\t" << command.first << "\t" << command.second->help << std::endl;
			
            return XS_SUCCESS;
        });

        RegisterCommand("quit", "quit the process", [](const Argv &) {
            return XS_QUIT;
        });
		
		Init();
    }

    Shell::~Shell()
    {
        for (auto &cmd: _commands)
            delete cmd.second;
    }

	void Shell::Init() {
		std::cout <<" ====================================================" << std::endl;
		std::cout <<" Welcome to eXentible Message Architecture Framework." << std::endl;
		std::cout <<" ====================================================" << std::endl;
		std::cout << std::endl << std::endl << std::endl;
	}

    void Shell::RegisterCommand(const std::string & command, const std::string & help, ShellFunc func) {
        ShellCommand *_cmd = new ShellCommand();

        _cmd->name = command;
        _cmd->help = help;
        _cmd->func = func;

        std::unique_lock<std::mutex> lock(_mutex);
        _commands[command] = _cmd;
    }


    void Shell::SetPrompt(const std::string & prompt) {
        _prompt = prompt;
    }

    std::string Shell::GetPrompt() const {
        return _prompt;
    }

    XmaStatus Shell::ExecCommand(const std::string & command) {
        // Convert input to vector
        std::vector<std::string> inputs;
        {
            std::istringstream iss(command);
            std::copy(std::istream_iterator<std::string>(iss),
                    std::istream_iterator<std::string>(),
                    std::back_inserter(inputs));
        }

        if (inputs.size() == 0) 
			return XS_SUCCESS;

		auto it = _commands.find(inputs[0]);
		if (it == _commands.end()) {
			std::cout << "Command '" << inputs[0] << "' not found.\n";
			return XS_INTRNL_ERR;
		}
        
        return static_cast<XmaStatus>((it->second->func)(inputs));
    }

    XmaStatus Shell::ExecFile(const std::string & filename) {
        std::ifstream input(filename);
        if ( ! input ) {
            std::cout << "Could not find the specified file to execute." << std::endl;
            return XS_INTRNL_ERR;
        }
		
        std::string command;
        int counter = 0, line = 0;
		XmaStatus result;

        while (std::getline(input, command)) {
			++line;
			
            if ( command[0] == '#' || command[0] == '/') 
				continue;
			
            std::cout << "[" << counter << "] " << command << std::endl;
			
			result = ExecCommand(command);
            if (result != XS_SUCCESS) {
				std::cout << "Execute '" << command << "' " << "in line " << line << " failed" << std::endl;
				return result;
			}
			
            ++counter; 
			std::cout << std::endl;
        }

        return XS_SUCCESS;
    }

    XmaStatus Shell::Readline() {
        char * buffer = readline(GetPrompt().c_str());
        if (!buffer) {
            std::cout << std::endl;
            return XS_QUIT;
        }

        if (buffer[0] != '\0')
            add_history(buffer);

        std::string line(buffer);
        free(buffer);

        return ExecCommand(line);
    }


	void Shell::Run() {
		
		std::cout << " Shell command loop is launched, press '?' for help" << std::endl;

		XmaStatus ret;
		while (true) {
			ret = Readline();
			switch (ret) {
			case XS_QUIT: 
				return ;

			case XS_SUCCESS:
				SetPrompt("$");
				break;

			case XS_INTRNL_ERR:
				SetPrompt("!#");
				break;

			default:
				break;

			}
		}
	}


    char **Shell::CommandCompletion(const char * text, int start, int end) {
        char ** matches = nullptr;

        if ( start == 0 )
            matches = rl_completion_matches(text, &Shell::CommandGenerator);

        return matches;
    }

    char * Shell::CommandGenerator(const char * text, int state) {
		auto it = _commands.find(text);
		if (it == _commands.end())
			return nullptr;
		
		return strdup(it->first.c_str());
    }
}
