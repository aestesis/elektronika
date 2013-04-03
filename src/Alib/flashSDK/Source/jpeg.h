/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef JPEG_INCLUDED
#define JPEG_INCLUDED

#ifdef USE_JPEG

#include "global.h"
#include "jpeg/jpeglib.h"

// The player wants a quiet failure when a JEPG error occurs
struct jpeg_error_mgr* jpeg_afx_error(struct jpeg_error_mgr* jerr);
void afx_jpeg_null(struct jpeg_common_struct* cinfo);	// does nothing...

#endif

#endif


