#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

#ifdef _WIN32
#include <GL/wglew.h>
#include <Windows.h>
#include <GL/GL.h>
#else
#include <GLFW/glfw3.h>
#endif

#include <string>
#include <functional>
#include <cstdint>

#include "../Misc.h"

bool oglOneTimeContext(int versionMajor, int versionMinor, std::function<void()> job, std::string& outError);
