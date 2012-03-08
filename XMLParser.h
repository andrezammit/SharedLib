/////////////////////////////////////////////////////////////////////////////
// Name:        XML_PARSER.h
// Version:     2.0
// Purpose:     Simple MSXML library Wrapper class for MFC/C++ (header file)
// Author:      André Sébastien  (maximus@oreka.com)
// Copyright:   (c) ANDRE Sébastien
/////////////////////////////////////////////////////////////////////////////

#if !defined(XML_PARSER_H)
#define XML_PARSER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// -- CArray STL --
//
#include <afxtempl.h>

// -- Load MSXML library --
//
#import <msxml3.dll> named_guids
using namespace MSXML2;

class XML_PARSER
{
public:
   XML_PARSER();                                                  // C'tor
   virtual ~XML_PARSER();                                         // D'tor

   // -- ********************************* --
   // -- *** LOADING / SAVING DOCUMENT *** --
   // -- ********************************* --
   bool Load_XML_Document(LPCTSTR strFileName);                   // Load an XML Document from File
   bool Save_XML_Document(LPCTSTR strFileName);                   // Save the Current XML Document into a file
   void Load_XML_From_Buffer(LPCTSTR source);                     // Load an XML Document from Buffer
   void Reset_XML_Document();                                     // Clear the XML Document

   // -- ************************** --
   // -- *** XML REPRESENTATION *** --
   // -- ************************** --
   void Get_XML(CString & buffer);                                // Get the XML Representation of the current node
   void Get_XML_Document(CString & buffer);                       // Get the XML Representation of the entire document

   // -- ***************************** --
   // -- *** XML Header management *** -- 
   // -- ***************************** --
   bool Set_Header(LPCTSTR header,LPCTSTR name,LPCTSTR value);    // Set an header property like <? xml version="1.0"?>
   bool Get_Header(LPCTSTR header,LPCTSTR name,CString & res);    // Return the header property like <? xml version="1.0"?>
                                                                  // Rq: For request the version number like the example below
                                                                  //     you must specify header = "xml" and name = "version"

   // -- ********************** --
   // -- *** PARSING METHOD *** --  
   // -- ********************** --
   virtual void Parse_XML_Document();                             // ===> YOU MUST IMPLEMENT IT FOR PARSING PROCESS <===

   // -- *********************************
   // -- *** CURRENT NODE TYPE CONTROL ***
   // -- *********************************
   bool Is_Tag(LPCTSTR aTag);                                     // Return true if it's equal to the Current Tag
   bool Is_Child_of(LPCTSTR parent_chain);                        // Test if a chain Tag is the parent of current Tag
   bool Is_Root();                                                // Test if a Tag is the Root
   bool Is_TextNode();                                            // Return true if it's a TEXT node    
   bool Is_CDataSection();                                        // Return true if it's a CDATA section 

   // -- *******************************************
   // -- *** CURRENT NODE TYPE ACCESS PROPERTIES ***
   // -- *******************************************
   CString & Get_CurrentTag();                                    // Get the Current Tag value  (with "<>")
   CString & Get_CurrentName();                                   // Get the Current Name value (without "<>")
   CString & Get_TextValue();                                     // Get the Text Value when it's a TEXT or CDATA node
                                                                  // If you make a call of "Get_TextValue" of a node
                                                                  // this method will return you it's Text value if it has one
                                                                  // For return the CData section, you must be on the CDATA
                                                                  // section itself
   bool Set_TextValue(LPCTSTR TextValue);                         // Set a Text to the Current Node
                                                                  // on a CData section, this method set the data

   // -- ************************* --
   // -- *** ATTRIBUTES ACCESS *** --
   // -- ************************* --
   bool Is_Having_Attribute(LPCTSTR Name);                        // Return true if current node have the specified attribute defined
   CString & Get_Attribute_Value();                               // Return the attribute value selected by "Is_Having_Attribute()"

   int Get_Attribute_Count();                                     // Return the number of attributes for the current node
   CString & Get_Attribute_Name(int index);                       // Return the attribute name  for the nth attribute
   CString & Get_Attribute_Value(int index);                      // Return the attribute value for the nth attribute

   bool Set_Attribute(LPCTSTR AttribName,LPCTSTR AttribValue);    // Set an attribute to the Current Node
   bool Remove_Attribute(LPCTSTR AttribName);                     // Remove an attribute the the Current Node
     
