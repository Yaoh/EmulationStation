#include "AsyncHandle.h"

int AsyncHandle::mAliveHandles = 0;
std::mutex AsyncHandle::mAliveMutex;

