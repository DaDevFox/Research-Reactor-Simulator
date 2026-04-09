#include "../include/SimulatorGuiApp.h"

#include <exception>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {
void setupCrashLogging() {
	try {
		if (!spdlog::default_logger()) {
			auto logger =
					spdlog::basic_logger_mt("simulator-main", "simulator.log", true);
			spdlog::set_default_logger(logger);
			spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
			spdlog::flush_on(spdlog::level::info);
		}
	} catch (...) {
	}

	std::set_terminate([]() {
		try {
			auto ex = std::current_exception();
			if (ex) {
				std::rethrow_exception(ex);
			}
			spdlog::critical("std::terminate called without active exception");
		} catch (const std::exception &e) {
			spdlog::critical("std::terminate: {}", e.what());
		} catch (...) {
			spdlog::critical("std::terminate: unknown exception");
		}
		spdlog::shutdown();
		std::abort();
	});

#if defined(_WIN32)
	SetUnhandledExceptionFilter([](EXCEPTION_POINTERS *ep) -> LONG {
		if (ep && ep->ExceptionRecord) {
			spdlog::critical("Unhandled SEH exception code=0x{:08X} address=0x{:X}",
											 ep->ExceptionRecord->ExceptionCode,
											 reinterpret_cast<uintptr_t>(
													 ep->ExceptionRecord->ExceptionAddress));
		} else {
			spdlog::critical("Unhandled SEH exception (no record)");
		}
		spdlog::shutdown();
		return EXCEPTION_EXECUTE_HANDLER;
	});
#endif
}
}  // namespace

int main(int argc, char **argv) {
	setupCrashLogging();
	spdlog::info("main start");
	const int rc = runSimulatorGuiApp(argc, argv);
	spdlog::info("main exit rc={}", rc);
	return rc;
}
