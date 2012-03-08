/////////////////////////////////////////////////////////////////////////////
// Name:        XML_PARSER.cpp
// Version:     2.0
// Purpose:     Simple MSXML library Wrapper class for MFC/C++ (source file)
// Author:      André Sébastien  (maximus@oreka.com)
// Copyright:   (c) ANDRE Sébastien
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Version 2.0 What's news ?
// 
//      1. The parsing document start from document and not from root element
//           - Let us to parse the header document informations
//      2. New XML header management
//           - encoding format, stylesheet reference, etc...
//                - Set_Header("xml","version","1.0")
//                - Set_Header("xml","encoding","UTF-8"), etc...
//      3. New CData section management 
//           - We can now read and add CData section with method like "Add_LastChildCData(LPCTSTR data)"
//      4. Some new methods for more fun
//           - like "Get_XML_Document()"
//      5. Minor design change for improve the class
//           - Some methods have been renamed like "Get_Text()" to "Get_TextValue()"
//      
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include <atlbase.h>
#include "XMLParser.h"

XML_PARSER::XML_PARSER()
{       // Constructor

        // Init our members
        //
        this->AttribNames.RemoveAll();
        this->AttribValues.RemoveAll();
        this->Tag.Empty();
        this->Name.Empty();
        this->ChainTag.Empty();
        this->TextField.Empty();

        // Init MSXML members
        //
        CurrentNode = NULL;
        m_plDomDocument = NULL;
        m_pDocRoot = NULL;

        attrib_index = -1;

        // -- Errors Init --
        //
        ok.Empty();
        ok = "Success";
        lasterror = ok;
}

XML_PARSER::~XML_PARSER()
{       // Free ressource
        //
        this->Reset_XML_Document();
}

CString & XML_PARSER::Get_LastError()
{ return lasterror; }

bool XML_PARSER::Init_MSXML()
{       lasterror = ok;

        // -- Init OLE Object Comunication for MSXML (only once time)--
        //
        static bool ole_initialized = false;
        if(!ole_initialized)
        {       ::AfxOleInit();
                ole_initialized = true;
        }

        // -- Instantiate an XML document --
        //
		HRESULT hr = m_plDomDocument.CreateInstance(MSXML2::CLSID_DOMDocument);
        if (FAILED(hr))
        {       _com_error er(hr);
                lasterror = er.ErrorMessage();
                return false;
        }

        return true;
}

bool XML_PARSER::Load_XML_Document(LPCTSTR strFileName)
{       lasterror = ok;

        // Reset Document
        //
        this->Reset_XML_Document();

        // Convert xml file name string to something COM can handle (BSTR)
        //
        CString sFileName = strFileName;
        _bstr_t bstrFileName;
        bstrFileName = sFileName.AllocSysString();

        // Call the IXMLDOMDocumentPtr's load function to load the XML document
        //
        variant_t vResult;
        vResult = m_plDomDocument->load(bstrFileName);
        if (((bool)vResult) == TRUE) // success!
        {
                // Now that the document is loaded, we need to initialize the root pointer
                //
                m_pDocRoot = m_plDomDocument->documentElement;

                // Now we can Parse this document !!
                //
                Parse_Objects(m_plDomDocument);

                this->Go_to_Root();

                return true;
        }
        else
        {       // XML document is not loaded, return error
                //
                lasterror = "XML Document FAILED to load";
                return false;
        }
}

void XML_PARSER::Load_XML_From_Buffer(LPCTSTR source)
{
        // Reset the document
        //
        this->Reset_XML_Document();

        // Load from Buffer
        //
        m_plDomDocument->loadXML(source);
        m_pDocRoot = m_plDomDocument->documentElement;

        // Start the Parsing
        //
        Parse_Objects(m_plDomDocument);

        // Move to the root node
        //
        this->Go_to_Root();
}

void XML_PARSER::Get_XML(CString & buffer)
{       
        if(this->CurrentNode == this->m_pDocRoot)
            this->Get_XML_Document(buffer);
        else
        {   BSTR  b_xml;
            this->CurrentNode->get_xml(&b_xml);

            buffer = b_xml;
        }
}

void XML_PARSER::Get_XML_Document(CString & buffer)
{   BSTR b_xml;
    this->m_plDomDocument->get_xml(&b_xml);

    buffer = b_xml;
}

void XML_PARSER::Parse_XML_Document()
{ /* Nothing to do , this method will be redefined in derived class */ }

