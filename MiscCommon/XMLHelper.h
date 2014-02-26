// Copyright 2014 GSI, Inc. All rights reserved.
//
// XML Helper header
//
#ifndef XMLHELPER_H
#define XMLHELPER_H

// Xerces-C++ headers
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>

// STD headers
#include <string>
#include <sstream>
#include <stdexcept>

// MiscCommon
#include "MiscUtils.h"

namespace MiscCommon
{
    /**
     *
     *  @brief Helpers for Xerces XML parser.
     *
     */
    namespace XMLHelper
    {
        /**
         *
         * @brief smart-wrapper around XMLCh class
         *
         */
        class smart_XMLCh
        {
        public:
            XMLCh* m_xmlString;

            smart_XMLCh()
                : m_xmlString(NULL)
            {
            }

            smart_XMLCh(const char* _Str)
            {
                m_xmlString = xercesc::XMLString::transcode(_Str);
            }

            smart_XMLCh(const XMLCh* const _XMLCh)
            {
                if (_XMLCh)
                    m_xmlString = xercesc::XMLString::replicate(_XMLCh);
            }

            ~smart_XMLCh()
            {
                Release();
            }

            operator XMLCh*() const
            {
                return m_xmlString;
            }

            std::string ToString() const
            {
                if (!m_xmlString)
                    return std::string();
                char* szTmp = xercesc::XMLString::transcode(m_xmlString);
                if (!szTmp)
                    return std::string();
                std::string strRetVal(szTmp);
                xercesc::XMLString::release(&szTmp);
                return strRetVal;
            }

            bool operator==(const std::string& _Val)
            {
                return (_Val == this->ToString());
            }

            XMLCh* operator&()
            {
                return m_xmlString;
            }

