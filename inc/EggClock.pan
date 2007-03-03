/*
============================================================================
 Name        : EggClock.pan
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application panic codes
============================================================================
*/

#ifndef __EGGCLOCK_PAN__
#define __EGGCLOCK_PAN__

/** EggClock application panic codes */
enum TEggClockPanics
{
  EEggClockUi = 1
  // add further panics here
};

inline void Panic(TEggClockPanics aReason)
{
  _LIT(applicationName,"EggClock");
  User::Panic(applicationName, aReason);
}

#endif // __EGGCLOCK_PAN__