void XML_PARSER::Parse_Objects(MSXML2::IXMLDOMNodePtr pObject)
{
        // Child node
        //
        MSXML2::IXMLDOMNodePtr pChild;

        // Grab Informations from the pObject node
        //
        this->Grab_Node_Informations(pObject);

        // Update "CurrentNode"
        //
        this->CurrentNode = pObject;

        // Call User Parse Handling for let him what he want.
        //
        this->Parse_XML_Document();

        if(Is_MSXML_Node(pObject) != MSXML2::NODE_ELEMENT)
           return;

        // add the ChainTag
        //
        this->ChainTag_Add(this->Tag);

        for (pChild = pObject->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
        {
                // Parse Child nodes
                //
                this->Parse_Objects(pChild);
        }

        // Remove Current Tag from ChainTag
        //
        ChainTag_Remove(1);
}

bool XML_PARSER::Is_Tag(LPCTSTR aTag)
{ return this->Get_CurrentTag() == aTag; }

bool XML_PARSER::Is_TextNode()
{ return (this->Is_Tag(_T("<#TEXT#>"))); }

bool XML_PARSER::Is_CDataSection()
{ return (this->Is_Tag(_T("<#CDATA#>"))); }

CString & XML_PARSER::Get_CurrentTag()
{       return this->Tag; }

CString & XML_PARSER::Get_CurrentName()
{  return this->Name; }

bool XML_PARSER::Is_Root()
{   return (this->ChainTag.IsEmpty() != 0); }

bool XML_PARSER::Is_Child_of(LPCTSTR parent_chain)
{
        int pos = this->ChainTag.Find(parent_chain);
        int len = _tcslen(parent_chain);

        if(pos != -1)
        {       // look if it's the end of this chainTag
                //
                return ( this->ChainTag.GetLength() == pos + len );
        }

        return false;
}

CString & XML_PARSER::Get_TextValue()
{  // Now you can handle the text value on the real node directly if you want

   if(Is_MSXML_Node(this->CurrentNode) == MSXML2::NODE_TEXT || Is_MSXML_Node(this->CurrentNode) == MSXML2::NODE_CDATA_SECTION)
      return this->TextField;

   // We must find his text value
   //
   CString TextValue;

   // Find if a CHILD TEXT NODE exist or not
   //
   if(CurrentNode == NULL)
   {       if(this->m_pDocRoot != NULL)
                   this->CurrentNode = this->m_pDocRoot;
           else
           {       // We can't set the text
                   //
                   lasterror = "XML_PARSER::Get_TextValue() Can't set text on NULL node";
                   this->TextField = "";
                   return this->TextField;
           }
   }

   // Find it now
   //
   if(this->CurrentNode->hasChildNodes())
   {
           MSXML2::IXMLDOMNodePtr pChild;
           for(pChild = this->CurrentNode->firstChild; pChild != NULL; pChild = pChild->nextSibling)
           {       // Find it's a NODE TEXT
                   //
                   if(this->Is_MSXML_Node(pChild) == MSXML2::NODE_TEXT)
                   {       // Take informations from this Text Node
                           //
                           this->Grab_Node_Informations(pChild);
                           TextValue = this->TextField;
                           this->Grab_Node_Informations(this->CurrentNode); // it remove this->TextField
                           this->TextField = TextValue;
                           return (this->TextField);
                   }
           }
   }

   this->TextField = "";
   return (this->TextField);
}

HRESULT XML_PARSER::Is_MSXML_Node(MSXML2::IXMLDOMNodePtr pChild)
{       if(pChild == NULL) return NULL;

        // I handle only few node type other are ignored and are considered as NODE_ELEMENT
        //
        // Handled Node type by this wrapper:
        //      - NODE_ELEMENT
        //      - NODE_TEXT
        //      - NODE_CDATA_SECTION
        //      - NODE_PROCESSING_INSTRUCTION

        if(pChild->nodeType == MSXML2::NODE_TEXT) return MSXML2::NODE_TEXT;
        if(pChild->nodeType == MSXML2::NODE_CDATA_SECTION) return MSXML2::NODE_CDATA_SECTION;
        if(pChild->nodeType == MSXML2::NODE_PROCESSING_INSTRUCTION ) return MSXML2::NODE_PROCESSING_INSTRUCTION;
        return MSXML2::NODE_ELEMENT;
}

void XML_PARSER::Grab_Node_Informations(MSXML2::IXMLDOMNodePtr pChild)
{
        if(this->Is_MSXML_Node(pChild) == MSXML2::NODE_ELEMENT)
        {       // It's a node
                //

                // Tag Update
                //
				CString rNodeName((const char*)pChild->nodeName);

                Tag.Format(_T("<%s>"), rNodeName);
                Name = (const char*)(pChild->nodeName);

                // TextField no significant value
                TextField.Empty();

                // Update Attribute List
                //
                this->AttribNames.RemoveAll();
                this->AttribValues.RemoveAll();

                MSXML2::IXMLDOMNamedNodeMapPtr pAttribs = pChild->Getattributes();
                if(pAttribs != NULL)
                {       long nAttriCnt = pAttribs->Getlength();
                        // Iterate over the attributes
                        //
                        for(int i=0; i<nAttriCnt; ++i)
                        {  MSXML2::IXMLDOMNodePtr pAttrib = pAttribs->Getitem(i);
                           if(pAttrib != NULL)
                           {
                              this->AttribNames.Add(  (CString)(const char *) pAttrib->GetnodeName() );

                              _variant_t vVal = pAttrib->GetnodeValue ( );
                              this->AttribValues.Add( (CString)(const char *) _bstr_t(vVal) );
                           }
                        }
                }
        }
        else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_TEXT)
        {   // Tag is #TEXT#
            //
            Tag.Empty();
            Tag = "<#TEXT#>";
            Name.Empty();
            Name = "#TEXT#";

            // TextField Update
            //
            TextField = (const char*)(pChild->text);

            // Update Attribute List have no means
            //
            this->AttribNames.RemoveAll();
            this->AttribValues.RemoveAll();
        }
        else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_CDATA_SECTION)
        {   // Tag is #CDATA#
            //
            Tag.Empty();
            Tag = "<#CDATA#>";
            Name.Empty();
            Name = "#CDATA#";

            // TextField Update
            //
            TextField = (const char*)(pChild->text);

            // Update Attribute List have no means
            //
            this->AttribNames.RemoveAll();
            this->AttribValues.RemoveAll();
        }
        else if (this->Is_MSXML_Node(pChild) == MSXML2::NODE_PROCESSING_INSTRUCTION)
        {  /* Do nothing here */ } 

        return;
}

