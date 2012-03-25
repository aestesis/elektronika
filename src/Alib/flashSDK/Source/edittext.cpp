/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#include "stdafx.h"

#include "sobject.h"

#include "stags.h"
#include "splay.h"
#include "splayer.h"
#include "edittext.h"
#include "curve.h"
#include "sstroker.h"


#include NATIVE_UTIL
#include NATIVE_PLAYERWND

// now defined in sobject.cpp
void ResolveFontName(char *result, const char *fontName);

enum
{
	kUnixSerifFont      = 1,
	kUnixSansFont       = 9,
	kUnixTypewriterFont = 17,
	kUnixBoldOffset     = 2,
	kUnixItalicOffset   = 2
};

#define BORDER_PAD 8

void AddRect(P_SRECT rect,
			 P_MATRIX mat, RColor* color,
			 DisplayList *display, REdge** edgeList,
			 SStroker* stroker);

// INVALID_CODE is used to represent a "missing" glyph which is not present
// in an outline font.  Characters with missing glyphs are rendered zero-width
// and are ignored when typed by the user.
#define INVALID_CODE 0xFFFF

// The default fallback font when no device font is found
#define DEFAULT_EDITTEXT_FONT "Times New Roman"
#define DEFAULT_EDITTEXT_J_FONT_1 "MS UI Gothic"
#define DEFAULT_EDITTEXT_J_FONT_2 "MS PÉSÉVÉbÉN"
#define DEFAULT_EDITTEXT_HEIGHT 360

#ifdef EDITTEXT

//------------------------------------------------------------------------
//
//    EditText
//

#ifndef min
	#define min(x, y) ((x)<(y)?(x):(y))
#endif

#ifndef max
	#define max(x, y) ((x)>(y)?(x):(y))
#endif

BOOL EditText::m_insertMode = TRUE;

EditText::EditText(SObject* obj)
{
	SParser parser;
	parser.Attach(obj->character->data, 0);     

	m_flags = parser.GetWord();

	if ( m_flags & seditTextFlagsHasFont )
	{
		parser.SkipBytes(4);
	}

	if ( m_flags & seditTextFlagsHasTextColor )
	{
		parser.GetColor(true);
	}

	if ( m_flags & seditTextFlagsHasMaxLength )
	{
		m_maxLength = parser.GetWord();
	}
	else
	{
		m_maxLength = 0;
	}

	if ( m_flags & seditTextFlagsHasLayout )
	{
		m_align = parser.GetByte();
		parser.SkipBytes(8);
	}
	else
	{
		m_align = stextAlignLeft;
	}

	m_variable = parser.GetStringP();

	if ( m_flags & seditTextFlagsHasText )
	{
		m_initialText = parser.GetStringP();
	}
	else
	{
		m_initialText = NULL;
	}

	m_obj             = obj;
	m_character       = obj->character;
	m_clickTime       = 0;
	m_selecting       = FALSE;
	m_buffer          = new U16[1];
	m_buffer[0]       = '\0';
	m_length          = 0;
	m_selectionStart  = 0;
	m_selectionEnd    = 0;
	m_hscroll         = 0;
	m_vscroll         = 0;
	m_mouseIsDown     = FALSE;
	m_doFindCursor    = FALSE;
	m_mousePosition.x = 0;
	m_mousePosition.y = 0;
	m_lineX           = NULL;
	m_drawn           = FALSE;
	m_lineStarts      = NULL;
	m_numLines        = 0;
	m_atLineEnd       = FALSE;
	m_heldLeadByte    = -1;
	devCharWidths     = NULL;
	devLineSpacing    = 0;
	devAscent         = 0;

	RectSetEmpty(&devBounds);
}

EditText::~EditText()
{
	delete [] m_buffer;
	delete [] m_lineStarts;
	delete [] m_lineX;
	delete [] devCharWidths;
}

	#ifdef EDITTEXT
BOOL PlayerIsFontAvailable(const char *fontName)
{
	return FALSE;
}

void EditText::GetFontDesc(FontDesc& fontDesc)
{
	SParser parser;
	parser.Attach(m_character->data, 0);

	U16 flags = parser.GetWord();

	U16 fontTag = 0;

	if ( flags & seditTextFlagsHasFont )
	{
		fontTag = parser.GetWord();
		fontDesc.height = parser.GetWord();
	}

	ScriptPlayer* player = m_character->player;

	// On Unix, if no font was specified, fall back on built-in Times Roman.
	if ( !fontTag )
	{
		fontTag = kUnixSerifFont;
		player = &player->splayer->builtInFontsPlayer;
	}

	if ( fontTag )
	{
		// Find the font character
		SCharacter* font = player->FindCharacter(fontTag);
		char fontName[256];

		if ( font )
		{
			// Get the font name
			if ( font->tagCode == stagDefineFont2 )
			{
				parser.Attach(font->data - font->font.nDataOffset, 0);
				int len = parser.GetByte();
				parser.GetData(fontName, len);
				fontName[len] = 0;

				fontDesc.boldFlag = (font->font.flags & sfontFlagsBold) != 0;
				fontDesc.italicFlag = (font->font.flags & sfontFlagsItalic) != 0;
				fontDesc.japaneseFlag = (font->font.flags & sfontFlagsShiftJIS) != 0;

			}
			else if ( font->font.infoData )
			{
				parser.Attach(font->font.infoData, 0);
				int len = parser.GetByte();
				parser.GetData(fontName, len);
				fontName[len] = 0;

				// Get the font flags
				U8 flags = parser.GetByte();

				fontDesc.boldFlag = (flags & tfontBold) != 0;
				fontDesc.italicFlag = (flags & tfontItalic) != 0;
				fontDesc.japaneseFlag = (flags & 0xf) == tfontShiftJIS;
			}

			BOOL useGlyphFont = IsFontIncluded();

			// If outlines aren't included, we need to fall back
			// on a built-in font
			if ( !useGlyphFont )
			{
				if ( !strcmp(fontName, SANS_NAME) )
				{
					fontTag = kUnixSansFont;
				}
				else if ( !strcmp(fontName, TYPEWRITER_NAME) )
				{
					fontTag = kUnixTypewriterFont;
				}
				else
				{
					fontTag = kUnixSerifFont;
				}
				if ( fontDesc.boldFlag )
				{
					fontTag += kUnixBoldOffset;
				}
				if ( fontDesc.italicFlag )
				{
					fontTag += kUnixItalicOffset;
				}
				font = m_character->player->splayer->builtInFontsPlayer.FindCharacter(fontTag);
				useGlyphFont = TRUE;
			}

			// Figure out the code table offset, if applicable
			if ( useGlyphFont )
			{
				U16 g = font->font.nGlyphs;
				U8* s;
				if ( font->font.flags & sfontFlagsWideOffsets )
				{
					s = font->data + 4*g;
					fontDesc.codeOffset = (U32)s[0] | ((U32)s[1]<<8) | ((U32)s[2]<<16) | ((U32)s[3]<<24);
				}
				else
				{
					s = font->data + 2*g;
					fontDesc.codeOffset = (U16)s[0] | ((U16)s[1]<<8);
				}
			}
			else
			{
				// Not using the outlines so ignore the code table
				fontDesc.codeOffset = -1;

				// We must determine if this font actually exists.
				// If it doesn't, fall back on a predetermined font.
				if ( !PlayerIsFontAvailable(fontDesc.fontName) )
				{
					if ( fontDesc.japaneseFlag )
					{
						// MS UI Gothic is preferred, but fallback on MS Gothic if not present
						if ( PlayerIsFontAvailable(DEFAULT_EDITTEXT_J_FONT_1) )
						{
							strcpy(fontDesc.fontName, DEFAULT_EDITTEXT_J_FONT_1);
						}
						else
						{
							strcpy(fontDesc.fontName, DEFAULT_EDITTEXT_J_FONT_2);
						}
					}
					else
					{
						strcpy(fontDesc.fontName, DEFAULT_EDITTEXT_FONT);
					}
				}
			}

			// Success!  Return this font information.
			fontDesc.font = font;
			return;
		}
	}

	// No font info in the text field tag, so we'll have to use defaults.
	strcpy(fontDesc.fontName, DEFAULT_EDITTEXT_FONT);
	fontDesc.boldFlag = FALSE;
	fontDesc.italicFlag = FALSE;
	fontDesc.japaneseFlag = FALSE;
	fontDesc.height = DEFAULT_EDITTEXT_HEIGHT;
	fontDesc.font = NULL;
	fontDesc.codeOffset = -1;
}

	#endif


