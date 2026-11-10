// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <fmt/format.h>

#include "common/logging.h"
#include "core/hle/kernel/k_process.h"
#include "core/hle/kernel/k_readable_event.h"
#include "core/hle/kernel/k_synchronization_object.h"
#include "core/hle/kernel/k_thread.h"
#include "core/memory.h"
#include "core/hle/kernel/wait_trace.h"
#include "core/hle/service/kernel_helpers.h"

namespace Kernel::WaitTrace {
namespace {

using Clock = std::chrono::steady_clock;

struct Wait {
    KThread* thread{};
    std::string what;
    Clock::time_point start;
    bool reported{};
};

struct State {
    std::mutex mutex;
    std::unordered_map<u64, Wait> waits;
    std::once_flag watchdog;
};

State& GetState() {
    static State state;
    return state;
}

// The thread is switched out while it waits, so its saved context holds where it called from.
std::string Backtrace(KThread& thread) {
    const auto& context = thread.GetContext();
    std::string out = fmt::format("pc={:#x} lr={:#x} frames:", context.pc, context.lr);
    KProcess* const process = thread.GetOwnerProcess();
    if (process == nullptr) {
        return out;
    }
    auto& memory = process->GetMemory();
    u64 fp = context.fp;
    for (int frame = 0; frame < 16 && fp != 0 && (fp & 0xf) == 0; ++frame) {
        if (!memory.IsValidVirtualAddressRange(fp, 16)) {
            break;
        }
        const u64 next = memory.Read64(fp);
        out += fmt::format(" {:#x}", memory.Read64(fp + 8));
        if (next <= fp) {
            break;
        }
        fp = next;
    }
    return out;
}

void Watch() {
    auto& state = GetState();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::scoped_lock lk{state.mutex};
        const auto now = Clock::now();
        for (auto& [thread_id, wait] : state.waits) {
            const auto seconds =
                std::chrono::duration_cast<std::chrono::seconds>(now - wait.start).count();
            if (!wait.reported && seconds >= 3) {
                wait.reported = true;
                LOG_WARNING(Kernel, "Wait trace: thread {} waiting {} s on {}; {}", thread_id,
                            seconds, wait.what, Backtrace(*wait.thread));
            }
        }
    }
}

} // namespace

bool Enabled() {
    static const bool enabled = std::getenv("EDEN_NX2_WAIT_TRACE") != nullptr;
    return enabled;
}

std::string Describe(KSynchronizationObject* object) {
    if (auto* readable = object->DynamicCast<KReadableEvent*>(); readable != nullptr) {
        return fmt::format("event '{}'", Service::KernelHelpers::DebugEventName(readable));
    }
    return object->GetTypeName();
}

void Begin(KThread& thread, std::string what) {
    auto& state = GetState();
    std::call_once(state.watchdog, [] { std::thread{Watch}.detach(); });
    std::scoped_lock lk{state.mutex};
    state.waits[thread.GetThreadId()] = Wait{&thread, std::move(what), Clock::now()};
}

void End(KThread& thread) {
    const u64 thread_id = thread.GetThreadId();
    auto& state = GetState();
    std::scoped_lock lk{state.mutex};
    const auto it = state.waits.find(thread_id);
    if (it == state.waits.end()) {
        return;
    }
    if (it->second.reported) {
        LOG_WARNING(Kernel, "Wait trace: thread {} woke after {} s from {}", thread_id,
                    std::chrono::duration_cast<std::chrono::seconds>(Clock::now() -
                                                                     it->second.start)
                        .count(),
                    it->second.what);
    }
    state.waits.erase(it);
}

} // namespace Kernel::WaitTrace
