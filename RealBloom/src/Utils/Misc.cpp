#include "Misc.h"

static std::function<void(std::string)> g_printHanlder = [](std::string s) { std::cout << s << "\n"; };

std::string makeError(const std::string& source, const std::string& stage, const std::string& message, bool print)
{
    std::string s = "";

    if (!source.empty())
        s += strFormat("[%s] ", source.c_str());

    if (!stage.empty())
        s += strFormat("%s: ", stage.c_str());

    s += message;

    if (print) g_printHanlder(s);

    return s;
}

void printError(const std::string& source, const std::string& stage, const std::string& message)
{
    makeError(source, stage, message, true);
}

void printWarning(const std::string& source, const std::string& stage, const std::string& message)
{
    makeError(source, stage, message, true);
}

void printInfo(const std::string& source, const std::string& stage, const std::string& message)
{
    makeError(source, stage, message, true);
}

void setPrintHandler(std::function<void(std::string)> handler)
{
    g_printHanlder = handler;
}

uint32_t getMaxNumThreads()
{
    static uint32_t v = 1;
    static bool init = true;
    if (init)
    {
        init = false;
        v = std::max(1u, std::thread::hardware_concurrency());
    }
    return v;
}

uint32_t getDefNumThreads()
{
    static uint32_t num = 1;
    static bool init = true;
    if (init)
    {
        init = false;
        num = std::max(1u, getMaxNumThreads() / 2);
    }
    return num;
}

float getElapsedMs(std::chrono::system_clock::time_point startTime)
{
    auto duration = std::chrono::system_clock::now() - startTime;
    uint64_t elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return (float)elapsedNs / 1000000.0f;
}

float getElapsedMs(std::chrono::system_clock::time_point startTime, std::chrono::system_clock::time_point endTime)
{
    auto duration = endTime - startTime;
    uint64_t elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return (float)elapsedNs / 1000000.0f;
}

#ifdef _WIN32
HANDLE createMutex(const std::string& name)
{
    return CreateMutexA(
        NULL,              // default security descriptor
        FALSE,             // mutex not owned
        name.c_str());     // object name
}

HANDLE openMutex(const std::string& name)
{
    return OpenMutexA(
        MUTEX_ALL_ACCESS,  // request full access
        FALSE,             // handle not inheritable
        name.c_str());     // object name
}

void waitForMutex(HANDLE hMutex)
{
    if (hMutex != NULL)
        WaitForSingleObject(hMutex, INFINITE);
}

void releaseMutex(HANDLE hMutex)
{
    if (hMutex != NULL)
        ReleaseMutex(hMutex);
}

void closeMutex(HANDLE& hMutex)
{
    if (hMutex != NULL)
    {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}
#endif

const std::string& getPathSeparator()
{
#ifdef _WIN32
    static std::string pathSeparator = "\\";
#else
    static std::string pathSeparator = "/";
#endif
    return pathSeparator;
}

const std::string& getExecDir()
{
    static std::string execDir = "";

    if (execDir.empty())
    {
        char path_cstr[2048] = { 0 };
#ifdef _WIN32
        GetModuleFileNameA(NULL, path_cstr, 2048);
#elif defined(__APPLE__)
        uint32_t bufSize = sizeof(path_cstr);
        _NSGetExecutablePath(path_cstr, &bufSize);
#else
        ssize_t len = readlink("/proc/self/exe", path_cstr, sizeof(path_cstr) - 1);
        if (len > 0) path_cstr[len] = '\0';
#endif

        auto path = std::filesystem::path(std::string(path_cstr)).parent_path();
        execDir = std::filesystem::canonical(path).string();

        if (!execDir.ends_with(getPathSeparator()))
            execDir += getPathSeparator();

        execDir = std::filesystem::path(execDir).make_preferred().string();
    }

    return execDir;
}

const std::string& getTempDirectory()
{
    static std::string tempDir = "";

    if (tempDir.empty())
    {
        tempDir = std::filesystem::temp_directory_path().string();
        if (!tempDir.ends_with(getPathSeparator()))
            tempDir += getPathSeparator();
    }

    return tempDir;
}

std::string getLocalPath(const std::string& path)
{
    return getExecDir() + path;
}

std::string getFileExtension(const std::string& filename)
{
    return strLowercase(std::filesystem::path(filename).extension().string());
}


bool deleteFile(const std::string& filename)
{
    if (std::filesystem::exists(filename))
        return std::filesystem::remove(filename);
    return true;
}

#ifdef _WIN32
void killProcess(PROCESS_INFORMATION pi)
{
    if (TerminateProcess(pi.hProcess, 1))
        WaitForSingleObject(pi.hProcess, INFINITE);
}

bool processIsRunning(PROCESS_INFORMATION pi)
{
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode))
        return exitCode == STILL_ACTIVE;
    return false;
}
#endif

void openURL(std::string url)
{
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = "open \"" + url + "\"";
    system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + url + "\"";
    system(cmd.c_str());
#endif
}