void XML_PARSER::ChainTag_Add(CString & val)
{
        // Add a tag to the ChainTag
        //
        if(!ChainTag.IsEmpty())
           ChainTag += val;
        else
           ChainTag = val;
}

void XML_PARSER::ChainTag_Remove(int number)
{
        // Remove the n tag to the ChainTag
        //
        for(int bcl = 0; bcl < number; bcl ++)
        {
           int pos = ChainTag.ReverseFind('<');
           if(pos == -1)
              ChainTag.Empty();
           else
              ChainTag = ChainTag.Left(pos);
        }
}

int XML_PARSER::Get_Attribute_Count()
{ return this->AttribNames.GetSize(); }

CString & XML_PARSER::Get_Attribute_Name(int index)
{  lasterror = "XML_PARSER::Get_Attribute_Name(int) failed";

   if(index < 0 || index > this->Get_Attribute_Count())
   {    tmp.Empty();
        return tmp;
   }

   lasterror = ok;
   return this->AttribNames[index]; 
}

CString & XML_PARSER::Get_Attribute_Value(int index)
{  lasterror = "XML_PARSER::Get_Attribute_Value(int) failed";

   if(index < 0 || index > this->Get_Attribute_Count())
   {    tmp.Empty();
        return tmp;
   }

   lasterror = ok;
   return this->AttribValues[index];
   //return this->AttribValues[attrib_index];
}

bool XML_PARSER::Is_Having_Attribute(LPCTSTR Name)
{
   // Create the CString Name Object
   //
   CString sName = Name;
                
   // Clear attribute index
   //
   attrib_index = -1;

   int bcl;
   for(bcl = 0; bcl < this->Get_Attribute_Count() ; bcl++)
   {  // Check if the name is equal
      //
      if(this->AttribNames[bcl] == sName)
      {  // set index fot let user to retrieve value with "Get_Attribute_Value()" method
         //
         attrib_index = bcl;
         return true;
      }
   }
   return false;
}

CString & XML_PARSER::Get_Attribute_Value()
{  // Assume Success
   //
   lasterror = ok;

   if(attrib_index != -1)
         return this->AttribValues[attrib_index];

   // We can't retrieve a Attribute values
   //
   lasterror = "XML_PARSER::Get_Attribute_Value()  : Can't Retrieve an Attribute";
   return lasterror;
}

bool XML_PARSER::_add_lastchild(MSXML2::IXMLDOMNodePtr newNode)
{       // Attach the Node to the document
        //
        if(CurrentNode != NULL)
        {       if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT) return false;
                this->ChainTag_Add( this->Get_CurrentTag() );
                newNode = CurrentNode->appendChild(newNode);
        }
        else if(m_pDocRoot != NULL)
        {       if(Is_MSXML_Node(this->m_pDocRoot) != MSXML2::NODE_ELEMENT) return false;
                this->ChainTag_Add( (CString)(const char*)m_pDocRoot->nodeName );
                newNode = m_pDocRoot->appendChild(newNode);
        }
        else
        {       this->ChainTag.Empty();
                m_pDocRoot = newNode;
                m_plDomDocument->documentElement = m_pDocRoot;
        }

        // Update Current Node (cast operation)
        //
        CurrentNode = newNode;

        // Update information for this Node
        //
        this->Grab_Node_Informations(CurrentNode);

        return true;
}

bool XML_PARSER::_add_firstchild(MSXML2::IXMLDOMNodePtr newNode)
{       // Create Reference Node for the Insertion
        //
        _variant_t NodeRef = (IUnknown *)CurrentNode->firstChild;

        // Attach the Node to the document
        //
        if(CurrentNode != NULL)
        {       if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT) return false;
                this->ChainTag_Add( this->Get_CurrentTag() );
                newNode = CurrentNode->insertBefore(newNode,NodeRef);
        }
        else if(m_pDocRoot != NULL)
        {       if(Is_MSXML_Node(this->m_pDocRoot) != MSXML2::NODE_ELEMENT) return false;
                this->ChainTag_Add( (CString)(const char*)m_pDocRoot->nodeName );
                newNode = m_pDocRoot->insertBefore(newNode,NodeRef);
        }
        else
        {       this->ChainTag.Empty();
                m_pDocRoot = newNode;
                m_plDomDocument->documentElement = m_pDocRoot;
        }

        // Update Current Node (cast operation)
        //
        CurrentNode = newNode;

        // Update information for this Node
        //
        this->Grab_Node_Informations(CurrentNode);

        return true;
}

