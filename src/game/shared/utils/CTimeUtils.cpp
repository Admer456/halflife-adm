
#include "CTimeUtils.h"
#include <ctime>

// Not thread-safe
static tm& GetLocalTime()
{
	// This API is a little ugly, but my MSVC does not
	// seem to have the new C++20 chrono classes for day/month/year :(
	time_t now = std::time(nullptr);
	tm* localNow = std::localtime(&now);

	localNow->tm_year += 1900;
	localNow->tm_mon += 1;

	return *localNow;
}

bool CTimeUtils::IsTime(const int& hour, const int& minute, const int& second)
{
	tm now = GetLocalTime();

	if (hour != -1 && now.tm_hour != hour)
		return false;

	if (minute != -1 && now.tm_min != minute)
		return false;

	if (second != -1 && now.tm_sec != second)
		return false;

	return true;
}

void CTimeUtils::GetTime(int& outHours, int& outMinutes, int& outSeconds)
{
	tm now = GetLocalTime();

	outHours = now.tm_hour;
	outMinutes = now.tm_min;
	outSeconds = now.tm_sec;
}

bool CTimeUtils::IsDate(const int& year, const int& month, const int& day)
{
	tm now = GetLocalTime();

	if (year != -1 && now.tm_year != year)
		return false;

	if (month != -1 && now.tm_mon != month)
		return false;

	if (day != -1 && now.tm_mday != day)
		return false;

	return true;
}

void CTimeUtils::GetDate(int& outYears, int& outMonths, int& outDays)
{
	tm now = GetLocalTime();

	outYears = now.tm_year;
	outMonths = now.tm_mon;
	outDays = now.tm_mday;
}
