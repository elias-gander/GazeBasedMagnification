// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <sstream>

#include <Producer.h>
#include <Consumer.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")