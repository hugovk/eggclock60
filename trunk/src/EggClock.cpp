/*
============================================================================
 Name        : client.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application entry point
============================================================================
*/

// INCLUDE FILES
#include <eikstart.h>
#include "EggClockApplication.h"

LOCAL_C CApaApplication* NewApplication()
{
  return new CEggClockApplication;
}

GLDEF_C TInt E32Main()
{
  return EikStart::RunApplication( NewApplication );
}
