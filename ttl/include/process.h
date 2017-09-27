#pragma once
#include <string>
#include <vector>
#include <map>
#include "string_util.h"
#include "noncopyable.h"
#include <fstream>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <cstring>
#include <ext/stdio_filebuf.h>
#endif

namespace thalhammer {
	class process : public noncopyable {
	public:
		typedef std::vector<std::string> args_t;
		typedef std::map<std::string, std::string> env_t;
#ifdef _WIN32
		typedef HANDLE native_handle_t;
#else
		typedef pid_t native_handle_t;
#endif

		process();
		~process();

		bool open(const std::string& path, const args_t& args = {}, const env_t& env = {});
		bool kill();
		bool wait();
		bool is_alive();
		std::ostream& get_stdin();
		std::istream& get_stdout();
		std::istream& get_stderr();
		uint32_t exitcode();
		native_handle_t native_handle() const {	return _native_handle; }

		const std::string& errormsg() const { return _errormsg; }
	private:
		native_handle_t _native_handle;
		std::string _errormsg;
#ifdef _WIN32
		static FILE* open_handle(native_handle_t hdl, const char* mode);
		static bool open_handle(native_handle_t hdl, std::ifstream& stream);
		static bool open_handle(native_handle_t hdl, std::ofstream& stream);
		static std::string last_error();

		std::ofstream _stdin;
		std::ifstream _stdout;
		std::ifstream _stderr;
#else
		__gnu_cxx::stdio_filebuf<char> _stdin_buf;
		__gnu_cxx::stdio_filebuf<char> _stdout_buf;
		__gnu_cxx::stdio_filebuf<char> _stderr_buf;
   		std::unique_ptr<std::ostream> _stdin;
   		std::unique_ptr<std::istream> _stdout;
   		std::unique_ptr<std::istream> _stderr;
#endif
	};
#ifdef _WIN32
	process::process() {
		_native_handle = INVALID_HANDLE_VALUE;
	}

	process::~process() {
		if (_native_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(_native_handle);
		}
	}

	bool process::open(const std::string& path, const args_t& args, const env_t& env) {
		SECURITY_ATTRIBUTES security_attributes;
		STARTUPINFOA startup_info;
		PROCESS_INFORMATION process_information;

		memset(&security_attributes, 0x00, sizeof(security_attributes));
		memset(&startup_info, 0x00, sizeof(startup_info));
		memset(&process_information, 0x00, sizeof(process_information));

		security_attributes.nLength = sizeof(security_attributes);
		startup_info.cb = sizeof(startup_info);

		security_attributes.bInheritHandle = TRUE;
		security_attributes.lpSecurityDescriptor = NULL;

		// Setup pipes
		// pipes[fd][0] = read
		// pipes[fd][1] = write
		HANDLE pipes[3][2];
		for (int i=0; i<3; i++)
		{
			if (!CreatePipe(&pipes[i][0], &pipes[i][1], &security_attributes, 0)) {
				for (int c = 0; c < i; c++) {
					CloseHandle(pipes[c][0]);
					CloseHandle(pipes[c][1]);
				}
				this->_errormsg = "Failed to create pipes";
				return false;
			}
		}

		startup_info.dwFlags = STARTF_USESTDHANDLES;
		startup_info.hStdInput = pipes[0][0];
		startup_info.hStdOutput = pipes[1][1];
		startup_info.hStdError = pipes[2][1];

		// Setup streams
		if (!open_handle(pipes[0][1], _stdin)
			|| !open_handle(pipes[1][0], _stdout)
			|| !open_handle(pipes[2][0], _stderr))
		{
			_stdin.close();
			_stdout.close();
			_stderr.close();
			CloseHandle(pipes[0][0]);
			CloseHandle(pipes[1][1]);
			CloseHandle(pipes[2][1]);
		}

		std::string cmdline = path + " " + string::join(args, std::string(" "));
		if (CreateProcessA(path.c_str(), (char*)cmdline.c_str(), 0, 0, TRUE, 0, 0, 0, &startup_info, &process_information) == FALSE)
		{
			auto strerror = last_error();
			// Close handles
			_stdin.close();
			_stdout.close();
			_stderr.close();
			CloseHandle(pipes[0][0]);
			CloseHandle(pipes[1][1]);
			CloseHandle(pipes[2][1]);
			this->_errormsg = "Failed to create process: " + strerror;
			return false;
		}
		else {
			// Close unused handles
			CloseHandle(pipes[0][0]);
			CloseHandle(pipes[1][1]);
			CloseHandle(pipes[2][1]);
			CloseHandle(process_information.hThread);
			_native_handle = process_information.hProcess;
			return true;
		}

	}

