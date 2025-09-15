#include <chrono>

double get_seconds_since_epoch()
{
	using namespace std::chrono;
	return duration<double>(steady_clock::now().time_since_epoch()).count();
}
