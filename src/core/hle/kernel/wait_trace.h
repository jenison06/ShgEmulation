// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "common/common_types.h"

namespace Kernel {
class KSynchronizationObject;
class KThread;
}

// EDEN_NX2_WAIT_TRACE=1: names what blocked guest threads wait on. Each wait with no timeout or
// one of a second or more is recorded while it lasts; a watchdog logs every wait older than 3 s
// once, with its objects (service events by name) and the thread's saved pc, lr and frame-pointer
// return addresses, and logs again when that thread wakes.
namespace Kernel::WaitTrace {

bool Enabled();

std::string Describe(KSynchronizationObject* object);

void Begin(KThread& thread, std::string what);
void End(KThread& thread);

/// Whether a wait with this timeout is long enough to record.
constexpr bool IsLongWait(s64 timeout_ns) {
    return timeout_ns < 0 || timeout_ns >= 1'000'000'000;
}

} // namespace Kernel::WaitTrace