   // -- ****************************** --
   // -- *** CDATA SECTION CREATION *** --
   // -- ****************************** --
   bool Add_LastChildCData(LPCTSTR data);                         // Add a new CDATA Section at the End of Current Node
   bool Add_FirstChildCData(LPCTSTR data);                        // Add a new CDATA Section at the Begining of Current Node
   bool Add_CDataBefore(LPCTSTR data);                            // Add a new CDATA Section (same level) before the Current Node
   bool Add_CDataAfter(LPCTSTR data);                             // Add a new CDATA Section (same levelà after the Current Node
                                                                  // Rq: When Current Node is a CDATA Section, 
                                                                  //     use Get_TextValue() for retrieve DATA
                                                                  //     and Set_TextValue() for sets DATA
     
   // -- ********************* --
   // -- *** NODE CREATION *** --
   // -- ******************** --
   bool Add_LastChildNode(LPCTSTR Name);                          // Add a new CHILD NODE at the End of Current Node
   bool Add_FirstChildNode(LPCTSTR Name);                         // Add a new CHILD NODE at the Begining of Current Node
   bool Add_NodeBefore(LPCTSTR Name);                             // Add a new NODE (same level) before the Current Node
   bool Add_NodeAfter(LPCTSTR Name);                              // Add a new NOFR (same level) after the Current Node

   // -- *********************
   // -- *** NODE DELETION ***
   // -- *********************
   bool Remove();                                                 // Remove the Current Node (Move to the Parent Node)
   bool RemoveChild(LPCTSTR NodeName);                            // Remove a Child Node     (No Move);

   // -- ******************* --
   // -- *** NODE MOVING *** --
   // -- ******************* --
   void Go_to_Root();                                             // Go to the Root node
   bool Go_to_Child();                                            // Go to the first child of the current node
   bool Go_to_Parent();                                           // Go to the parent node of the current node
   bool Go_Forward();                                             // Go to the next     node of same level as the current node
   bool Go_Backward();                                            // Go to the previous node of same level as the current node
   
   // -- *************************** --
   // -- *** NODE MOVING by name *** --
   // -- *************************** --
   bool Go_to_Child(LPCTSTR NodeName);                            // Go to a Direct Child Node
   bool Go_to_Descendant(LPCTSTR NodeName);                       // Go to a Descendant Node (Child, Little Child, Etc...)
   bool Go_to_Parent(LPCTSTR NodeName);                           // Go to a Parent Node
   bool Go_Forward(LPCTSTR NodeName);                             // Go to a next Node at the same Node than the Current
   bool Go_Backward(LPCTSTR NodeName);                            // Go to a previous Node attached at the same Node than the Current

   // -- ************************ --
   // -- *** Error Management *** --
   // -- ************************ --
   CString & Get_LastError();                                     // Get the last error signification

private:
   // Node Value
   //
   CString Tag;                                                   // The last Tag (the name with "<...>")
   CString Name;                                                  // The Name (without "<...>")
   CString ChainTag;                                              // The Chain Tag
   CString TextField;                                             // The Text Value

   // Init MSXML library
   //
   bool Init_MSXML();

   // XML document pointer
   //
   MSXML2::IXMLDOMDocumentPtr m_plDomDocument;
	
   // Root node of XML document
   //
   MSXML2::IXMLDOMElementPtr m_pDocRoot;

   // The current Node added
   //
   MSXML2::IXMLDOMNodePtr CurrentNode;

   // Error Status String
   //
   CString lasterror,ok,tmp;

   // Attrib Values
   //
   CArray<CString,CString &> AttribNames;
   CArray<CString,CString &> AttribValues;
   int attrib_index;

   // Internal Methods, Parse the Tree nodes
   //
   void Parse_Objects(MSXML2::IXMLDOMNodePtr pObject);

   // Update "Node Values" & Test type
   //
   void Grab_Node_Informations(MSXML2::IXMLDOMNodePtr pChild);            // Grab all eficients informations about a MSXML Node
   HRESULT Is_MSXML_Node(MSXML2::IXMLDOMNodePtr pChild);                  // Return NULL,NODE_ELEMENT,NODE_TEXT or NODE_CDATA_SECTION

   // ChainTag methods management
   //
   void ChainTag_Add(CString & val);
   void ChainTag_Remove(int number);

   // Internal adding node
   //
   bool _add_lastchild(MSXML2::IXMLDOMNodePtr newNode);       
   bool _add_firstchild(MSXML2::IXMLDOMNodePtr newNode);                  
   bool _add_before(MSXML2::IXMLDOMNodePtr newNode);                      
   bool _add_after(MSXML2::IXMLDOMNodePtr newNode);                      
};
#endif
