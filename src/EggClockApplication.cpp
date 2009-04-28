/*
============================================================================
 Name        : EggClockApplication.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Main application class
============================================================================
*/

// INCLUDE FILES
#include "EggClockApplication.h"
#include "EggClockDocument.h"

// ============================ MEMBER FUNCTIONS ===============================

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidEggClockApp = { 0xA00026FC };

CApaDocument* CEggClockApplication::CreateDocumentL()
{
  // Create an client document, and return a pointer to it
  return (static_cast<CApaDocument*>( CEggClockDocument::NewL( *this ) ) );
}

TUid CEggClockApplication::AppDllUid() const
{
  // Return the UID for the client application
  return KUidEggClockApp;
}

// End of file