bool XML_PARSER::_add_before(MSXML2::IXMLDOMNodePtr newNode)
{       if(this->Is_Root())
        {       lasterror = "XML_PARSER::_add_before()   : Can't add node at same level that the root node";
                return false;
        }

        // Create Reference Node for the Insertion
        //
        MSXML2::IXMLDOMNodePtr pParent;
        this->CurrentNode->get_parentNode(&pParent);
        _variant_t NodeRef = (IUnknown *)CurrentNode;


        // Attach the Node to the document
        //
        if(CurrentNode != NULL)
                newNode = pParent->insertBefore(newNode,NodeRef);
        else if(m_pDocRoot != NULL)
                newNode = m_pDocRoot->insertBefore(newNode,NodeRef);
        else
        {       m_pDocRoot = newNode;
                m_plDomDocument->documentElement = m_pDocRoot;
        }

        // Update Current Node (cast operation)
        //
        CurrentNode = newNode;

        // Update information for this Node
        //
        this->Grab_Node_Informations(CurrentNode);

        return true;
}

bool XML_PARSER::_add_after(MSXML2::IXMLDOMNodePtr newNode)
{       // If CurrentNode->NextSibling == NULL then must call Add_LastChildNode on Parent Node
        // Because we can't use InsertBefore on a NULL Reference ;o)
        //
        // We are sure that a Parent node exist because before we control that we aren't not on root node.
        //
        if(CurrentNode->nextSibling == NULL)
        {       // Get Parent Node
                //
                MSXML2::IXMLDOMNodePtr pParent;
                this->CurrentNode->get_parentNode(&pParent);

                // Set Parent node as Current Node
                //
                this->CurrentNode = pParent;
                this->Grab_Node_Informations(pParent);

                // Add Node as Last Child Node
                //
                return (this->Add_LastChildNode(Name));
        }

        // Create Reference Node for the Insertion
        //
        MSXML2::IXMLDOMNodePtr pParent;
        this->CurrentNode->get_parentNode(&pParent);
        _variant_t NodeRef = (IUnknown *)CurrentNode->nextSibling;


        // Attach the Node to the document
        //
        if(CurrentNode != NULL)
                newNode = pParent->insertBefore(newNode,NodeRef);
        else if(m_pDocRoot != NULL)
                newNode = m_pDocRoot->insertBefore(newNode,NodeRef);
        else
        {       m_pDocRoot = newNode;
                m_plDomDocument->documentElement = m_pDocRoot;
        }

        // Update Current Node (cast operation)
        //
        CurrentNode = newNode;

        // Update information for this Node
        //
        this->Grab_Node_Informations(CurrentNode);

        return true;
}

bool XML_PARSER::Add_LastChildCData(LPCTSTR data)
{
     // Nodes members
     //
     MSXML2::IXMLDOMNodePtr newNode;

     CComVariant vType(MSXML2::NODE_CDATA_SECTION);
     newNode = m_plDomDocument->createNode(vType,"","");

     // Attach the Node to the document
     //
     bool Result = this->_add_lastchild(newNode);
     this->Set_TextValue(data);
   
     return Result;
}

bool XML_PARSER::Add_FirstChildCData(LPCTSTR data)
{    // Nodes members
     //
     MSXML2::IXMLDOMNodePtr newNode;

     // If no child then use Add_LastChildCData or CurrentElement node not set yet
     //
     if(this->CurrentNode != NULL)
     {  if( !this->CurrentNode->hasChildNodes() )
             return (this->Add_LastChildCData(data));
     }
     else
         return (this->Add_LastChildCData(data));

     // Create the NODE
     //
     CComVariant vType(MSXML2::NODE_CDATA_SECTION);
     newNode = m_plDomDocument->createNode(vType,"","");
      

     bool Result = this->_add_firstchild(newNode);
     this->Set_TextValue(data);

     return Result;
}

bool XML_PARSER::Add_CDataBefore(LPCTSTR data)
{   // Nodes members
    // IXMLDOMElementPtr CurrentNode;        /* Global Member */
    //
    MSXML2::IXMLDOMNodePtr newNode;

    // Can't use this function on the root node
    //
    if(this->Is_Root())
        return false;
        
    // Create the NODE
    //
    CComVariant vType(MSXML2::NODE_CDATA_SECTION);
    newNode = m_plDomDocument->createNode(vType,"","");

    bool Result = this->_add_before(newNode);
    this->Set_TextValue(data);

    return Result;
}

bool XML_PARSER::Add_CDataAfter(LPCTSTR data)
{   // Nodes members
    //
    MSXML2::IXMLDOMNodePtr newNode;

    // Can't use this function on the root node
    //
    if(this->Is_Root())
    {       lasterror = "XML_PARSER::Add_CDataAfter(LPCTSTR)   : Can't add node at same level that the root node";
            return false;
    }

    // Create the NODE
    //
    CComVariant vType(MSXML2::NODE_CDATA_SECTION);
    newNode = m_plDomDocument->createNode(vType,"","");

    bool Result = this->_add_after(newNode);
    this->Set_TextValue(data);

    return Result;
}

bool XML_PARSER::Add_LastChildNode(LPCTSTR Name)
{
        // Nodes members
        //
        MSXML2::IXMLDOMNodePtr newNode;

        // Create NODE TYPE
        //
        CComVariant vType(MSXML2::NODE_ELEMENT);

        // Create the NODE
        //
        newNode = m_plDomDocument->createNode(vType,Name,"");

        // Attach the Node to the document
        //
        return this->_add_lastchild(newNode);
}

