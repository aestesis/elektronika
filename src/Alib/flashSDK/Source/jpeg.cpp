/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// 980226	mnk	In afx_error_exit(), throw AfxThrowNotSupportedException() instead
//				of AfxThrowMemoryException

#ifdef USE_JPEG

#include "jpeg.h"

//
// Helper Code for JPEG error manager
//

void afx_error_exit (j_common_ptr cinfo)
{
	#if 0
	/* We could postpone this until after returning, if we chose. */
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message) (cinfo, buffer);
	FLASHOUTPUT(buffer);
	#endif 
}

void afx_jpeg_null(j_common_ptr cinfo)
{
}

void afx_jpeg_null_int(j_common_ptr cinfo, int)
{
}

void afx_jpeg_null_char(j_common_ptr cinfo, char*)
{
}

GLOBAL struct jpeg_error_mgr *
jpeg_afx_error (struct jpeg_error_mgr * err)
{
  err->error_exit = afx_error_exit;
  err->emit_message = afx_jpeg_null_int;
  err->output_message = afx_jpeg_null;
  err->format_message = afx_jpeg_null_char;
  err->reset_error_mgr = afx_jpeg_null;

  err->trace_level = 0;		/* default = no tracing */
  err->num_warnings = 0;	/* no warnings emitted yet */
  err->msg_code = 0;		/* may be useful as a flag for "no error" */

  /* Initialize message table pointers */
  err->jpeg_message_table = 0;//jpeg_std_message_table;
  err->last_jpeg_message = 0;//(int) JMSG_LASTMSGCODE - 1;

  err->addon_message_table = 0;
  err->first_addon_message = 0;	/* for safety */
  err->last_addon_message = 0;

  return err;
}

#endif
