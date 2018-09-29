/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <vector>


#if defined(_WIN32)
#include <windows.h>
typedef cl_ulong uint64_t;
typedef unsigned int uint;
#else
#include <stdint.h>
#include <unistd.h>
#endif

class Timer
{
private:
#if defined(_WIN32)
    LARGE_INTEGER frequency_;
    DWORD         startTick_;
    LONGLONG      prevElapsedTime_;
    LARGE_INTEGER startTime_;
#else
    struct timespec startTime_;
#endif //_WIN32

    template <typename T>
    T _max(T a,T b)
    {
        return (a > b ? a : b);
    }

    uint64_t getTime(unsigned long long scale)
    {
        uint64_t ticks;
#if defined(_WIN32)
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        LONGLONG elapsedTime = currentTime.QuadPart - startTime_.QuadPart;

        // Compute the number of millisecond ticks elapsed.
        unsigned long msecTicks =
            (unsigned long)(1000 * elapsedTime / frequency_.QuadPart);
        // Check for unexpected leaps in the Win32 performance counter.
        // (This is caused by unexpected data across the PCI to ISA
        // bridge, aka south bridge.  See Microsoft KB274323.)
        unsigned long elapsedTicks = GetTickCount() - startTick_;

        signed long msecOff = (signed long)(msecTicks - elapsedTicks);
        if (msecOff < -100 || msecOff > 100) {
                // Adjust the starting time forwards.
                LONGLONG msecAdjustment =
                    _max(msecOff *
                        frequency_.QuadPart / 1000, elapsedTime -
                        prevElapsedTime_);
                startTime_.QuadPart += msecAdjustment;
                elapsedTime -= msecAdjustment;
        }
        // Store the current elapsed time for adjustments next time.
        prevElapsedTime_ = elapsedTime;

        ticks = (uint64_t)(scale*elapsedTime / frequency_.QuadPart);
#else
        struct timespec tp;
        ::clock_gettime(CLOCK_MONOTONIC, &tp);
        // check for overflow
        if ((tp.tv_nsec - startTime_.tv_nsec) < 0)
        {
            // Remove a second from the second field and add it to the
            // nanoseconds field to prevent overflow.
            // Then scale
            ticks = (uint64_t) (tp.tv_sec - startTime_.tv_sec - 1) * scale
                    + (uint64_t) ((1000ULL * 1000ULL * 1000ULL) + tp.tv_nsec - startTime_.tv_nsec)
                                  * scale / (1000ULL * 1000ULL * 1000ULL);
        }
        else
        {
            ticks = (uint64_t) (tp.tv_sec - startTime_.tv_sec) * scale
                + (uint64_t) (tp.tv_nsec - startTime_.tv_nsec) * scale / (1000ULL * 1000ULL * 1000ULL);
        }
#endif //_WIN32

        return ticks;
    }

public:
    //! Constructor
    Timer()
    {
#if defined(_WIN32)
        QueryPerformanceFrequency(&frequency_);
#endif
        reset();
    }

    //! Destructor
    ~Timer()
    {
    }

    /*!
     * \brief Resets timer such that in essence the elapsed time is zero
     * from this point.
     */
    void reset()
    {
#if defined(_WIN32)
        QueryPerformanceCounter(&startTime_);
        startTick_ = GetTickCount();
        prevElapsedTime_ = 0;
#else
        ::clock_gettime(CLOCK_MONOTONIC, &startTime_);
#endif
    }

    /*!
     * \brief Calculates the time since the last reset.
     * \returns The time in milli seconds since the last reset.
     */
    uint64_t getTimeMilliseconds(void)
    {
        return getTime(1000ULL);
    }

    /*!
     * \brief Calculates the time since the last reset.
     * \returns The time in nano seconds since the last reset.
     */
    uint64_t getTimeNanoseconds(void)
    {
        return getTime(1000ULL * 1000ULL * 1000ULL);
    }

    /*!
     * \brief Calculates the time since the last reset.
     * \returns The time in micro seconds since the last reset.
     */
    uint64_t getTimeMicroseconds(void)
    {
        return getTime(1000ULL * 1000ULL);
    }

    /*!
     * \brief Calculates the tick rate for millisecond counter.
     */
    float getMillisecondsTickRate(void)
    {
        return 1000.f;
    }

    /*!
     * \brief Calculates the tick rate for nanosecond counter.
     */
    float getNanosecondsTickRate(void)
    {
        return (float) (1000ULL * 1000ULL * 1000ULL);
    }

    /*!
     * \brief Calculates the tick rate for microsecond counter.
     */
    float getMicrosecondsTickRate(void)
    {
        return (float) (1000ULL * 1000ULL);
    }
};

#endif // UTILS_H
