// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __I_XML_READER_H_INCLUDED__
#define __I_XML_READER_H_INCLUDED__

#include "IUnknown.h"

namespace irr
{
namespace io
{
	//! Enumeration for all xml nodes which are parsed by IXMLReader
	enum EXML_NODE
	{
		//! No xml node. This is usually the node if you did not read anything yet.
		EXN_NONE,

		//! A xml element, like <foo>
		EXN_ELEMENT,

		//! End of an xml element, like </foo>
		EXN_ELEMENT_END,

		//! Text within a xml element: <foo> this is the text. </foo>
		EXN_TEXT,

		//! An xml comment like <!-- I am a comment --> or a DTD definition.
		EXN_COMMENT,

		//! Unknown element.
		EXN_UNKNOWN
	};

	//! Interface providing easy read access to an XML file.
	/** The Irrlicht Engine XML Reader provides forward-only, read-only 
     access to a stream of non validated XML data. It was fully implemented by
	 Nikolaus Gebhardt, and does not rely on external XML software. 
	 It is quite fast, and has the advantage, that it can read everywhere the Irrlicht
	 Engine can, for example directly from compressed .zip files. This XML parser
	 is very basic and simple. It was created with the intention to parse configuration 
	 data, please use a more advanced XML parser if you have a more complicated task.
	 It currently has the following limitations:
		- The reader only recognizes some special xml characters:
          & (&amp;), < (&lt;), > (&gt;), and " (&quot;)
		- The XML Parser is able to parse ASCII and UTF-16 text files, 
		  in little and big endian. Other text formats like UTF-8 are not supported yet,
		  but maybe will be added later.
		- The xml file is not validated and assumed to be correct. If the is malformatted,
		  the parser may crash. But in most cases, the parser should just ignore the problem.

    The following code demonstrates the basic usage of the xml reader. A simple xml
	file like this is parsed:
    \code
	<?xml version="1.0"?>
	<config>
		<!-- This is a config file for the mesh viewer -->
		<startUpModel file="../data/dwarf.x" />
		<messageText caption="Irrlicht Engine Mesh Viewer">
		Welcome to the Mesh Viewer of the &quot;Irrlicht Engine&quot;.
		</messageText>
	</config>
	\endcode

	The code for parsing this file would look like this:
	\code
	io::IXMLReader* xml = device->getFileSystem()->createXMLReader("config.xml");

	core::stringw startUpModelFile;
	core::stringw messageText;
	core::stringw caption;

	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
		case io::EXN_TEXT:
			// in this xml file, the only text which occurs is the messageText
			messageText = xml->getNodeData();
			break;
		case io::EXN_ELEMENT:
			{
				if (core::stringw("startUpModel") == xml->getNodeName())
					startUpModelFile = xml->getAttributeValue(L"file");
				else
				if (core::stringw("messageText") == xml->getNodeName())
					caption = xml->getAttributeValue(L"caption");
			}
			break;
		}
	}

	if (xml)
		xml->drop(); // don't forget to delete the xml reader 
	\endcode
	*/
	class IXMLReader : public IUnknown
	{
	public:

		//! Destructor
		virtual ~IXMLReader() {};

		//! Reads forward to the next xml node. 
		//! \return Returns false, if there was no further node. 
		virtual bool read() = 0;

		//! Returns the type of the current XML node.
		virtual EXML_NODE getNodeType() = 0;

        //! Returns attribute count of the current XML node. This is usually
		//! non null if the current node is EXN_ELEMENT, and the element has attributes.
		//! \return Returns amount of attributes of this xml node.
		virtual s32 getAttributeCount() = 0;

		//! Returns name of an attribute. 
		//! \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		//! \return Name of the attribute, 0 if an attribute with this index does not exist.
		virtual const wchar_t* getAttributeName(s32 idx) = 0;

		//! Returns the value of an attribute. 
		//! \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		//! \return Value of the attribute, 0 if an attribute with this index does not exist.
		virtual const wchar_t* getAttributeValue(s32 idx) = 0;

		//! Returns the value of an attribute. 
		//! \param name: Name of the attribute.
		//! \return Value of the attribute, 0 if an attribute with this name does not exist.
		virtual const wchar_t* getAttributeValue(const wchar_t* name) = 0;

		//! Returns the value of an attribute, like getAttributeValue(), but does not 
		//! return 0 if the attribute does not exist. An empty string ("") is returned then.
		//! \param name: Name of the attribute.
		//! \return Value of the attribute, and "" if an attribute with this name does not exist
		virtual const wchar_t* getAttributeValueSafe(const wchar_t* name) = 0;

		//! Returns the value of an attribute as integer. 
		//! \param name: Name of the attribute.
		//! \return Value of the attribute as integer, and 0 if an attribute with this name does not exist or
		//! the value could not be interpreted as integer.
		virtual s32 getAttributeValueAsInt(const wchar_t* name) = 0;

		//! Returns the value of an attribute as float. 
		//! \param name: Name of the attribute.
		//! \return Value of the attribute as float, and 0 if an attribute with this name does not exist or
		//! the value could not be interpreted as float.
		virtual f32 getAttributeValueAsFloat(const wchar_t* name) = 0;

		//! Returns the name of the current node. Only non null, if the node type is
		//! EXN_ELEMENT.
		//! \return Name of the current node or 0 if the node has no name.
		virtual const wchar_t* getNodeName() = 0;

		//! Returns data of the current node. Only non null if the node has some
		//! data and it is of type EXN_TEXT or EXN_UNKNOWN.
		virtual const wchar_t* getNodeData() = 0;

		//! Returns if an element is an empty element, like <foo />
		virtual bool isEmptyElement() = 0;
	};

} // end namespace irr
} // end namespace io

#endif