BOOL PlayerIsMultiByte();
BOOL PlayerIsLeadByte(U8 ch);

BOOL EditText::IsMultiByte()
{
	return PlayerIsMultiByte();
}

BOOL EditText::IsLeadByte(U8 ch)
{
	return PlayerIsLeadByte(ch);
}

U16 * EditText::MBCSToWide(char *buffer, int index, int count)
{
	U16 *result = new U16[count + 1];

	U16 *dst = result;
	U8 *src = (U8*)buffer + index;

	while ( count )
	{
		if ( IsLeadByte(*src) && count > 1 )
		{
			*dst++ = ((U16)*src<<8) | *(src+1);
			src += 2;
			count -= 2;
		}
		else
		{
			*dst++ = *src++;
			count--;
		}
	}
	*dst = 0;

	return result;
}

char * EditText::WideToMBCS(U16 *buffer, int index, int count)
{
	char *result = new char[count * 2 + 1];

	U16 *src = buffer + index;
	char *dst = result;

	while ( count-- )
	{
		if ( *src > 0xff )
		{
			*dst++ = (*src>>8)&0xff;
			*dst++ = *src&0xff;
		}
		else
		{
			*dst++ = (char)(*src);
		}
		src++;
	}

	*dst = 0;
	return result;
}

int WideStrLen(const U16 *buf)
{
	int len = 0;
	while ( *buf++ )
	{
		len++;
	}
	return len;
}

void WideStrCopy(U16 *dst, const U16 *src)
{
	while ( *dst++ = *src++ );
}

char* EditText::GetBuffer()
{
	return WideToMBCS(m_buffer, 0, m_length);
}

void EditText::SetBuffer(char *text)
{
	U16 *newBuffer = MBCSToWide(text, 0, strlen(text));
	if ( newBuffer )
	{
		delete [] m_buffer;
		m_buffer = newBuffer;
		m_length = WideStrLen(m_buffer);
		m_selectionStart = min(m_length, m_selectionStart);
		m_selectionEnd   = min(m_length, m_selectionEnd);
	}
}

void EditText::AutoScroll()
{
	if ( !m_mouseIsDown )
	{
		return;
	}

	int maxVScroll = CalcMaxVScroll();

	SRECT bounds;
	SPOINT pt = m_mousePosition;

	{
		bounds = m_character->bounds;
		RectInset(2*onePoint, &bounds);

		// Transform the mouse position into the local
		// coordinate system of the edit text
		MATRIX invmat;
		MatrixInvert(&m_mat, &invmat);
		if ( m_character->player->display->antialias )
		{
			pt.x *= 4;
			pt.y *= 4;
		}
		MatrixTransformPoint(&invmat, &pt, &pt);
	}

	// if the selection does not extend to the start of the buffer
	// and the mouse is out of bounds on the top, dec vscroll
	if ( pt.y < bounds.ymin )
	{
		if ( m_selectionEnd > 0 && m_vscroll > 0 )
		{
			m_vscroll--;
		}
	}
	if ( pt.y > bounds.ymax )
	{
		if ( m_selectionEnd < m_length && m_vscroll < m_numLines-1 )
		{
			m_vscroll++;
		}
	}

	if ( m_vscroll > maxVScroll )
	{
		m_vscroll = maxVScroll;
	}

	// Find the line that the cursor is on
	int lineStart = FindLineStart(m_selectionEnd);

	// Find the length of the line the cursor is on
	int lineLength = FindLineEnd(m_selectionEnd) - lineStart;

	// if the selection does not extend to the end of the buffer
	// and the mouse is out of bounds ont he bottom, inc vscroll

	// if the selection does not extend to the end of line and
	// the mouse is out of bounds on the right, inc hscroll

	// if the selection does not extend to the start of the line and
	// the mouse is out of bounds on the left, dec hscroll

	if ( !(m_flags & seditTextFlagsWordWrap) && m_align != stextAlignRight && m_align != stextAlignCenter )
	{
		int lineSelectionIndex = m_selectionEnd - lineStart;
		if ( pt.x > bounds.xmax && lineSelectionIndex < lineLength && m_hscroll < lineLength )
		{
			m_hscroll++;
		}
		if ( pt.x < bounds.xmin && lineSelectionIndex > 0 && m_hscroll > 0 )
		{
			m_hscroll--;
		}
	}
}

void EditText::FindCursor(SRECT* devBounds, int *devCharWidths)
{
	// First, find the line that the cursor is on.
	int lineno = FindLineNumber(m_selectionEnd);
	// Scroll, if necessary, to keep the line in view
	if ( m_vscroll > lineno )
	{
		m_vscroll = lineno;
	}
	else
	{
		int visibleLines = CalcVisibleLines();
		if ( lineno-m_vscroll >= visibleLines )
		{
			m_vscroll = lineno-visibleLines+1;
		}
	}

	// Scroll, if necessary, to keep m_selectionEnd in view
	// Find the position of the beginning of the line the
	// cursor is on
	int startLine = FindLineStart(m_selectionEnd);
	int lineSelectIndex = m_selectionEnd-startLine;

	// Find the x position of m_selectionEnd
	if ( !(m_flags & seditTextFlagsWordWrap) )
	{
		if ( m_hscroll >= lineSelectIndex )
		{
			m_hscroll = lineSelectIndex;
			// Scroll the cursor so that it's at
			// the 1/4 mark of the text field's width
			int x = devBounds->xmin;
			int target = devBounds->xmin + (devBounds->xmax - devBounds->xmin) / 4;
			while ( x < target && m_hscroll )
			{
				m_hscroll--;
				x += devCharWidths[startLine+m_hscroll];
			}
		}
		else
		{
			int x = devBounds->xmin;
			for ( int i=m_hscroll; i<lineSelectIndex; i++ )
			{
				x += devCharWidths[startLine+i];
			}
			// If we're out of view on the right, scroll to the right
			while ( x > devBounds->xmax )
			{
				x -= devCharWidths[startLine+m_hscroll];
				m_hscroll++;
			}
		}
	}
}

void EditText::ClearSelection()
{
	m_selectionStart = m_selectionEnd = 0;
}