bool XML_PARSER::Add_NodeBefore(LPCTSTR Name)
{       // Nodes members
        // IXMLDOMElementPtr CurrentNode;        /* Global Member */
        //
        MSXML2::IXMLDOMNodePtr newNode;

        // Can't use this function on the root node
        //
        if(this->Is_Root())
                return false;
        // Create NODE TYPE
        //
        CComVariant vType(MSXML2::NODE_ELEMENT);

        // Create the NODE
        //
        newNode = m_plDomDocument->createNode(vType,Name,"");

        return this->_add_before(newNode);
}

bool XML_PARSER::Add_NodeAfter(LPCTSTR Name)
{       // Nodes members
        // IXMLDOMElementPtr CurrentNode;        /* Global Member */
        //
        MSXML2::IXMLDOMNodePtr newNode;

        // Can't use this function on the root node
        //
        if(this->Is_Root())
        {       lasterror = "XML_PARSER::Add_NodeAfter(LPCTSTR)   : Can't add node at same level that the root node";
                return false;
        }

        // Create NODE TYPE
        //
        CComVariant vType(MSXML2::NODE_ELEMENT);

        // Create the NODE
        //
        newNode = m_plDomDocument->createNode(vType,Name,"");

        return this->_add_after(newNode);
}

bool XML_PARSER::Add_FirstChildNode(LPCTSTR Name)
{       // Nodes members
        // IXMLDOMElementPtr CurrentNode;        /* Global Member */
        //
        MSXML2::IXMLDOMNodePtr newNode;
        //IXMLDOMNodeListPtr ChildList;

        // If no child then use Add_LastChildNode or CurrentElement node not set yet
        //
        if(CurrentNode != NULL)
        { if( !this->CurrentNode->hasChildNodes() )
             return (this->Add_LastChildNode(Name));
        }
        else
          return (this->Add_LastChildNode(Name));

        // Create NODE TYPE
        //
        CComVariant vType(MSXML2::NODE_ELEMENT);

        // Create the NODE
        //
        newNode = m_plDomDocument->createNode(vType,Name,"");

        return this->_add_firstchild(newNode);
}

bool XML_PARSER::Set_Attribute(LPCTSTR AttribName,LPCTSTR AttribValue)
{
        // Nodes members
        MSXML2::IXMLDOMElementPtr CurrentElementNode = CurrentNode;

        // Create Attribute variable
        //
        CComVariant sAttribute(AttribValue);

        // Set the new attribute
        //
        if(CurrentNode != NULL)
        {
           CurrentElementNode->setAttribute( AttribName, sAttribute );
           CurrentNode = CurrentElementNode;
           this->Grab_Node_Informations(CurrentNode);
        }
        else
        {   m_pDocRoot->setAttribute( AttribName, sAttribute );
            this->Grab_Node_Informations(m_pDocRoot);
        }

        // Return
        //
        return true;
}

bool XML_PARSER::Remove_Attribute(LPCTSTR AttribName)
{
        if(CurrentNode == NULL)
        {       if(this->m_pDocRoot != NULL)
                        this->CurrentNode = this->m_pDocRoot;
                else
                {       lasterror = "XML_PARSER::Remove_Attribute(LPCTSTR)   : Can't remove attribute on a NULL Node";
                        return false;
                }
        }

        MSXML2::IXMLDOMElementPtr CurrentElementNode = CurrentNode;
        CurrentElementNode->removeAttribute(AttribName);
        CurrentNode = CurrentElementNode;

        return true;
}

bool XML_PARSER::Set_TextValue(LPCTSTR TextValue)
{
        // Nodes members
        // IXMLDOMElementPtr CurrentNode;        /* Global Member */
        //
        MSXML2::IXMLDOMNodePtr newNode;

        // Find if a CHILD TEXT NODE exist or not
        //
        if(CurrentNode == NULL)
        {       if(this->m_pDocRoot != NULL)
                        this->CurrentNode = this->m_pDocRoot;
                else
                {       lasterror = "XML_PARSER::Set_TextValue(LPCTSTR)   : Can't set a Text on a NULL Node";
                        return false;
                }
        }

        if(this->Is_MSXML_Node(this->CurrentNode) == MSXML2::NODE_CDATA_SECTION)
        {
           CComVariant sValue(TextValue);
           this->CurrentNode->put_nodeValue(sValue);

           return true;
        }

        // Rq: a NODE_CDATA_SECTION can't have any childs
        //
        if(this->CurrentNode->hasChildNodes())
        {
                MSXML2::IXMLDOMNodePtr pChild;
                for(pChild = this->CurrentNode->firstChild; pChild != NULL; pChild = pChild->nextSibling)
                {       // Find it's a NODE TEXT
                        //
                        if(this->Is_MSXML_Node(pChild) == MSXML2::NODE_TEXT)
                        {  // A Text Node is found, Replace it now!!
                           //
                           CComVariant sValue(TextValue);
                           pChild->put_nodeValue(sValue);
   
                           return true;
                        }
                }
        }

        // No previous Text was defined before, we can add it.
        //
        if(this->Is_MSXML_Node(this->CurrentNode) == MSXML2::NODE_ELEMENT)
        {   // Create NODE TEXT type
            //
            CComVariant vType(MSXML2::NODE_TEXT);

            // Create the node
            //
            newNode = m_plDomDocument->createTextNode(TextValue);

           // Attach the Node to the document
           //
           newNode = CurrentNode->appendChild(newNode);
        }

        return true;
}

