
#pragma once

class CTimeUtils final
{
public:
    static bool IsTime( const int& hour, const int& minute, const int& second );
    static void GetTime( int& outHours, int& outMinutes, int& outSeconds );

    static bool IsDate( const int& year, const int& month, const int& day );
    static void GetDate( int& outYears, int& outMonths, int& outDays );
};
