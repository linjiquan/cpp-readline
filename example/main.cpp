#include "../src/xma_shell.hpp"
#include "../src/xma_application.hpp"

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

using namespace xma;

///----------------------------------Test---------------------------------------
class MainProcess:public Process
{
public:
	MainProcess(string name, uint64_t cpu_set): Process(name, cpu_set) {}
	
	void Init() {

	}
	
	void Main() {
		while (true) {
			cout << "Process run: " << Name() << endl;
			this_thread::sleep_for(std::chrono::seconds(2));
		}
	}
private:
	
};


int main()
{
	xma::Application::Init();
	
	//all processes/threads should create before application run
	MainProcess *p = new MainProcess("MainProcess", 0);
	xma::Application::Register(p);

	MainProcess *p1 = new MainProcess("MainProcess1", 0);
	xma::Application::Register(p1);		
	
	Worker *worker = new Worker("Worker", 0);
	xma::Application::Register(worker);	
	
	
	xma::Shell &c = xma::Shell::Instance();
	c.RegisterCommand("info", "show information", info);
	c.RegisterCommand("calc", "calc", calc);
	
	c.RegisterCommand("Test", "Test", [](const std::vector<std::string> &) -> int {
         std::cout << "This is a test command" << std::endl;
	});

	xma::Application::Run();

    return 0;
}
