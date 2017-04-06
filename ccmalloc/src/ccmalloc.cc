/*----------------------------------------------------------------------------
 * (C) 1997-2003 Armin Biere 
 *
 *     $Id: ccmalloc.cc,v 1.11 2003/02/03 08:03:56 biere Exp $
 *----------------------------------------------------------------------------
 */

extern "C"
{
#include "ccmalloc.h"
};

class CCMalloc_InitAndReport
{
public:
  CCMalloc_InitAndReport ()
  {
    ccmalloc_static_initialization ();
  }

   ~CCMalloc_InitAndReport ()
  {
    ccmalloc_report ();
  }
};

static CCMalloc_InitAndReport ccmalloc_initAndReport;
