// Copyright 2014 GSI, Inc. All rights reserved.
//
// IXMLPersist is a persistence interface.
//
#ifndef IXMLPERSIST_H
#define IXMLPERSIST_H

// MiscCommon
#include "XMLHelper.h"

namespace MiscCommon
{
/**
 *
 * @brief Declares an XML Persist Interface for user's class
 * @param[in] _T - name of the parent class.
 * @note Example
 * @code
 *
 class CMyClass : public MiscCommon::IXMLPersistImpl<CMyClass>
 {
 ...
   public:
       DECLARE_XMLPERSIST_IMPL(CMyClass);
 ...
 };

 * @endcode
 *
 */
#define DECLARE_XMLPERSIST_IMPL(_T) friend class MiscCommon::IXMLPersistImpl<_T>;
    /**
     *
     * @brief The IXMLPersistImpl interface is a class of the XML persistance.
     *
     */
    template <class _T>
    struct IXMLPersistImpl
    {
        void Read(xercesc::DOMNode* _element)
        {
            _T* pThis = reinterpret_cast<_T*>(this);
            pThis->ReadXmlCfg(_element);
        }
        void Write(xercesc::DOMNode* _element)
        {
            _T* pThis = reinterpret_cast<_T*>(this);
            pThis->WriteXmlCfg(_element);
        }
    };
/**
 *
 * @brief The ::BEGIN_READ_XML_NODE macro precedes the sequence of ::READ_NODE_VALUE.
 * @param[in] _T - Name of the parent class.
 * @param[in] _ELEMENT_NAME - Name of the XML element to read.
 *
 */
#define BEGIN_READ_XML_NODE(_T, _ELEMENT_NAME)                                                                         \
    void ReadXmlCfg(xercesc::DOMNode* _element)                                                                        \
    {                                                                                                                  \
        const std::string str("An internal error has been detected. Can't read configuration of " + std::string(#_T) + \
                              " manager, ");                                                                           \
        if (!_element)                                                                                                 \
            throw std::invalid_argument(str + "DOMNode is NULL.");                                                     \
        MiscCommon::XMLHelper::smart_XMLCh ElementName(_ELEMENT_NAME);                                                 \
        xercesc::DOMElement* config_element(dynamic_cast<xercesc::DOMElement*>(_element));                             \
        if (!config_element)                                                                                           \
            throw std::runtime_error(str + "element " + std::string(#_ELEMENT_NAME) + " is missing");                  \
        xercesc::DOMNodeList* list(config_element->getElementsByTagName(ElementName));                                 \
        if (!list)                                                                                                     \
            throw std::runtime_error(str + "element " + std::string(#_ELEMENT_NAME) + " is missing");                  \
        xercesc::DOMNode* node(list->item(0));                                                                         \
        if (!node)                                                                                                     \
            throw std::runtime_error(str + "element " + std::string(#_ELEMENT_NAME) + " is missing");                  \
        xercesc::DOMElement* elementConfig(NULL);                                                                      \
        if (xercesc::DOMNode::ELEMENT_NODE == node->getNodeType())                                                     \
            elementConfig = dynamic_cast<xercesc::DOMElement*>(node);                                                  \
        if (!elementConfig)                                                                                            \
            throw std::runtime_error(str + "empty XML document");
/**
 *
 * @brief Reads attributes of the node from XML file.
 * @param[in] ELEMENT_NAME - Name of the XML element to read.
 * @param[in,out] VAR - A buffer where value of the XML elements should be stored in.
 *
 */
#define READ_NODE_ATTR(ELEMENT_NAME, VAR) MiscCommon::XMLHelper::get_attr_value(elementConfig, ELEMENT_NAME, &VAR);
/**
 *
 * @brief
 *
 */
#define READ_NODE_VALUE(NODE_NAME, VAR) MiscCommon::XMLHelper::get_node_value(elementConfig, NODE_NAME, &VAR);

/**
 *
 * @brief Closes the sequence of READ_NODE_VALUE.
 *
 */
#define END_READ_XML_NODE }
/**
 *
 * @brief The BEGIN_READ_XML_CFG(_T) macro precedes the sequence of ::READ_NODE_VALUE.
 * @param[in] _T - Name of the parent class.
 * @note Example
 * @code
 *
 class CMyClass : public MiscCommon::IXMLPersistImpl<CMyClass>
 {
 ...
   public:
       DECLARE_XMLPERSIST_IMPL(CMyClass);

   private:
       // IXMLPersist implementation
       BEGIN_READ_XML_CFG(CCatalogManager)
       READ_NODE_VALUE( "lfc_host", m_Data.m_sLFCHost )
       READ_NODE_VALUE( "lfc_wrkdir", m_Data.m_sWrkDir )
       READ_NODE_VALUE( "lfc_session_comment", m_Data.m_sLFCSessionComment )
       END_READ_XML_CFG
 ...
 };

 * @endcode
 *
 */
#define BEGIN_READ_XML_CFG(_T) BEGIN_READ_XML_NODE(_T, "config")
    /**
     *
     * @brief Closes the sequence of ::READ_NODE_VALUE.
     * @note see the example of BEGIN_READ_XML_CFG(_T)
     *
     */

#define END_READ_XML_CFG }
/**
 *
 * @brief
 *
 */
#define BEGIN_WRITE_XML_CFG(_T)                      \
    void WriteXmlCfg(xercesc::DOMNode* /*_element*/) \
    {
/**
 *
 * @brief
 *
 */
#define END_WRITE_XML_CFG }
};

#endif
