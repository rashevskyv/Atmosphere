/*
 * Copyright (c) 2018-2020 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <mesosphere.hpp>

namespace ams::kern {

    void KWaitObject::OnTimer() {
        MESOSPHERE_ASSERT(KScheduler::IsSchedulerLockedByCurrentThread());

        /* Wake up all the waiting threads. */
        for (KThread &thread : m_wait_list) {
            thread.Wakeup();
        }
    }

    Result KWaitObject::Synchronize(s64 timeout) {
        /* Perform the wait. */
        KHardwareTimer *timer = nullptr;
        KThread *cur_thread   = GetCurrentThreadPointer();
        {
            KScopedSchedulerLock sl;

            /* Check that the thread isn't terminating. */
            R_UNLESS(!cur_thread->IsTerminationRequested(), svc::ResultTerminationRequested());

            /* Verify that nothing else is already waiting on the object. */
            if (timeout > 0) {
                R_UNLESS(!m_timer_used, svc::ResultBusy());
            }

            /* Check that we're not already in use. */
            if (timeout >= 0) {
                /* Verify the timer isn't already in use. */
                R_UNLESS(!m_timer_used, svc::ResultBusy());
            }

            /* If we need to, register our timeout. */
            if (timeout > 0) {
                /* Mark that we're using the timer. */
                m_timer_used = true;

                /* Use the timer. */
                timer = std::addressof(Kernel::GetHardwareTimer());
                timer->RegisterAbsoluteTask(this, timeout);
            }

            if (timeout == 0) {
                /* If we're timed out immediately, just wake up the thread. */
                this->OnTimer();
            } else {
                /* Otherwise, sleep until the timeout occurs. */
                m_wait_list.push_back(GetCurrentThread());
                cur_thread->SetState(KThread::ThreadState_Waiting);
                cur_thread->SetSyncedObject(nullptr, svc::ResultTimedOut());
            }
        }

        /* Cleanup as necessary. */
        {
            KScopedSchedulerLock sl;

            /* Remove from the timer. */
            if (timeout > 0) {
                MESOSPHERE_ASSERT(m_timer_used);
                MESOSPHERE_ASSERT(timer != nullptr);
                timer->CancelTask(this);
                m_timer_used = false;
            }

            /* Remove the thread from our queue. */
            if (timeout != 0) {
                m_wait_list.erase(m_wait_list.iterator_to(GetCurrentThread()));
            }
        }

        return ResultSuccess();
    }

}
