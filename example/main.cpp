#include "../src/xma_shell.hpp"

#include <iostream>
#include <string>
#include <thread>


unsigned info(const std::vector<std::string> &) {
    std::cout << "Welcome to the example console. This command does not really\n"
              << "do anything aside from printing this statement. Thus it does\n"
              << "not need to look into the arguments that are passed to it.\n";
    return 0;
}

// In this command we implement a basic calculator
unsigned calc(const std::vector<std::string> & input) {
    if ( input.size() != 4 ) {
        // The first element of the input array is always the name of the
        // command as registered in the console.
        std::cout << "Usage: " << input[0] << " num1 operator num2\n";
        // We can return an arbitrary error code, which we can catch later
        // as Console will return it.
        return 1;
    }
    double num1 = std::stod(input[1]),
           num2 = std::stod(input[3]);

    char op = input[2][0];

    double result;
    switch ( op ) {
        case '*':
            result = num1 * num2;
            break;
        case '+':
            result = num1 + num2;
            break;
        case '/':
            result = num1 / num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        default:
            std::cout << "The inserted operator is not supported\n";
            // Again, we can return an arbitrary error code to catch it later.
            return 2;
    }
    std::cout << "Result: " << result << '\n';
    return 0;
}

int main() {
    // We create a console. The '>' character is used as the prompt.
    // Note that multiple Consoles can exist at once, as they automatically
    // manage the underlying global readline state.
	xma::Shell &c = xma::Shell::Instance();

    // Here we register a new command. The string "info" names the command that
    // the user will have to type in in order to trigger this command (it can
    // be different from the function name).
    c.RegisterCommand("info", "show information", info);
    c.RegisterCommand("calc", "calc", calc);

    //xma::Shell::ShellCommand shell;
    //shell.name = "ShowThreadId";
    //shell.help = "Show current thread ID";
    //shell.func = calc;//std::move([](const std::vector<std::string> &argv) {});
    //c.RegisterCommand(shell);

    std::cout << std::this_thread::get_id() << std::endl;

    c.RegisterCommand("ShowThreadId", "show thread id", [](const std::vector<std::string> &) -> int {
        std::cout << std::this_thread::get_id() << std::endl;
    });

    // Here we call one of the defaults command of the console, "help". It lists
    // all currently registered commands within the console, so that the user
    // can know which commands are available.
    c.ExecCommand("help");

	c.Run();

    return 0;
}