bool XML_PARSER::Save_XML_Document(LPCTSTR strFileName)
{       // Save the XML document
        //
        m_plDomDocument->save(strFileName);

        // Return
        //
        return true;
}

void XML_PARSER::Reset_XML_Document()
{
        // Init
        //
        this->Init_MSXML();
        m_plDomDocument->loadXML("");
        m_pDocRoot = m_plDomDocument->documentElement;
        CurrentNode = NULL;

        // Init our members
        //
        this->AttribNames.RemoveAll();
        this->AttribValues.RemoveAll();
        this->Tag.Empty();
        this->Name.Empty();
        this->ChainTag.Empty();
        this->TextField.Empty();
        attrib_index = -1;
}

void XML_PARSER::Go_to_Root()
{ 
  attrib_index = -1;
  this->CurrentNode = this->m_pDocRoot;
  this->ChainTag.Empty();
}

bool XML_PARSER::Go_to_Child()
{       // Child node
        //
        MSXML2::IXMLDOMNodePtr pChild;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT)
                return false;

        pChild = CurrentNode->firstChild;
        if(pChild == NULL) return false;
                
        // Grab Information from Child node
        //
        attrib_index = -1;
        ChainTag_Add(this->Get_CurrentTag());
        this->Grab_Node_Informations(pChild);

        this->CurrentNode = pChild;
        return true;
}

bool XML_PARSER::Go_to_Parent()
{       // Parent node
        //
        MSXML2::IXMLDOMNodePtr pParent = NULL;

        if(this->CurrentNode == this->m_pDocRoot)
                return false;

        this->CurrentNode->get_parentNode(&pParent);
        this->CurrentNode = pParent;

        attrib_index = -1;
        this->ChainTag_Remove(1);
        this->Grab_Node_Informations(this->CurrentNode);
        return true;
}

bool XML_PARSER::Go_Forward()
{       // Sibling node
        //
        MSXML2::IXMLDOMNodePtr pSibling = NULL;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT && Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_CDATA_SECTION)
                return false;

        pSibling = CurrentNode->nextSibling;
        if(pSibling == NULL) return false;
        CurrentNode = pSibling;

        // Grab Information from Sibling node
        //
        attrib_index = -1;
        this->Grab_Node_Informations(CurrentNode);
        return true;
}

bool XML_PARSER::Go_Backward()
{       // Sibling node
        //
        MSXML2::IXMLDOMNodePtr pSibling;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT && Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_CDATA_SECTION)
                return false;

        pSibling = CurrentNode->previousSibling;
        if(pSibling == NULL) return false;
        CurrentNode = pSibling;

        // Grab Information from Sibling node
        //
        attrib_index = -1;
        this->Grab_Node_Informations(pSibling);
        return true;
}

bool XML_PARSER::Go_to_Child(LPCTSTR NodeName)
{
        // Child node
        //
        MSXML2::IXMLDOMNodePtr pChild;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT)
                return false;

        ChainTag_Add(this->Get_CurrentTag());

        for (pChild = CurrentNode->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
        {
                // Grab Information from Child node
                //
                this->Grab_Node_Informations(pChild);

                if(this->Get_CurrentName() == NodeName)
                {       // Update new Position
                        //
                        attrib_index = -1;
                        this->CurrentNode = pChild;
                        return true;
                }
        }

        // Node Not Found (Restore Node informations)
        //
        this->ChainTag_Remove(1);
        this->Grab_Node_Informations(this->CurrentNode);
        return false;
}

bool XML_PARSER::Go_to_Descendant(LPCTSTR NodeName)
{
        // Child node
        //
        MSXML2::IXMLDOMNodePtr pChild;

        // Current Node before the call method
        //
        MSXML2::IXMLDOMElementPtr pCurrent = this->CurrentNode;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT)
                return false;

        ChainTag_Add(this->Get_CurrentTag());

        for (pChild = CurrentNode->firstChild;  NULL != pChild;  pChild = pChild->nextSibling)
        {
                // Grab Information from Child node
                //
                this->Grab_Node_Informations(pChild);

                if(this->Get_CurrentName() == NodeName)
                {       // Update new Position
                        //
                        attrib_index = -1;
                        this->CurrentNode = pChild;
                        return true;
                }

                // Try to go into Childs of this Child
                //
                this->CurrentNode = pChild;
                if( this->Go_to_Descendant(NodeName) )
                { // We find the approriate node
                  // All is set, we can return
                  //
                  return true;
                }

                // Restore Current node
                //
                this->CurrentNode = pCurrent;
        }

        // Node Not Found (Restore Node informations)
        //
        this->ChainTag_Remove(1);
        this->Grab_Node_Informations(this->CurrentNode);
        return false;
}

bool XML_PARSER::Go_to_Parent(LPCTSTR NodeName)
{
        // Parent node
        //
        MSXML2::IXMLDOMNodePtr pParent = NULL;
        MSXML2::IXMLDOMNodePtr oldCurrent = this->CurrentNode;

        if(this->CurrentNode == this->m_pDocRoot)
                return false;

        CString oldChainTag = this->ChainTag;

        this->CurrentNode->get_parentNode(&pParent);

        while (true)
        {
                this->CurrentNode = pParent;
                this->ChainTag_Remove(1);
                this->Grab_Node_Informations(this->CurrentNode);
                if(this->Get_CurrentName() == NodeName)
                {       attrib_index = -1;
                        return true;
                }

                if(this->CurrentNode == this->m_pDocRoot)
                        break;

                this->CurrentNode->get_parentNode(&pParent);
        }

        // Parent not found
        //
        this->CurrentNode = oldCurrent;
        this->ChainTag = oldChainTag;
        this->Grab_Node_Informations(this->CurrentNode);
        return false;
}