            void Release()
            {
                if (NULL != m_xmlString)
                    xercesc::XMLString::release(&m_xmlString);
            }
        };
        inline bool operator==(const std::string& _Val1, const smart_XMLCh& _Val2)
        {
            return (_Val1 == _Val2.ToString());
        }
        /**
         *
         * @brief A template function, which helps to retrieve different types of attributes from an XML file.
         * @param[in] _element - XML node to process. Must not be a NULL value.
         * @param[in] _attr - Name of the attribute to read. Must not be a NULL value.
         * @param[in,out] _data - A buffer to keep a return value - value of the attribute. Must not be a NULL value.
         *
         */
        template <class _T>
        void get_attr_value(const xercesc::DOMElement* _element, const char* _attr, _T* _data)
        {
            smart_XMLCh attr_name(_attr);
            smart_XMLCh xmlTmpStr(_element->getAttribute(attr_name));
            std::istringstream str(xmlTmpStr.ToString());
            str >> *_data;
        }
        /**
         *
         * @brief A specialization of the get_attr_value template function with the bool type -- xml value: true or false
         * @param[in] _element - XML node to process. Must not be a NULL value.
         * @param[in] _attr - Name of the attribute to read. Must not be a NULL value.
         * @param[in,out] _data - A buffer to keep a retrieved value of the attribute. Must not be a NULL value.
         *
         */
        template <>
        inline void get_attr_value<bool>(const xercesc::DOMElement* _element, const char* _attr, bool* _data)
        {
            smart_XMLCh attr_name(_attr);
            smart_XMLCh xmlTmpStr(_element->getAttribute(attr_name));
            std::string str(xmlTmpStr.ToString());
            MiscCommon::to_lower(str);
            *_data = !(str.empty() || ("false" == str));
        }
        // TODO: Simplify this template by implementing traits
        /**
         *
         * @brief Returns a Node by the given name. (basic template without implementation)
         *
         */
        template <class _T>
        inline xercesc::DOMNode* GetSingleNodeByName(const _T* _Val, const std::string& _NodeName);
        /**
         *
         * @brief
         *
         */
        template <class _T>
        inline xercesc::DOMNodeList* GetNodesByName(const _T* _Val, const std::string& _NodeName);
        /**
         *
         * @brief
         *
         */
        template <>
        inline xercesc::DOMNodeList* GetNodesByName(const xercesc::DOMNode* _ParentNode, const std::string& _NodeName)
        {
            if (!_ParentNode)
                return NULL;
            const smart_XMLCh ElementName(_NodeName.c_str());

            const xercesc::DOMElement* element(NULL);
            if (xercesc::DOMNode::ELEMENT_NODE == _ParentNode->getNodeType())
            {
                if (xercesc::DOMNode::ELEMENT_NODE == _ParentNode->getNodeType())
                    element = dynamic_cast<const xercesc::DOMElement*>(_ParentNode);
                else
                    return NULL;
            }

            return (element->getElementsByTagName(ElementName));
        }
        /**
         *
         * @brief Returns a Node by the given name. A xercesc::DOMDocument specialization
         * @param[in] _Doc - XML Document object. Must not be a NULL value.
         * @param[in] _NodeName - Name of the child node to find.
         * @return A pointer to the found XML node or NULL in case of error
         *
         */
        template <>
        inline xercesc::DOMNode* GetSingleNodeByName(const xercesc::DOMDocument* _Doc, const std::string& _NodeName)
        {
            if (!_Doc)
                return NULL;
            const smart_XMLCh ElementName(_NodeName.c_str());

            const xercesc::DOMNodeList* list = _Doc->getElementsByTagName(ElementName);
            if (!list)
                return NULL;
            return list->item(0);
        }
        /**
         *
         * @brief Returns a Node by the given name. A xercesc::DOMNode specialization.
         * @param[in] _node - XML Node. Must not be a NULL value.
         * @param[in] _NodeName - Name of the child node to find.
         * @return A pointer to the found XML node or NULL in case of error
         *
         */
        template <>
        inline xercesc::DOMNode* GetSingleNodeByName(const xercesc::DOMNode* _node, const std::string& _NodeName)
        {
            const xercesc::DOMNodeList* list = GetNodesByName(_node, _NodeName);
            if (!list)
                return NULL;
            return list->item(0);
        }
        /**
         *
         * @brief A helper template function, which wraps GetSingleNodeByName template-functions.
         * @param[in] _Node - Could be a pointer to either XML Document or XML Node. Must not be a NULL value.
         * @param[in] _NodeName - Name of the child node to find.
         * @exception std::runtime_error - "can't find XML element [_NodeName]"
         * @return A pointer to the found XML node or an exception will be raised.
         *
         */
        template <class _T>
        inline xercesc::DOMNode* GetSingleNodeByName_Ex(const _T* _Node, const std::string& _NodeName) throw(std::exception)
        {
            xercesc::DOMNode* node = GetSingleNodeByName(_Node, _NodeName.c_str());
            if (!node)
                throw(std::runtime_error("can't find XML element \"" + _NodeName + "\""));
            return node;
        }
        /**
         *
         * @brief A template function, which helps to retrieve a value of xml node.
         * @param[in] _parent_node - XML parent node to process. Must not be a NULL value.
         * @param[in] _node_name - Name of the node which value should be retrieved. Must not be a NULL value.
         * @param[in,out] _data - A buffer to keep a return value. Must not be a NULL value.
         *
         */
        template <class _T>
        void get_node_value(const xercesc::DOMNode* _parent_node, const char* _node_name, _T* _data)
        {
            xercesc::DOMNode* node(GetSingleNodeByName(_parent_node, _node_name));
            if (!node)
                return;
            xercesc::DOMNode* child(node->getFirstChild());
            if (!child)
                return;
            smart_XMLCh xmlTmpStr(child->getNodeValue());
            std::istringstream ss(xmlTmpStr.ToString());
            ss >> *_data;
        }
        /**
         *
         * @brief A specialization of the get_node_value template function with the bool type -- xml value: true or false
         * @param[in] _parent_node - XML parent node to process. Must not be a NULL value.
         * @param[in] _node_name - Name of the node which value should be retrieved. Must not be a NULL value.
         * @param[in,out] _data - A buffer to keep a retrieved node value. Must not be a NULL value.
         *
         */
        template <>
        inline void get_node_value<bool>(const xercesc::DOMNode* _parent_node, const char* _node_name, bool* _data)
        {
            xercesc::DOMNode* node(GetSingleNodeByName(_parent_node, _node_name));
            if (!node)
                return;
            xercesc::DOMNode* child(node->getFirstChild());
            if (!child)
                return;
            smart_XMLCh xmlTmpStr(child->getNodeValue());
            std::string str(xmlTmpStr.ToString());
            MiscCommon::to_lower(str);
            *_data = !(str.empty() || ("false" == str));
        }
    };
};

#endif
