/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

// The object is responsible for generating edges and colors in a format 
//	that the scan converter can use.

#ifndef EDITTEXT_INCLUDED
#define EDITTEXT_INCLUDED

#define editTextNewline '\r'
#define editTextSoftNewline '\n'

#ifdef EDITTEXT
struct SObject;
struct SCharacter;
struct STransform;
class SPlayer;

enum {
	editTextGetScroll,
	editTextSetScroll,
	editTextGetMaxScroll
};

int WideStrLen(const U16 *buf);
void WideStrCopy(U16 *dst, const U16 *src);

class EditText
{
public:
	// next is pointer to next EditText in update list
	EditText *next;

	enum {
		PASSWORD_CHAR   = '*'
	};

	EditText(SObject* obj);
	~EditText();

	/*! HandleKeyDown processes keys for the edit text fields. The NativePlayerWnd 
	    gets passed in so the edit text can request pastes and copies to the 
		clipboard.
	 */
	void     HandleKeyDown( int key, int modifiers, NativePlayerWnd* native );
	char *   GetBuffer();
	void     SetBuffer(char *text);
	void     DoMouse(SPOINT *pt, BOOL mouseIsDown);
	void     ClearSelection();
	BOOL     IsSelected() { return m_selectionStart != m_selectionEnd; }

	enum {
		kCutEnabled			= 1,
		kCopyEnabled		= 2,
		kPasteEnabled		= 4,
		kClearEnabled		= 8,
		kSelectAllEnabled	= 16
	};
 	void	GetEnabledCommands(int& flags);

	BOOL	Draw(STransform* x, BOOL buildEdges);

	BOOL IsFontIncluded() { return (m_flags & seditTextFlagsUseOutlines) != 0; }

	U16	   *m_buffer;
	char	   *m_variable;
	char	   *m_initialText;
	int         m_length;
	int         m_selectionStart, m_selectionEnd;
	int         m_hscroll, m_vscroll;
	MATRIX		m_mat;
	int		   *m_lineX;
	BOOL        m_mouseIsDown;
	S32			m_clickTime;
	SPOINT      m_mousePosition;
	BOOL        m_selecting;
	U16	    m_flags;
	int		    m_maxLength;
	int         m_height;
	int        *m_lineStarts;
	int         m_numLines;
	BOOL	    m_atLineEnd;
	SObject*	m_obj;
	SCharacter *m_character;
	BOOL		m_drawn;
	BOOL		m_doFindCursor;
	int			m_heldLeadByte;
	int 		m_align;
	static BOOL m_insertMode;

	SRECT devBounds;
	int devAscent;
	int devLineSpacing;
	int *devCharWidths;

	void Expand(int size);
	void Insert(U16 ch) { InsertWideChars(&ch, 1); }
	void Insert(char *s, int len);
	void InsertWideChars(U16 *s, int len);
	void Backward(int wholeWord, int extend);
	void Forward(int wholeWord, int extend);
	void Backspace();
	void Delete();
	void DeleteSelection();
	void FindCursor(SRECT* devBounds, int *devCharWidths);
	void CopyToClipboard( NativePlayerWnd* );
	void PasteFromClipboard( NativePlayerWnd* );
	int  FindLineStart(int pos) { return m_lineStarts[FindLineNumber(pos)]; }
	int  FindLineEnd(int pos);
	int  FindLineNumber(int pos);
	void AutoScroll();
	void CalculateLineStarts(int *devCharWidths, int width, int indent);

	struct FontDesc {
		char fontName[256];
		int height;
		BOOL boldFlag;
		BOOL italicFlag;
		BOOL japaneseFlag;
		SCharacter* font;
		S32 codeOffset;
	};

	void GetFontDesc(FontDesc& fontDesc);

	BOOL IsLeadByte(U8 ch);
	BOOL IsMultiByte();
	U16 *MBCSToWide(char *buffer, int index, int count);
	char *WideToMBCS(U16 *buffer, int index, int count);
	U16 FindGlyph(U16 targetCode, FontDesc& fontDesc);
	U16 *FindGlyphs(U16 *buffer, int length, FontDesc& fontDesc);

	int CalcVisibleLines();
	int CalcMaxVScroll();

	void UpdateFromVariable();

	BOOL BreakEnglish(U16 ch1, U16 ch2);
	BOOL LineBreakOK(U16 ch1, U16 ch2);
	BOOL LineBreakOK_Index(int index);
};

// The SaveFocus object is used to save and restore the current focus.

struct SaveFocus
{
	char	*variable;
	int		depth;
	int		selectionStart;
	int		selectionEnd;
	BOOL	selecting;

	SaveFocus();
	void Save(SPlayer* splayer);
	void Clear();
};

#endif // EDITTEXT

#endif // EDITTEXT_INCLUDED


