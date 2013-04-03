// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_FILE_SYSTEM_H_INCLUDED__
#define __I_FILE_SYSTEM_H_INCLUDED__

#include "IUnknown.h"

namespace irr
{
namespace io
{

class IReadFile;
class IWriteFile;
class IFileList;
class IXMLReader;
class IXMLWriter;

//! The FileSystem manages files and archives and provides access to them.
/*!
	It manages where files are, so that modules which
	use the the IO do not need to know where every file is located. A file
	could be in a .zip-Archive or as file on disk, using the IFileSystem
	makes no difference to this.
*/
class IFileSystem : public IUnknown
{
public:

	//! destructor
	virtual ~IFileSystem() {};

	//! Opens a file for read access.
	//! \param filename: Name of file to open.
	//! \return Returns a pointer to the created file interface.
	//! The returned pointer should be dropped when no longer needed.
	//! See IUnknown::drop() for more information.
	virtual IReadFile* createAndOpenFile(const c8* filename) = 0;

	//! Opens a file for write access.
	//! \param filename: Name of file to open.
	//! \param append: If the file already exist, all write operations are 
	//! appended to the file.
	//! \return Returns a pointer to the created file interface. 0 is returned, if the
	//! file could not created or opened for writing.
	//! The returned pointer should be dropped when no longer needed.
	//! See IUnknown::drop() for more information.
	virtual IWriteFile* createAndWriteFile(const c8* filename, bool append=false) = 0;

	//! Adds an zip archive to the file system.
	//! \param filename: Filename of the zip archive to add to the file system.
	//! \param ignoreCase: If set to true, files in the archive can be accessed without
	//! writing all letters in the right case.
	//! \param ignorePaths: If set to true, files in the added archive can be accessed
	//! without its complete path.
	//! \return Returns true if the archive was added successful, false if not.
	virtual bool addZipFileArchive(const c8* filename, bool ignoreCase = true, bool ignorePaths = true) = 0;
	
	//! Returns the string of the current working directory.
	virtual const c8* getWorkingDirectory() = 0;

	//! Changes the current Working Directory to the overgiven string.
	//! \param
	//! newDirectory is a string specifiing the new working directory.
	//! The string is operating system dependent. Under Windows it has
	//! the form "<drive>:\<directory>\<sudirectory>\<..>". An example would be: "C:\Windows\"
	//! \return Returns true if successful, otherwise false.
	virtual bool changeWorkingDirectoryTo(const c8* newDirectory) = 0;

	//! Creates a list of files and directories in the current working directory 
	//! and returns it.
	//! \return a Pointer to the created IFileList is returned. After the list has been used
	//! it has to be deleted using its IFileList::drop() method.
	//! See IUnknown::drop() for more information.
	virtual IFileList* createFileList() = 0;

	//! determinates if a file exists and would be able to be opened.
	//! \param filename is the string identifying the file which should be tested for existance.
	//! \return Returns true if file exsits, and false if it does not exist or an error occured.
	virtual bool existFile(const c8* filename) = 0;

	//! Creates a XML Reader from a file.
	//! \return 0, if file could not be opened, otherwise a pointer to the created
	//! IXMLReader is returned. After use, the reader
	//! has to be deleted using its IXMLReader::drop() method.
	//! See IUnknown::drop() for more information.
	virtual IXMLReader* createXMLReader(const c8* filename) = 0;

	//! Creates a XML Reader from a file.
	//! \return 0, if file could not be opened, otherwise a pointer to the created
	//! IXMLReader is returned. After use, the reader
	//! has to be deleted using its IXMLReader::drop() method.
	//! See IUnknown::drop() for more information.
	virtual IXMLReader* createXMLReader(IReadFile* file) = 0;

	//! Creates a XML Writer from a file.
	//! \return 0, if file could not be opened, otherwise a pointer to the created
	//! IXMLWriter is returned. After use, the reader
	//! has to be deleted using its IXMLWriter::drop() method.
	//! See IUnknown::drop() for more information.
	virtual IXMLWriter* createXMLWriter(const c8* filename) = 0;

	//! Creates a XML Writer from a file.
	//! \return 0, if file could not be opened, otherwise a pointer to the created
	//! IXMLWriter is returned. After use, the reader
	//! has to be deleted using its IXMLWriter::drop() method.
	//! See IUnknown::drop() for more information.
	virtual IXMLWriter* createXMLWriter(IWriteFile* file) = 0;
};

} // end namespace io
} // end namespace irr


#endif