void EditText::Forward(int wholeWord, int extend)
{
	if ( wholeWord )
	{
		// Forward one word
		// Skip initial whitespace
		while ( m_selectionEnd < m_length && LineBreakOK_Index(m_selectionEnd) )
		{
			m_selectionEnd++;
		}
		// Skip until whitespace
		while ( m_selectionEnd < m_length && !LineBreakOK_Index(m_selectionEnd) )
		{
			m_selectionEnd++;
		}
	}
	else
	{
		// Forward one character
		if ( m_selectionEnd < m_length )
		{
			m_selectionEnd++;
		}
	}
	if ( !extend )
	{
		m_selectionStart = m_selectionEnd;
	}
	m_atLineEnd = FALSE;
}

void EditText::Backspace()
{
	if ( m_selectionStart == m_selectionEnd )
	{
		// No selection, delete character before the cursor.
		if ( m_selectionEnd > 0 )
		{
			WideStrCopy(m_buffer+m_selectionEnd-1, m_buffer+m_selectionEnd);
			m_selectionEnd--;
			m_selectionStart--;
			m_length--;
			m_atLineEnd = FALSE;
		}
	}
	else
	{
		// There is a selection, delete it.
		DeleteSelection();
	}
}

void EditText::Backward(int wholeWord, int extend)
{
	if ( wholeWord )
	{
		// By one word
		// Skip initial whitespace
		while ( m_selectionEnd > 0 && LineBreakOK_Index(m_selectionEnd-1) )
		{
			m_selectionEnd--;
		}
		// Skip until whitespace
		while ( m_selectionEnd > 0 && !LineBreakOK_Index(m_selectionEnd-1) )
		{
			m_selectionEnd--;
		}
	}
	else
	{
		// By one character
		if ( m_selectionEnd > 0 )
		{
			m_selectionEnd--;
		}
	}
	if ( !extend )
	{
		m_selectionStart = m_selectionEnd;
	}
	m_atLineEnd = FALSE;
}

void EditText::DeleteSelection()
{
	if ( m_selectionStart != m_selectionEnd )
	{
		int A, B;
		if ( m_selectionStart < m_selectionEnd )
		{
			A = m_selectionStart;
			B = m_selectionEnd;
		}
		else
		{
			A = m_selectionEnd;
			B = m_selectionStart;
		}
		int count = B - A;
		WideStrCopy(m_buffer+A, m_buffer+B);
		m_selectionStart = m_selectionEnd = A;
		m_length -= count;
		m_atLineEnd = FALSE;
	}
}

int EditText::CalcVisibleLines()
{
	if ( !devLineSpacing )
	{
		return 0;
	}

	SRECT bounds = m_character->bounds;
	RectInset(2*onePoint, &bounds);

	// Calculate the max value for vscroll based on # of lines
	int visibleLines = (devBounds.ymax-devBounds.ymin-devAscent)/devLineSpacing;
	if ( visibleLines < 0 )
	{
		visibleLines = 0;
	}
	visibleLines++;
	return visibleLines;
}

// Calculate the max value for vscroll based on # of lines
int EditText::CalcMaxVScroll()
{
	int maxVScroll = m_numLines-CalcVisibleLines();
	if ( maxVScroll < 0 )
	{
		maxVScroll = 0;
	}
	return maxVScroll;
}

int EditText::FindLineNumber(int pos)
{
	// Search the line starts array.  Find a line
	// with an index greater than pos.
	int i;
	for ( i=0; i<m_numLines-1; i++ )
	{
		if ( pos < m_lineStarts[i+1] )
		{
			break;
		}
	}
	return i;
}

int EditText::FindLineEnd(int pos)
{
	int lineno = FindLineNumber(pos);

	if ( lineno < m_numLines-1 )
	{
		lineno++;
	}
	else
	{
		return m_length;
	}

	int lineEnd = m_lineStarts[lineno];

	if ( lineEnd != 0 )
	{
		// Not the beginning or end of the buffer
		// Try to back up
		lineEnd--;
	}

	return lineEnd;
}