	bool process::kill() {
		if (TerminateProcess(native_handle(), (UINT)-1))
		{
			// TerminateProcess is asynchronous
			return this->wait();
		}
		return false;
	}

	bool process::wait() {
		if (is_alive())
		{
			auto status = WaitForSingleObject(_native_handle, INFINITE);
			while (status != WAIT_OBJECT_0) {
				if (status != WAIT_TIMEOUT) {
					this->_errormsg = "Failed to determine status";
					return false;
				}
				status = WaitForSingleObject(_native_handle, INFINITE);
			}
		}
		return true;
	}

	bool process::is_alive() {
		auto status = WaitForSingleObject(_native_handle, 0);
		if (status == WAIT_OBJECT_0)
			return false;
		else if (status == WAIT_TIMEOUT)
			return true;
		else return false;
	}

	std::ostream& process::get_stdin() {
		return _stdin;
	}

	std::istream& process::get_stdout() {
		return _stdout;
	}

	std::istream& process::get_stderr() {
		return _stderr;
	}

	uint32_t process::exitcode() {
		DWORD code;
		if (GetExitCodeProcess(_native_handle, &code)) {
			return code;
		}
		return -1;
	}

	FILE* process::open_handle(native_handle_t hdl, const char* mode) {
		if (hdl == INVALID_HANDLE_VALUE)
			return nullptr;
		auto fd = _open_osfhandle((intptr_t)hdl, 0);
		if (fd == -1)
			return nullptr;
		auto file = _fdopen(fd, mode);
		if (file == nullptr)
		{
			_close(fd);
			return nullptr;
		}
		return file;
	}

	bool process::open_handle(native_handle_t hdl, std::ifstream& stream) {
		auto file = open_handle(hdl, "rb");
		if (file == nullptr)
			return false;
		stream = std::ifstream(file);
		return true;
	}

	bool process::open_handle(native_handle_t hdl, std::ofstream& stream) {
		auto file = open_handle(hdl, "wb");
		if (file == nullptr)
			return false;
		stream = std::ofstream(file);
		return true;
	}

	std::string process::last_error() {
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, lpMsgBuf, (lstrlen((LPCTSTR)lpMsgBuf)), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpMsgBuf, (lstrlen((LPCTSTR)lpMsgBuf)), &strTo[0], size_needed, NULL, NULL);

		return string::trim_copy(strTo) + " (" + std::to_string(dw) + ")";
	}
