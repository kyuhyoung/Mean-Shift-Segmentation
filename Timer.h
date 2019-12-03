#ifndef TIMER_H
#define TIMER_H



#if defined(WIN32) || defined(_WIN32)   // Windows system specific
#include <windows.h>
#else          // Unix based system specific
#include <sys/time.h>
#endif


class Timer
{
public:
    Timer()
	{
#if defined(WIN32) || defined(_WIN32)
    	QueryPerformanceFrequency(&frequency);
		startCount.QuadPart = 0;
		endCount.QuadPart = 0;
#else
		startCount.tv_sec = startCount.tv_usec = 0;
		endCount.tv_sec = endCount.tv_usec = 0;
#endif
		stopped = 0;
		startTimeInMicroSec = 0;
		endTimeInMicroSec = 0;
        m_n_frame = 0;
	}                                    // default constructor
	~Timer() {};                                   // default destructor

	void Start()                             // start timer
	{
		stopped = 0; // reset stop flag
#if defined(WIN32) || defined(_WIN32)
		QueryPerformanceCounter(&startCount);
#else
		gettimeofday(&startCount, NULL);
#endif
	}
	void Stop()                              // stop the timer
	{
		stopped = 1; // set timer stopped flag

#if defined(WIN32) || defined(_WIN32)
		QueryPerformanceCounter(&endCount);
#else
		gettimeofday(&endCount, NULL);
#endif
	}

    double updateFPS()
    {
        double secElaps = ElapsedTimeInSec();
        return (double)(++m_n_frame) / secElaps;
    }

	double ElapsedTime()                    // get elapsed time in milli-second
	{
	    return this->ElapsedTimeInMilliSec();
	}
	double ElapsedTimeInSec()               // get elapsed time in second 
	{
		return this->ElapsedTimeInMicroSec() * 0.000001;
	}
	double ElapsedTimeInMilliSec()          // get elapsed time in milli-second	(same as getElapsedTime)
	{
		return this->ElapsedTimeInMicroSec() * 0.001;
	}
	double ElapsedTimeInMicroSec()          // get elapsed time in micro-second
	{
#if defined(WIN32) || defined(_WIN32)
        if(!stopped) QueryPerformanceCounter(&endCount);
		startTimeInMicroSec = startCount.QuadPart * (1000000.0 / frequency.QuadPart);
		endTimeInMicroSec = endCount.QuadPart * (1000000.0 / frequency.QuadPart);
#else
		if(!stopped) gettimeofday(&endCount, NULL);

		startTimeInMicroSec = (startCount.tv_sec * 1000000.0) + startCount.tv_usec;
		endTimeInMicroSec = (endCount.tv_sec * 1000000.0) + endCount.tv_usec;
#endif
		return endTimeInMicroSec - startTimeInMicroSec;
	}

protected:

private:
	double startTimeInMicroSec;                 // starting time in micro-second
	double endTimeInMicroSec;                   // ending time in micro-second
	int    stopped;                             // stop flag 
    unsigned long long m_n_frame;
#if defined(WIN32) || defined(_WIN32)
	LARGE_INTEGER frequency;                    // ticks per second
	LARGE_INTEGER startCount;                   //
	LARGE_INTEGER endCount;                     //
#else
	timeval startCount;                         //
	timeval endCount;                           //
#endif
};



#endif	//	TIMER_H