void EditText::HandleKeyDown(int key, int modifiers, NativePlayerWnd* native )
{
	if ( !m_drawn )
	{
		return;
	}

	BOOL readOnly = ((m_flags & seditTextFlagsReadOnly) != 0);

	switch ( key )
	{
	case Keyboard::ID_KEY_CUT:
		if ( !readOnly )
		{
			CopyToClipboard( native );
			DeleteSelection();
		}
		break;
	case Keyboard::ID_KEY_COPY:
		if ( m_selectionStart != m_selectionEnd )
		{
			CopyToClipboard( native );
		}
		break;
	case Keyboard::ID_KEY_PASTE:
		if ( !readOnly )
		{
			DeleteSelection();
			PasteFromClipboard( native );
		}
		break;
	case Keyboard::ID_KEY_BACKSPACE:
		if ( !readOnly )
		{
			Backspace();
		}
		break;
	case Keyboard::ID_KEY_SELECT_ALL:
		m_selectionStart = m_length;
		m_selectionEnd = 0;
		m_hscroll = 0;
		break;
	case Keyboard::ID_KEY_UP:
		{
			if ( m_selectionEnd != 0 && m_atLineEnd )
			{
				m_selectionEnd--;
			}

			// Find our position on this line
			int lineStart = FindLineStart(m_selectionEnd);
			int index = m_selectionEnd - lineStart;

			// Find the beginning of the previous line
			int target = lineStart;
			if ( target )
			{
				target = FindLineStart(target-1);
			}

			// New position is index'th character of the line
			int lineEnd = FindLineEnd(target);
			target += index;
			if ( target > lineEnd )
			{
				target = lineEnd;
			}

			if ( modifiers & Keyboard::ID_KEY_SHIFT )
			{
				// If shift-home, extend selection to beginning of line
				m_selectionEnd = target;
			}
			else
			{
				// If home, de-select and move cursor to beginning of line
				m_selectionStart = m_selectionEnd = target;
			}

			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_DOWN:
		{
			if ( m_atLineEnd )
			{
				if ( m_selectionEnd )
				{
					m_selectionEnd--;
				}
			}

			// Find our position on this line
			int lineStart = FindLineStart(m_selectionEnd);
			int index = m_selectionEnd - lineStart;

			// Find the beginning of the next line
			int target = FindLineEnd(m_selectionEnd);
			if ( target < m_length )
			{
				target++;
			}

			// New position is index'th character of the line
			int lineEnd = FindLineEnd(target);
			target += index;
			if ( target > lineEnd )
			{
				target = lineEnd;
			}

			if ( modifiers & Keyboard::ID_KEY_SHIFT )
			{
				// If shift-home, extend selection to beginning of line
				m_selectionEnd = target;
			}
			else
			{
				// If home, de-select and move cursor to beginning of line
				m_selectionStart = m_selectionEnd = target;
			}

			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_LEFT:
		Backward((modifiers & Keyboard::ID_KEY_CTRL) != 0,
				 (modifiers & Keyboard::ID_KEY_SHIFT) != 0);
		break;
	case Keyboard::ID_KEY_RIGHT:
		Forward((modifiers & Keyboard::ID_KEY_CTRL) != 0,
				(modifiers & Keyboard::ID_KEY_SHIFT) != 0);
		break;
	case Keyboard::ID_KEY_PAGE_UP:
		{
			if ( modifiers & Keyboard::ID_KEY_SHIFT )
			{
				// If shift-home, extend selection to beginning of line
				m_selectionEnd = 0;
			}
			else
			{
				// If home, de-select and move cursor to beginning of line
				m_selectionStart = m_selectionEnd = 0;
			}
			m_atLineEnd = FALSE;
		}   
		break;
	case Keyboard::ID_KEY_PAGE_DOWN:
		{
			if ( modifiers & Keyboard::ID_KEY_SHIFT )
			{
				// If shift-home, extend selection to beginning of line
				m_selectionEnd = m_length;
			}
			else
			{
				// If home, de-select and move cursor to beginning of line
				m_selectionStart = m_selectionEnd = m_length;
			}
			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_HOME:
		{
			if ( m_selectionEnd != 0 && m_atLineEnd )
			{
				m_selectionEnd--;
			}

			int target;
			if ( modifiers & Keyboard::ID_KEY_CTRL )
			{
				// Jump to beginning of buffer
				target = 0;
			}
			else
			{
				// Jump to beginning of line
				target = FindLineStart(m_selectionEnd);
			}   
			if ( modifiers & Keyboard::ID_KEY_SHIFT )
			{
				// If shift-home, extend selection to beginning of line
				m_selectionEnd = target;
			}
			else
			{
				// If home, de-select and move cursor to beginning of line
				m_selectionStart = m_selectionEnd = target;
			}
			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_END:
		{
			if ( !m_atLineEnd )
			{
				int target;
				if ( modifiers & Keyboard::ID_KEY_CTRL )
				{
					// Jump to end of buffer
					target = m_length;
				}
				else
				{
					// Jump to end of line
					target = FindLineEnd(m_selectionEnd);
					if ( LineBreakOK_Index(target) )
					{
						target++;
						m_atLineEnd = TRUE;
					}
				}
				// If shift-end, extend selection to end of line
				if ( modifiers & Keyboard::ID_KEY_SHIFT )
				{
					m_selectionEnd = target;
				}
				else
				{
					// Move to end of line
					m_selectionStart = m_selectionEnd = target;
				}
			}
		}
		break;
	case Keyboard::ID_KEY_INSERT:
		m_insertMode = !m_insertMode;
		break;
	case Keyboard::ID_KEY_DELETE:
		if ( !readOnly )
		{

			if ( (modifiers & Keyboard::ID_KEY_CTRL) != 0 )
			{
				// Ctrl-Shift-Delete means nothing
				if ( (modifiers & Keyboard::ID_KEY_SHIFT) == 0 )
				{
					// If there is a selection, do regular delete.
					if ( m_selectionStart == m_selectionEnd )
					{
						Forward(TRUE, TRUE);
						Delete();
					}
					else
					{
						Delete();
					}
				}
			}
			else
			{
				Delete();
			}
			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_CLEAR:
		if ( !readOnly )
		{
			DeleteSelection();
			m_atLineEnd = FALSE;
		}
		break;
	case Keyboard::ID_KEY_ENTER:
		if ( !readOnly && (m_flags & seditTextFlagsMultiline) != 0 )
		{
			Insert(editTextNewline);
		}
		break;
	default:
		if ( !readOnly )
		{
			Insert(key);
		}
		break;
	}

	m_doFindCursor = TRUE;
}

void EditText::CopyToClipboard( NativePlayerWnd* native )
{
	int A, B;
	if ( m_selectionStart < m_selectionEnd )
	{
		A = m_selectionStart;
		B = m_selectionEnd;
	}
	else
	{
		A = m_selectionEnd;
		B = m_selectionStart;
	}
	int count = B - A;

	#ifdef EDITTEXT_CLIPBOARD_WIDE
		U16 *buffer = new U16[count * 2 + 1];
		U16 *src = m_buffer+A;
		U16 *dst = buffer;
		while ( count-- )
		{
			*dst++ = *src++;
		}
		*dst = 0;
		native->SetStringToClipboard(buffer);
		delete [] buffer;

	#else
		// Translate linefeeds to carriage returns
		char *buffer = new char[count * 2 + 1];
		U16 *src = m_buffer+A;
		char *dst = buffer;
		while ( count-- )
		{
// 			if ( *src == editTextNewline )
// 			{
//  				*dst++ = '\n';
// 			}
// 			else 
			if ( *src > 0xff )
			{
				*dst++ = (*src>>8)&0xff;
				*dst++ = *src&0xff;
			}
			else
			{
				*dst++ = *src;
			}
			src++;
		}
		*dst = 0;

		native->SetStringToClipboard(buffer);
		delete [] buffer;
	#endif
}

void EditText::PasteFromClipboard( NativePlayerWnd* native)
{
	#ifdef EDITTEXT_CLIPBOARD_WIDE
		U16* clipboard;
	#else
		char* clipboard;
	#endif

	clipboard = native->GetStringFromClipboard();
	if ( clipboard )
	{
		#ifdef EDITTEXT_CLIPBOARD_WIDE
			InsertWideChars( clipboard, WideStrLen( clipboard) );
			delete [] clipboard;
		#else
			Insert( clipboard, strlen( clipboard ) );
			FreeStr( clipboard );
		#endif
	}
	m_atLineEnd = FALSE;
}

void EditText::Insert(char *s, int len)
{
	U16 *wideChars = MBCSToWide(s, 0, len);
	InsertWideChars(wideChars, WideStrLen(wideChars));
	delete [] wideChars;
}

void EditText::InsertWideChars(U16 *s, int insertLen)
{
	U16 *buffer = s;
	int count;

	if ( !insertLen )
	{
		goto exit_gracefully;
	}

	// If using font outlines, purge any characters not in the font
	{
		buffer = new U16[insertLen];
		if ( !buffer )
		{
			goto exit_gracefully;
		}
		memcpy(buffer, s, insertLen * sizeof(U16));
		FontDesc fontDesc;
		GetFontDesc(fontDesc);
		U16 *dst = buffer, *src = buffer;
		while ( insertLen-- )
		{
			U16 code = *src;
			if ( code == editTextNewline || code == editTextSoftNewline )
			{
				*dst++ = code;
			}
			else
			{
				U16 g = FindGlyph(code, fontDesc);
				if ( g == INVALID_CODE )
				{
					// If it's uppercase/lowercase, switch case and give
					// it another go.
					if ( code >= 'A' && code <= 'Z' )
					{
						code += ('a' - 'A');
						g = FindGlyph(code, fontDesc);
					}
					else if ( code >= 'a' && code <= 'z' )
					{
						code -= ('a' - 'A');
						g = FindGlyph(code, fontDesc);
					}
				}
				if ( g != INVALID_CODE )
				{
					*dst++ = code;
				}
			}
			src++;
		}
		insertLen = dst - buffer;
	}

	m_atLineEnd = FALSE;

	// Delete any selection
	DeleteSelection();

	// Handle overwrite mode.
	if ( !m_insertMode )
	{
		int writeEnd = m_selectionEnd + insertLen;
		if ( writeEnd > m_length )
		{
			// If there is a maximum length setting, check
			// against it first.
			if ( m_maxLength != 0 && writeEnd > m_maxLength )
			{
				insertLen -= (writeEnd - m_maxLength);
				if ( insertLen <= 0 )
				{
					goto exit_gracefully;
				}
				writeEnd = m_maxLength;
			}
			Expand(writeEnd);
			m_length = writeEnd;
			m_buffer[m_length] = '\0';
		}

		memcpy(m_buffer+m_selectionEnd, buffer, sizeof(U16) * insertLen);
		m_selectionStart += insertLen;
		m_selectionEnd += insertLen;
		goto exit_gracefully;
	}

	// We're in insert mode.
	// If there is a maximum length setting, check
	// against it first.
	if ( m_maxLength != 0 && m_length + insertLen > m_maxLength )
	{
		// Too long, shorten the string
		insertLen = m_maxLength - m_length;
		if ( insertLen <= 0 )
		{
			goto exit_gracefully;
		}
	}

	// Expand the string
	Expand(m_length + insertLen);

	// Move all characters after the cursor.
	count = m_length - m_selectionEnd + 1;
	if ( count > 0 )
	{
		memmove(m_buffer + m_selectionEnd + insertLen,
				m_buffer + m_selectionEnd,
				sizeof(U16) * count);
	}

	// Bump up the length
	m_length += insertLen;

	// Place the new characters
	memcpy(m_buffer + m_selectionEnd, buffer, sizeof(U16) * insertLen);

	m_selectionEnd += insertLen;
	m_selectionStart += insertLen;

	exit_gracefully:
	if ( buffer != s )
	{
		delete [] buffer;
	}
}

void EditText::Expand(int size)
{
	if ( size > m_length )
	{
		U16 *newBuffer = new U16[size + 1];
		memset(newBuffer, 0, sizeof(U16) * (size + 1));
		if ( m_buffer )
		{
			WideStrCopy(newBuffer, m_buffer);
			delete [] m_buffer;
		}
		m_buffer = newBuffer;
	}
}

void EditText::Delete()
{
	if ( m_selectionStart == m_selectionEnd )
	{
		// There is no selection, delete character under cursor
		if ( m_selectionEnd < m_length )
		{
			WideStrCopy(m_buffer+m_selectionEnd, m_buffer+m_selectionEnd+1);
			m_length--;
		}
	}
	else
	{
		// There is a selection, delete it.
		DeleteSelection();
	}
}


	#define DOUBLE_CLICK_TIME 500

void EditText::DoMouse(SPOINT* pt, BOOL mouseIsDown)
{
	if ( !m_drawn )
	{
		return;
	}

	if ( m_flags & seditTextFlagsNoSelect )
	{
		return;
	}

	SRECT bounds;
// #ifndef _UNIX
// 	if (IsFontIncluded())
// #endif
	{

		bounds = m_character->bounds;
		RectInset(2*onePoint, &bounds);
	}

	BOOL shiftKey;
	shiftKey = FALSE;

	m_atLineEnd = FALSE;

	// Check for double-click
	if ( mouseIsDown && m_mousePosition.x == pt->x && m_mousePosition.y == pt->y )
	{
	#ifndef SPRITE_XTRA
		// Check if less than DOUBLE_CLICK_TIME ms has elapsed
		if ( GetTimeMSec() - m_clickTime < DOUBLE_CLICK_TIME )
		{
			// Select entire word
			m_selectionStart = m_selectionEnd;
			// Bump selection end up to nearest word boundary
			U16 *ptr = m_buffer + m_selectionEnd;
			while ( m_selectionEnd < m_length )
			{
				if ( LineBreakOK_Index(m_selectionEnd) || *ptr == editTextNewline || *ptr == editTextSoftNewline )
				{
					break;
				}
				ptr++;
				m_selectionEnd++;
			}
			// Back selection start down to nearest word boundary
			ptr = m_buffer + m_selectionStart;
			while ( m_selectionStart > 0 )
			{
				if ( LineBreakOK_Index(m_selectionStart-1) || *(ptr-1) == editTextNewline || *(ptr-1) == editTextSoftNewline )
				{
					break;
				}
				ptr--;
				m_selectionStart--;
			}
			return;
		}
	#endif
	}

	m_mouseIsDown = mouseIsDown;
	m_mousePosition = *pt;

	m_clickTime = GetTimeMSec();

	SPOINT localpt = *pt;

	{
		// Transform the mouse position into local coordinates
		MATRIX invmat;
		MatrixInvert(&m_mat, &invmat);

		if ( m_character->player->display->antialias )
		{
			localpt.x *= 4;
			localpt.y *= 4;
		}
		MatrixTransformPoint(&invmat, &localpt, &localpt);
	}

	int visibleLines = CalcVisibleLines();
	if ( !visibleLines )
	{
		return;
	}

	// Find the line that the mouse is over
	int lineno = (localpt.y-bounds.ymin)/devLineSpacing;
	lineno = max(0, min(lineno, visibleLines-1));
	lineno += m_vscroll;
	if ( lineno > m_numLines )
	{
		lineno = m_numLines;
	}

	int index = m_lineStarts[lineno];
	int lineEnd = FindLineEnd(index);

	index += m_hscroll;
	if ( index > lineEnd )
	{
		index = lineEnd;
	}

	// Find the character in the text right before
	// the position of the mouse click
	int x = m_lineX[lineno];
	for ( ; m_buffer[index]; index++ )
	{
		if ( index >= lineEnd )
		{
			if ( LineBreakOK_Index(index) )
			{
				index++;
				m_atLineEnd = TRUE;
			}
			break;
		}
		if ( localpt.x < x+devCharWidths[index]/2 )
		{
			break;
		}
		x += devCharWidths[index];
		if ( x > bounds.xmax )
		{
			break;
		}
	}

	m_selectionEnd = index;
	if ( !m_selecting )
	{
		m_selecting = TRUE;
		if ( !shiftKey )
		{
			m_selectionStart = index;
		}
	}
	else
	{
		if ( !mouseIsDown )
		{
			m_selecting = FALSE;
		}
	}
}

BOOL EditText::BreakEnglish(U16 ch1, U16 ch2)
{
	return ch1 == ' ' || ch1 == '-';
}

enum
{
	J_END   = 0x00,
	J_SETHI = 0x01,
	J_RANGE = 0x02
};

static BOOL JCheckTable(U8 *table, U16 ch)
{
	U8 ch_hi = (ch >> 8) & 0xFF;
	U8 ch_lo = (ch & 0xFF);

	U8 table_hi = 0;

	while ( *table )
	{
		if ( *table == 1 )
		{
			// Select a new high byte
			table_hi = *++table;
		}
		else if ( *table == 2 )
		{
			// Expect a range
			U8 range_lo = *++table;
			U8 range_hi = *++table;
			if ( ch_hi == table_hi && ch_lo >= range_lo && ch_lo <= range_hi )
			{
				return TRUE;
			}
		}
		else
		{
			// Regular character
			if ( ch_hi == table_hi && ch_lo == *table )
			{
				return TRUE;
			}
		}
		table++;
	}

	return FALSE;
}

BOOL EditText::LineBreakOK_Index(int index)
{
	if ( index < 0 || index >= m_length )
	{
		return FALSE;
	}
	if ( index == m_length-1 )
	{
		return LineBreakOK(m_buffer[index], 0);
	}
	return LineBreakOK(m_buffer[index], m_buffer[index+1]);
}

BOOL EditText::LineBreakOK(U16 ch1, U16 ch2)
// ch1: Last character of current line
// ch2: First character of next line
// return: 
// 	TRUE  if it is ok to break this two characters
// 	FALSE do not break the characters
{
	return BreakEnglish(ch1, ch2);
}

void EditText::CalculateLineStarts(int *devCharWidths, int width, int indent)
{
	U16 *ptr = m_buffer;
	int x = indent;
	int maxLineStarts = 16;
	BOOL done = FALSE;
	U16 *currentBreak = NULL;

	delete [] m_lineStarts;
	m_lineStarts = new int[maxLineStarts];
	m_numLines = 0;
	m_lineStarts[0] = 0;

	BOOL breakHere = FALSE;
	int nextIndent = 0;

	while ( !done )
	{
		switch ( *ptr )
		{
		case 0:
			breakHere = TRUE;
			done = TRUE;
			break;

		case editTextNewline:
			ptr++;
			breakHere = TRUE;
			x = indent;
			break;

		case editTextSoftNewline:
			ptr++;
			breakHere = TRUE;
			x = 0;
			break;

		default:
			// Regular character
			x += devCharWidths[ptr-m_buffer];
			if ( (m_flags & seditTextFlagsWordWrap) != 0 && x > width )
			{
				breakHere = TRUE;
				x = 0;
				if ( currentBreak )
				{
					ptr = currentBreak + 1;
				}
			}
			else
			{
				// Is this character a potential break position?
				if ( LineBreakOK(ptr[0], ptr[1]) )
				{
					currentBreak = ptr;
				}

				ptr++;
			}
		}

		if ( breakHere )
		{
			if ( m_numLines >= maxLineStarts-1 )
			{
				maxLineStarts *= 2;
				int *newLineStarts = new int[maxLineStarts];
				memcpy(newLineStarts, m_lineStarts, sizeof(int) * (m_numLines+1));
				delete [] m_lineStarts;
				m_lineStarts = newLineStarts;
			}

			m_lineStarts[++m_numLines] = ptr - m_buffer;

			currentBreak = NULL;
			breakHere    = FALSE;
		}
	}
}

// Helper for BuildEdges
RColor* CreateEditTextColor(int red,
							int green,
							int blue,
							int alpha,
							int layer,
							STransform* x,
							SObject* obj)
{
	if ( obj->clipDepth )
	{
		return NULL;
	}

	RColor* color = obj->display->CreateColor();
	if ( !color )
	{
		return NULL;
	}

	color->SetUp(&obj->display->raster);
	color->nextColor = obj->colors;
	obj->colors = color;

	color->order = layer;
	FLASHASSERT(color->order < 0x10000);

	color->rgb.alpha = alpha;
	color->rgb.red = red;
	color->rgb.green = green;
	color->rgb.blue = blue;

	if ( x->cxform.HasTransform() )
	{
		x->cxform.Apply(&color->rgb);
	}

	color->transparent = PreMulAlpha(&color->rgb);

	if ( obj->display->raster.bits )
	{
		color->BuildCache();
	}

	return color;
}

// Helper for BuildEdges
void StrokeCursor(SObject* obj,
				  P_MATRIX mat,
				  int cursorX,
				  int cursorTop,
				  int cursorBottom,
				  RColor* color,
				  SStroker* stroker)
{
	// Transform cursorTop and cursorBottom into global space
	SPOINT topLeft;
	topLeft.x = cursorX;
	topLeft.y = cursorTop;
	MatrixTransformPoint(mat, &topLeft, &topLeft);

	SPOINT bottomRight;
	bottomRight.x = cursorX;
	bottomRight.y = cursorBottom;
	MatrixTransformPoint(mat, &bottomRight, &bottomRight);

	stroker->BeginStroke(1, color);

	CURVE c;
	CurveSetLine(&topLeft, &bottomRight, &c);
	stroker->AddStrokeCurve(&c);
	stroker->EndStroke();
}

// FindGlyph locates the glyph for a particular character code in a font
// and returns the glyph index
U16 EditText::FindGlyph(U16 targetCode, FontDesc& fontDesc)
{
	SCharacter* font = fontDesc.font;
	U8 *codeTable = font->data + fontDesc.codeOffset;

	// Find the glyph for this character using binary search
	int lo = 0;
	int hi = font->font.nGlyphs-1;
	if ( font->font.flags & sfontFlagsWideCodes )
	{
		while ( lo <= hi )
		{
			int pivot = (lo+hi)>>1;
			U16 testCode = (U16)codeTable[pivot*2] | ((U16)codeTable[pivot*2+1]<<8);
			if ( testCode == targetCode )
			{
				return pivot;
			}
			else if ( targetCode < testCode )
			{
				hi = pivot-1;
			}
			else
			{
				lo = pivot+1;
			}
		}
	}
	else
	{
		while ( lo <= hi )
		{
			int pivot = (lo+hi)>>1;
			if ( codeTable[pivot] == targetCode )
			{
				return pivot;
			}
			else if ( targetCode < codeTable[pivot] )
			{
				hi = pivot-1;
			}
			else
			{
				lo = pivot+1;
			}
		}
	}
	return INVALID_CODE;
}

// Converts an array of character codes to an array of glyph indices into
// a particular font.  Missing characters are returned as INVALID_CODE.
U16 * EditText::FindGlyphs(U16 *buffer, int length, FontDesc& fontDesc)
{
	if ( !length )
	{
		return NULL;
	}
	// Convert character codes to glyph indices
	U16* glyphs = new U16[length];
	U16* dst = glyphs;
	while ( length-- )
	{
		*dst++ = FindGlyph(*buffer++, fontDesc);
	}
	return glyphs;
}

BOOL EditText::Draw(STransform* x, BOOL buildEdges)
{
	ScriptPlayer* player = m_character->player;
	SPlayer* splayer = player->splayer;

	if ( splayer )
	{
		if ( !splayer->LoadBuiltInFonts() )
		{
			return false;
		}
	}

	int startLine, nextLine, visibleLines;
	BOOL retval = FALSE;
	U8 *advanceTable;
	SCharacter* font;
	SRGB rgb;
	int i, lineno, y;
	int devHeight, screenHeight;
	RColor* whiteColor, *blackColor, *color, *backRColor;
	U8 *s;
	U16 *glyphs = NULL;
	U16 *buffer = NULL;
	MATRIX mat, screenmat;
	BOOL mayDrawCursor;

	U16 lMargin=0, rMargin=0, indent=0, leading=0;
	int devLMargin, devRMargin, devIndent, devLeading, devCurrIndent;

	// Set up a stroker for any lines
	SStroker* stroker = NULL;

	BOOL isFocus = player->splayer &&
				   player->splayer->focus &&
				   player->splayer->focus->character == m_character;

	mayDrawCursor = isFocus &&
					player->splayer->cursorBlink &&
					!(m_flags & seditTextFlagsReadOnly);

	mat = x->mat;
	m_mat = x->mat;

	// Device font
	screenmat = mat;
	if ( player->display->antialias )
	{
		MATRIX aaToDev;
		MatrixScale(fixed_1/4, fixed_1/4, &aaToDev);
		MatrixConcat(&screenmat, &aaToDev, &screenmat);	// we don't want the supersampled coords
	}

	// Get font information
	FontDesc fontDesc;
	GetFontDesc(fontDesc);
	m_height = fontDesc.height;
	font = fontDesc.font;

	SParser parser;
	parser.Attach(m_character->data, 0);

	// Skip flags
	parser.SkipBytes(2);

	if ( m_flags & seditTextFlagsHasFont )
	{
		parser.SkipBytes(4);
	}

	if ( m_flags & seditTextFlagsHasTextColor )
	{
		rgb = parser.GetColor(true);
	}
	else
	{
		rgb.all = SRGBBlack;
	}

	if ( m_flags & seditTextFlagsHasMaxLength )
	{
		parser.SkipBytes(2);
	}

	if ( m_flags & seditTextFlagsHasLayout )
	{
		parser.GetByte();
		lMargin = parser.GetWord();
		rMargin = parser.GetWord();
		indent = parser.GetWord();
		leading = parser.GetWord();
	}

	// Calculate widths of characters
	delete [] devCharWidths;
	devCharWidths = new int[m_length+1];

	screenHeight = FixedMul(m_height << 16, screenmat.d) / fixed_1;
	if ( !buildEdges && !screenHeight )
	{
		goto exit_gracefully;
	}

	if ( buildEdges )
	{
		// Glyph outline font

		stroker = new SStroker(m_obj->display, &m_obj->edges);
		stroker->antialias = m_obj->display->antialias;

		devHeight = m_height;

		// Skip offset and shape tables
		parser.Attach(font->data, fontDesc.codeOffset);

		// Skip code tables
		if ( font->font.flags & sfontFlagsWideCodes )
		{
			parser.SkipBytes(font->font.nGlyphs * 2);
		}
		else
		{
			parser.SkipBytes(font->font.nGlyphs);
		}
		devAscent = parser.GetWord() * m_height / 1024;
		int descent = parser.GetWord() * m_height / 1024;
		// Skip leading
		parser.SkipBytes(2);
		devLineSpacing = devAscent + descent + (short)leading;

		advanceTable = (U8*) (parser.script+parser.pos);

		// Set up a new color
		color = CreateEditTextColor(
								   rgb.rgb.red,
								   rgb.rgb.green,
								   rgb.rgb.blue,
								   rgb.rgb.transparency,
								   1, //layer
								   x, m_obj
								   );

		backRColor = CreateEditTextColor(255, 255, 255, 255, 0, x, m_obj);
		blackColor = CreateEditTextColor(0, 0, 0, 255, 2, x, m_obj);
		whiteColor = CreateEditTextColor(255, 255, 255, 255, 3, x, m_obj);

		if ( m_flags & seditTextFlagsBorder )
		{
			AddRect(&m_character->bounds, &m_mat, blackColor,
					m_obj->display, &m_obj->edges, stroker);

			AddRect(&m_character->bounds, &m_mat, backRColor,
					m_obj->display, &m_obj->edges, NULL);
		}

		devBounds = m_character->bounds;
		RectInset(2*onePoint, &devBounds);

		// Copy left, right margin, indent, leading.. no need to transform
		devLMargin = lMargin;
		devRMargin = rMargin;
		devIndent  = indent;
		devLeading = leading;

	}
	else
	{
		// Device font
		mat = screenmat;

		if ( !MatrixIsScaleOnly(&mat) )
		{
			goto exit_gracefully;
		}

		MatrixTransformRect(&mat, &m_character->bounds, &devBounds);

		player->display->raster.Flush();

		devHeight = screenHeight;

		// Convert left, right margin and indent to device quantities
		devLMargin = FixedMul(lMargin << 16, mat.a) / fixed_1;
		devRMargin = FixedMul(rMargin << 16, mat.a) / fixed_1;
		devIndent  = FixedMul(indent  << 16, mat.a) / fixed_1;
		devLeading = FixedMul(leading << 16, mat.d) / fixed_1;

		rgb = x->cxform.Apply(rgb);

		devBounds = m_character->bounds;
		RectInset(2*onePoint, &devBounds);
		MatrixTransformRect(&mat, &devBounds, &devBounds);
	}

	// Get the buffer
	buffer = new U16[m_length + 1];

	if ( m_flags & seditTextFlagsPassword )
	{
		for ( i=0; i<m_length; i++ )
		{
			buffer[i] = PASSWORD_CHAR;
		}
		buffer[m_length] = 0;
	}
	else
	{
		if ( m_buffer )
		{
			WideStrCopy(buffer, m_buffer);
		}
		else
		{
			buffer[0] = 0;
		}
	}

	if ( buildEdges )
	{
		glyphs = FindGlyphs(buffer, m_length, fontDesc);

		for ( i=0; i<m_length; i++ )
		{
			U16 code = glyphs[i];
			U16 advance;
			if ( code == INVALID_CODE )
			{
				advance = 0;
			}
			else
			{
				advance = (U16)advanceTable[code*2] | ((U16)advanceTable[code*2+1]<<8);
				advance = advance * m_height / 1024;
			}
			devCharWidths[i] = advance;
		}
	}
	else
	{
		// Calculate device character widths
		{
			char *mbcsText = WideToMBCS(buffer, 0, m_length);
			char *mbcsPtr = mbcsText;
			int oldWidth = 0;
			for ( i=0; i<m_length; i++ )
			{
				int newWidth = 0;
				if ( IsLeadByte(*mbcsPtr) )
				{
					mbcsPtr++;
				}
				mbcsPtr++;
				devCharWidths[i] = newWidth - oldWidth;
				oldWidth = newWidth;
			}
			delete [] mbcsText;
		}
	}

	CalculateLineStarts(devCharWidths, RectWidth(&devBounds) - devLMargin - devRMargin, indent);
	visibleLines = CalcVisibleLines();

	if ( m_doFindCursor )
	{
		FindCursor(&devBounds, devCharWidths);
		m_doFindCursor = FALSE;
	}

	y = devBounds.ymin;

	// Skip any lines that are scrolled out of view
	lineno = m_vscroll;
	if ( lineno > m_numLines )
	{
		lineno = m_numLines - 1;
	}

	startLine = m_lineStarts[lineno];

	MATRIX charMat;
	MatrixIdentity(&charMat);
	charMat.ty = devBounds.ymin + devAscent;
	charMat.a = charMat.d = m_height * (fixed_1/1024);

	delete [] m_lineX;
	m_lineX = new int[m_numLines+1];
	for ( i=0; i<=m_numLines; i++ )
	{
		m_lineX[i] = 0;
	}

	devCurrIndent = devIndent;

	while ( --visibleLines >= 0 )
	{
		int cursorTop = y;
		int cursorBottom = y + devHeight;
		int selectTop = y;
		int selectBottom = y + devHeight;

		// Find the next line
		nextLine = m_lineStarts[min(m_numLines, lineno+1)];

		int x = devBounds.xmin + devLMargin + devCurrIndent;
		devCurrIndent = 0;

		int len = nextLine - startLine;

		// Don't render the newline
		if ( len )
		{
			U16 endChar = buffer[startLine+len-1];
			if ( endChar == editTextNewline || endChar == editTextSoftNewline )
			{
				len--;
			}
			if ( endChar == editTextNewline )
			{
				devCurrIndent = devIndent;
			}
		}

		if ( m_hscroll > len )
		{
			len = 0;
		}
		else
		{
			len -= m_hscroll;
			startLine += m_hscroll;
		}

		// Compute the X coordinate of each character
		int *charX = new int[len + 1];

		for ( i=0; i<len; i++ )
		{
			charX[i] = x;
			int charWidth = devCharWidths[startLine+i];
			if ( !(m_flags & seditTextFlagsWordWrap) )
			{
				// If word wrap is on then let CalculateLineStarts
				// decide on line breaks
				if ( x + charWidth > devBounds.xmax - devRMargin )
				{
					break;
				}
			}
			x += charWidth;
		}
		charX[i] = x;
		len = i;

		int xOffset = 0;
		switch ( m_align )
		{
		case stextAlignCenter:
			xOffset = (devBounds.xmax - devBounds.xmin - (charX[len] - charX[0])) >> 1;
			break;
		case stextAlignRight:
			xOffset = devBounds.xmax - devBounds.xmin - (charX[len] - charX[0]);
			break;
		}
		if ( xOffset )
		{
			for ( i=0; i<=len; i++ )
			{
				charX[i] += xOffset;
			}
		}

		m_lineX[lineno] = charX[0];

		lineno++;

		int A, B;
		if ( m_selectionStart < m_selectionEnd )
		{
			A = m_selectionStart;
			B = m_selectionEnd;
		}
		else
		{
			A = m_selectionEnd;
			B = m_selectionStart;
		}
		A -= startLine;
		B -= startLine;

		if ( B > len )
		{
			// Selection extends beyond this line
			selectBottom = y + devLineSpacing;
		}

		A = max(0, min(A, len));
		B = max(0, min(B, len));

		if ( buildEdges )
		{
			MATRIX m;
			for ( i=0; i<len; i++ )
			{
				U16 g = glyphs[startLine+i];
				if ( g != INVALID_CODE )
				{
					charMat.tx = charX[i];
					MatrixConcat(&charMat, &m_mat, &m);

					S32 glyphOffset;
					if ( font->font.flags & sfontFlagsWideOffsets )
					{
						s = font->data + 4*g;
						glyphOffset = (U32)s[0] | ((U32)s[1]<<8) | ((U32)s[2]<<16) | ((U32)s[3]<<24);
					}
					else
					{
						s = font->data + 2*g;
						glyphOffset = (U16)s[0] | ((U16)s[1]<<8);
					}
					{
						SCharacterParser parser(player, font->data, glyphOffset, &m);

						parser.useWinding = true;
						parser.obj = m_obj;
						parser.fillIndex[0] = 0;
						parser.lineIndex[0].color = 0;
						parser.lineIndex[0].thickness = 0;
						if ( i >= A && i < B )
						{
							parser.fillIndex[1] = whiteColor;
						}
						else
						{
							parser.fillIndex[1] = color;
						}
						parser.nFills = 1;
						SCharacterParser* test = &parser;
						parser.BuildEdges(false, &m_obj->devBounds);
					}
				}
			}

			if ( A != B )
			{
				SRECT rect;
				rect.xmin = charX[A];
				rect.ymin = selectTop;
				rect.xmax = charX[B];
				rect.ymax = selectBottom;

				AddRect(&rect, &m_mat, blackColor,
						m_obj->display, &m_obj->edges, NULL);
			}
		}
		else
		{
			if ( m_selectionStart == m_selectionEnd )
			{
				char *mbcsText = WideToMBCS(buffer+startLine, 0, len);
				delete [] mbcsText;
			}
			else
			{
				char *mbcsA = WideToMBCS(buffer+startLine, 0, A);
				char *mbcsB = WideToMBCS(buffer+startLine, A, B-A);
				char *mbcsC = WideToMBCS(buffer+startLine, B, len-B);

				delete [] mbcsA;
				delete [] mbcsB;
				delete [] mbcsC;
			}
		}

		// If edit text has the focus, then maybe draw the cursor
		if ( mayDrawCursor )
		{
			int cursorPos = m_selectionEnd-startLine;
			BOOL drawIt = (cursorPos >= 0 && cursorPos < len);
			if ( cursorPos == len && (m_atLineEnd || m_selectionEnd >= m_length-1 || len == 0 || buffer[startLine+len] == editTextNewline) )
			{
				drawIt = TRUE;
			}
			if ( drawIt )
			{
				int cursorX = charX[cursorPos];
				SPOINT screenPt;
				if ( buildEdges )
				{
					StrokeCursor(m_obj, &m_mat, cursorX, cursorTop, cursorBottom, color, stroker);
					screenPt.x = cursorX;
					screenPt.y = cursorTop;
					MatrixTransformPoint(&screenmat, &screenPt, &screenPt);
				}
				else
				{
					screenPt.x = cursorX;
					screenPt.y = cursorTop;
				}
				mayDrawCursor = FALSE;
			}
		}

		delete [] charX;

		if ( !buffer[startLine] )
		{
			break;
		}

		startLine = nextLine;
		y += devLineSpacing;
		charMat.ty += devLineSpacing;
	}

	if ( !(m_flags & seditTextFlagsNoSelect) )
	{
		// Auto-scroll
		AutoScroll();
	}

	// Signal success!
	retval = TRUE;

	exit_gracefully:
	delete [] glyphs;
	delete [] buffer;
	delete stroker;

	m_drawn = retval;
	return retval;
}

void EditText::GetEnabledCommands(int& flags)
{
	if ( m_flags & seditTextFlagsNoSelect )
	{
		// No commands permitted at all
		flags = 0;
	}
	else
	{
		// Select All is always permitted if selection isn't disabled
		flags = kSelectAllEnabled;

		BOOL selection = (m_selectionStart != m_selectionEnd);

		// Copy is permitted if a selection exists and not in password mode
		if ( selection && ((m_flags & seditTextFlagsPassword) == 0) )
		{
			flags |= kCopyEnabled;
		}

		if ( (m_flags & seditTextFlagsReadOnly) == 0 )
		{
			// Cut is enabled if Copy is enabled and not read only
			if ( flags & kCopyEnabled )
			{
				flags |= kCutEnabled;
			}
			// Clear is enabled if a selection exists and not read only
			if ( selection )
			{
				flags |= kClearEnabled;
			}
			// Paste is enabled if not read only and clipboard data exists
			// For simplicity, Paste is always enabled on UNIX
			flags |= kPasteEnabled;

		}
	}
}

void EditText::UpdateFromVariable()
{
	SPlayer* splayer = m_character->player->splayer;

	if ( splayer )
	{
		char *varName;
		ScriptThread* thread = splayer->ResolveVariable(m_variable,
														m_obj->thread->rootObject->thread,
														&varName);

		if ( thread && varName[0] )
		{
			ScriptVariable* var = thread->FindVariable(varName);
			if ( var )
			{
				SetBuffer(var->value);
			}
			else if ( m_initialText )
			{
				// The variable is not yet set, set it from the initial value
				// in the edit text field.
				SetBuffer(m_initialText);
				thread->SetVariable(varName, m_initialText, TRUE);
			}
		}
		else if ( m_initialText )
		{
			SetBuffer(m_initialText);
		}

		SaveFocus* saveFocus = &splayer->saveFocus;
		if ( saveFocus->variable && m_obj->depth == saveFocus->depth && StrEqual(m_variable, saveFocus->variable) )
		{
			m_selectionStart = saveFocus->selectionStart;
			m_selectionEnd   = saveFocus->selectionEnd;
			m_selecting      = saveFocus->selecting;
			splayer->focus   = m_obj;
		}
	}

}

//
// SaveFocus object
//
SaveFocus::SaveFocus()
{
	Clear();
}

void SaveFocus::Clear()
{
	variable        = 0;
	depth           = 0;
	selectionStart  = 0;
	selectionEnd    = 0;
	selecting       = FALSE;
}

void SaveFocus::Save(SPlayer* splayer)
{
	if ( splayer && splayer->focus )
	{
		variable        = splayer->focus->editText->m_variable;
		depth           = splayer->focus->depth;
		selectionStart  = splayer->focus->editText->m_selectionStart;
		selectionEnd    = splayer->focus->editText->m_selectionEnd;
		selecting       = splayer->focus->editText->m_selecting;
	}
	else
	{
		Clear();
	}
}

#endif // EDITTEXT