#else
	process::process() {

	}

	process::~process() {
		if (_native_handle != 0) {
			// Clean up resources
			siginfo_t info;
			memset(&info, 0x00, sizeof(siginfo_t));
			waitid(P_PID, _native_handle, &info, WEXITED | WNOHANG);
		}
	}

	bool process::open(const std::string& path, const args_t& args, const env_t& env) {
		// 4 pipes
		// Stdin, Stdout, Stderr
		// Status pipe set to close on exec in child, writes exec result on failure, closed otherwise
		// [0] = read [1] = write
		int child_stdout[2] = {-1,-1};
		int child_stdin[2] = {-1,-1};
		int child_stderr[2] = {-1,-1};
		int child_exec_check[2] = {-1,-1};

		#define closepipe(x) if(x[0] != -1) close(x[0]); \
		if(x[1] != -1) close(x[1]);

		#define closepipes() \
		closepipe(child_stdout); \
		closepipe(child_stdin); \
		closepipe(child_stderr); \
		closepipe(child_exec_check)

		if (pipe(child_stdout) == -1
			|| pipe(child_stdin) == -1
			|| pipe(child_stderr) == -1
			|| pipe(child_exec_check) == -1) {
			closepipes();
			this->_errormsg = "pipe() failed";
			return false;
		}

		// Split
		int fresult = fork();
		if (fresult < 0) {
			// Error, should never happen
			closepipes();
			this->_errormsg = "fork() failed";
			return false;
		} else if(fresult == 0) {
			// Child flow
			// stdin write, stderr read, stdout read, check read ends
			close(child_stdin[1]);
			close(child_stdout[0]);
			close(child_stderr[0]);
			close(child_exec_check[0]);
			// Duplicate pipe to stdin
			dup2(child_stdin[0], STDIN_FILENO);
			dup2(child_stdout[1], STDOUT_FILENO);
			dup2(child_stderr[1], STDERR_FILENO);
			// We move check_fd to fd(4) to make closing all other fds easier
			dup2(child_exec_check[1], 4);
			// Enable close on exec
			fcntl(4, F_SETFD, FD_CLOEXEC);

			// Close all fd's larger as 4
			for (int i = getdtablesize(); i > 4; i--)
			{
				close(i);
			}

			// Setup argv and environment arrays
			char** argv = new char*[args.size() + 2];
			argv[args.size() + 1] = NULL;
			argv[0] = new char[path.size() + 1];
			memcpy(argv[0], path.c_str(), path.size());
			for (size_t i=0; i< args.size(); i++)
			{
				argv[i+1] = new char[args[i].size() + 1];
				memcpy(argv[i+1], args[i].c_str(), args[i].size() + 1);
			}

			size_t env_cnt = env.size();
			char** envp = new char*[env_cnt + 1];
			envp[env_cnt] = NULL;
			for (auto& e : env)
			{
				env_cnt--;
				size_t size = e.first.size() + e.second.size() + 2;
				envp[env_cnt] = new char[size];
				memcpy(envp[env_cnt], e.first.data(), e.first.size());
				envp[env_cnt][e.first.size()] = '=';
				memcpy(&envp[env_cnt][e.first.size() + 1], e.second.data(), e.second.size() + 1);
				envp[env_cnt][size-1] = 0x00;
			}

			// execute new process
			// On success check_descriptor is closed,
			// On error  we send errno via check_descriptor and exit
			execve(path.c_str(), argv, envp);
			int terrno = errno;
			int res = write(4, &terrno, sizeof(terrno));
			_exit(-1);
		} else {
			// Parent flow
			_native_handle = fresult;
			// stdin read, stderr write, stdout write, check write ends
			close(child_stdin[0]);
			close(child_stdout[1]);
			close(child_stderr[1]);
			close(child_exec_check[1]);

			// To check if process was successfully started we try to read from child_exec_check.
			// If this fails with broken pipe the process was started successfully
			// Otherwise 4 bytes containing errno are read.
			int terrno = 0;
			ssize_t res = read(child_exec_check[0], &terrno, sizeof(errno));
			if (res == 0) { // execve successfull
				close(child_exec_check[0]);
				// Setup handler streams
				_stdin_buf = __gnu_cxx::stdio_filebuf<char>(child_stdin[1], std::ios_base::out | std::ios_base::binary);
				_stdout_buf = __gnu_cxx::stdio_filebuf<char>(child_stdout[0], std::ios_base::in | std::ios_base::binary);
				_stderr_buf = __gnu_cxx::stdio_filebuf<char>(child_stderr[0], std::ios_base::in | std::ios_base::binary);

				_stdin = std::make_unique<std::ostream>(&_stdin_buf);
				_stdout = std::make_unique<std::istream>(&_stdout_buf);
				_stderr = std::make_unique<std::istream>(&_stderr_buf);

				return true;
			}
			else {
				close(child_exec_check[0]);
				close(child_stdin[1]);
				close(child_stdout[0]);
				close(child_stderr[0]);
				this->_errormsg = "Execve failed: " + std::to_string(terrno);
				return false;
			}
		}
	}

	bool process::kill() {
		if (is_alive()) {
			if (!::kill(_native_handle, SIGKILL)) {
				this->_errormsg = "Kill failed";
				return false;
			}
		}
		return true;
	}

	bool process::wait() {
		siginfo_t info;
		memset(&info, 0x00, sizeof(siginfo_t));
		int res = waitid(P_PID, _native_handle, &info, WEXITED | WNOWAIT);
		if (res == -1) {
			this->_errormsg = "waitid failed";
			return false;
		}
		if (info.si_pid != _native_handle) {
			this->_errormsg = "Invalid pid value";
			return false;
		}
		return true;
	}

	bool process::is_alive() {
		this->_errormsg = "";
		siginfo_t info;
		memset(&info, 0x00, sizeof(siginfo_t));
		int res = waitid(P_PID, _native_handle, &info, WNOHANG | WEXITED | WNOWAIT);
		if (res == -1) {
			this->_errormsg = "waitid failed";
			return false;
		}
		if (info.si_pid == 0) return true;
		if (info.si_pid == _native_handle) return false;
		this->_errormsg = "Invalid pid value";
		return false;
	}

	uint32_t process::exitcode() {
		this->_errormsg = "";
		siginfo_t info;
		memset(&info, 0x00, sizeof(siginfo_t));
		int res = waitid(P_PID, _native_handle, &info, WNOHANG | WEXITED | WNOWAIT);
		if (res == -1) {
			this->_errormsg = "waitid failed";
			return -1;
		}
		if (info.si_pid == 0) {
			this->_errormsg = "still alive";
			return -1;
		}
		if (info.si_pid == _native_handle) return info.si_status;
		this->_errormsg = "Invalid pid value";
		return -1;
	}

	std::ostream& process::get_stdin() {
		return *_stdin;
	}

	std::istream& process::get_stdout() {
		return *_stdout;
	}

	std::istream& process::get_stderr() {
		return *_stderr;
	}
#endif
}