// Go to a Node attached at the same Node than the Current Node (Forward sens)
//
bool XML_PARSER::Go_Forward(LPCTSTR NodeName)
{
        // Sibling node
        //
        MSXML2::IXMLDOMNodePtr pSibling = NULL;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT && Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_CDATA_SECTION)
                return false;

        for (pSibling= CurrentNode;  true ;  pSibling = pSibling->nextSibling)
        {
                if(pSibling == NULL)
                        break;

                // Grab Information from Sibling node
                //
                this->Grab_Node_Informations(pSibling);

                if(this->Get_CurrentName() == NodeName)
                {       // Update new Position
                        //
                        attrib_index = -1;
                        this->CurrentNode = pSibling;
                        return true;
                }
        }

        // Node Not Found (Restore Node informations)
        //
        this->Grab_Node_Informations(this->CurrentNode);
        return false;
}

bool XML_PARSER::Go_Backward(LPCTSTR NodeName)
{
        // Sibling node
        //
        MSXML2::IXMLDOMNodePtr pSibling;

        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT && Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_CDATA_SECTION)
                return false;

        for (pSibling = CurrentNode;  NULL != pSibling;  pSibling = pSibling->previousSibling)
        {
                // Grab Information from Sibling node
                //
                this->Grab_Node_Informations(pSibling);

                if(this->Get_CurrentName() == NodeName)
                {       // Update new Position
                        //
                        attrib_index = -1;
                        this->CurrentNode = pSibling;
                        return true;
                }
        }

        // Node Not Found (Restore Node informations)
        //
        this->Grab_Node_Informations(this->CurrentNode);
        return false;
}

bool XML_PARSER::Remove()
{
        // Parent node
        //
        MSXML2::IXMLDOMNodePtr pParent= NULL;

        if(CurrentNode == NULL)
        {       if(this->m_pDocRoot != NULL)
                        this->CurrentNode = this->m_pDocRoot;
                else
                        return false;
        }

        if(this->CurrentNode != this->m_pDocRoot)
        {       this->CurrentNode->get_parentNode(&pParent);
                pParent->removeChild(this->CurrentNode);
                this->CurrentNode = pParent;
                this->Grab_Node_Informations(this->CurrentNode);
        }
        else
                this->Reset_XML_Document();

        attrib_index = -1;
        return true;
}

bool XML_PARSER::RemoveChild(LPCTSTR NodeName)
{
        // Find the Child
        //
        if(Is_MSXML_Node(this->CurrentNode) != MSXML2::NODE_ELEMENT) return false;

        if( this->Go_to_Child(NodeName) )
        {       int cur_attrib_index = attrib_index;
                bool result = this->Remove();

                attrib_index = cur_attrib_index;
                return result;
        }
        else
                return false;
}

// ***************************
// ** Header XML Management **
// ***************************
bool XML_PARSER::Set_Header(LPCTSTR header,LPCTSTR name,LPCTSTR value)
{
      lasterror = ok;
      bool empty_xml = false;

      CString strHeader = header;
      CString strName = name;
      CString strValue = value;
      
      BSTR bstr;
      CString cstr,cstr2;

      MSXML2::IXMLDOMNodePtr pChild = NULL;           // Working node
      pChild = m_plDomDocument->firstChild;   // Start at first document child

      
      
      if(pChild == NULL)
        empty_xml = true;
      
      while(pChild != NULL)
      { 
         if(pChild == m_pDocRoot) 
         {  // Root document reach, it's done, the processing node wasn't found
            //
            break;
         }
           
         if(pChild->nodeType != MSXML2::NODE_PROCESSING_INSTRUCTION) 
         {  pChild = pChild->nextSibling;          // Go to Next Processing instruction node
            continue;
         }
        
            
         pChild->get_baseName(&bstr);
         cstr = bstr;
         if(cstr == header) 
         {    
           // Correct header, check the correct property
           //
           pChild->get_text(&bstr);
           cstr = bstr;

           int index = cstr.Find(name,0);
           if(index == -1)
           {  // The property doesn't exist on this processing instruction");
              //
               
              // Assume correct constraint about "xml" processing instruction
              //
              {
                 // must have version="xxx" in first
                 // must have standalone="xxx" in last if exist
                 //
                 cstr2.Empty();
                 int standalone_index = cstr.Find(_T("standalone"),0);
                 if(standalone_index != -1)
                 {   cstr2 = cstr.Right( cstr.GetLength() - standalone_index + 1);
                     cstr  = cstr.Left(standalone_index);
                 }
   
                 int version_index = cstr.Find(_T("version"),0);
                 if(version_index == -1 && strHeader == "xml")
                 {   
                    CString strTemp = cstr;
                    cstr = "version=\"1.0\" ";
                    cstr += strTemp;
                 }

                 if(strName != _T("version"))
                    cstr += _T(" ") + strName + _T("=\"") + strValue + _T("\" ") + cstr2;
                 else
                    cstr += cstr2;
              }
              
              // Create the new Processing Instruction node
              //
              HRESULT hr;
              MSXML2::IXMLDOMProcessingInstruction *pIXMLDOMPInstr = NULL;
              hr = m_plDomDocument->raw_createProcessingInstruction(_bstr_t(strHeader), _bstr_t(cstr), &pIXMLDOMPInstr);

              if(SUCCEEDED(hr))
              {   
                 // We succes the creation of the processing instruction
                 // Replace the node
                 //
                 m_plDomDocument->replaceChild(pIXMLDOMPInstr,pChild);
              }
              else
              {  // Arf, i fails the creation, grrr, again
                 //
                 lasterror="XML_PARSER::Set_Header() : Can't create the new processing instruction node";
                 return false;
              }
              return true;
           }
           else
           {  // The processing instruction node exist, must change it's value !! 
              //
              int start,end;
              start = cstr.Find(_T('"'),index);
              if(start == -1)
              {  lasterror = "XML_PARSER::Set_Header() : bad value structure";
                 return false;
              }
              end = cstr.Find('"',start+1);
              if(end == -1) 
              {  lasterror = "XML_PARSER::Set_Header() : bad value structure";
                 return false;
              }

              cstr2 = cstr.Mid(0,start+1) + value + cstr.Mid(end,cstr.GetLength() - end);
              
              MSXML2::IXMLDOMNodePtr m_lpNode = NULL;
              MSXML2::IXMLDOMProcessingInstruction *pIXMLDOMPInstr = NULL;
              HRESULT hr;
         
              hr = m_plDomDocument->raw_createProcessingInstruction(_bstr_t(strHeader), _bstr_t(cstr2), &pIXMLDOMPInstr);

              if(SUCCEEDED(hr))
              {
                 // We succes the creation of the processing instruction
                 // Replace the node
                 //
                 m_plDomDocument->replaceChild(pIXMLDOMPInstr,pChild);
              }
              else
              {  lasterror="XML_PARSER::Set_Header() : Can't create the new processing instruction node";
                 return false;
              }
              
              return true;
           }
        }
          
        pChild = pChild->nextSibling;   // Next Processing instruction node
     }

     // No processing node for our header 
     //
     { 
        if(strName != _T("version") && strHeader == _T("xml"))
          cstr = _T("version=\"1.0\" ") + strName + _T("=\"") + strValue + _T("\"");
        else
          cstr = strName + _T("=\"") + strValue + _T("\"");

        MSXML2::IXMLDOMNodePtr m_lpNode = NULL;
        MSXML2::IXMLDOMProcessingInstruction *pIXMLDOMPInstr = NULL;
        HRESULT hr;

        hr = m_plDomDocument->raw_createProcessingInstruction(_bstr_t(strHeader), _bstr_t(cstr), &pIXMLDOMPInstr);
        if(SUCCEEDED(hr))
        {  
          if(!empty_xml)
          {  _variant_t NodeRef = (IUnknown *)this->m_pDocRoot;
             m_lpNode = NULL;
             m_lpNode = m_plDomDocument->insertBefore(pIXMLDOMPInstr,NodeRef);
             if(m_lpNode == NULL) lasterror = "PARSER_XML::SetHeader() : Can't insert Processing node after the root document";
             return (m_lpNode != NULL);
          }
          else
          {  m_lpNode = NULL;
             m_lpNode = m_plDomDocument->appendChild(pIXMLDOMPInstr);
             if(m_lpNode == NULL) lasterror = "PARSER_XML::SetHeader() : Can't insert Processing node in the empty document";
             return (m_lpNode != NULL);
          }
        }
        
        lasterror = "PARSER_XML::SetHeader() : Can't create new Processing node";
        return false;
     } 
}

bool XML_PARSER::Get_Header(LPCTSTR header,LPCTSTR name,CString & res)
{
      lasterror = ok;

      MSXML2::IXMLDOMNodePtr pChild;                  // Working node
      res.Empty();

      pChild = m_plDomDocument;
      pChild = m_plDomDocument->firstChild;   // Start at first document child

      if(pChild == NULL) 
      {  lasterror = "XML_PARSER::Get_Header() : The XML Document is a null pointer";
         return false;  
      }

      while(pChild != NULL)
      {
          if(pChild->nodeType != MSXML2::NODE_PROCESSING_INSTRUCTION) break;

          BSTR bstr;
          CString cstr;
          
          pChild->get_baseName(&bstr);
          cstr = bstr;
          if(cstr == header) 
          {    // Correct header, check the correct property
               //
               pChild->get_text(&bstr);
               cstr = bstr;

               int index = cstr.Find(name,0);
               if(index == -1)
                  return false;
            
               int start,end;
               start = cstr.Find('"',index);
               if(start == -1) 
               {   lasterror = "XML_PARSER::Get_Header() : bad value structure";
                   return false;
               }

               end = cstr.Find('"',start+1);
               if(end == -1) 
               {   lasterror = "XML_PARSER::Get_Header() : bad value structure";
                   return false;
               }


               res = cstr.Mid(start+1,end-start-1);
               return true;
          }
          
          pChild = pChild->nextSibling;   // Next Processing instruction node
     }
     return false;
}
