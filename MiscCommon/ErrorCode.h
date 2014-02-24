/************************************************************************/
/**
 * @file ErrorCode.h
 * @brief error cods collection file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2006-05-11
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2006-2007 GSI GridTeam. All rights reserved.
************************************************************************/
#ifndef ERRORCODE_H
#define ERRORCODE_H

// API
#include <errno.h>
#include <stdlib.h>
#include <string.h>
// STD
#include <string>
#include <sstream>

namespace MiscCommon
{

    typedef int ERRORCODE;
    typedef const ERRORCODE ERRORCODE_C;

    ERRORCODE_C erOK(EXIT_SUCCESS);
    ERRORCODE_C erTrue(-1);
    ERRORCODE_C erFalse(-2);
    ERRORCODE_C erError(EXIT_FAILURE);
    ERRORCODE_C erNotImpl(3);

#define SUCCEEDED(er) (er <= erOK)
#define FAILED(er) (!SUCCEEDED(er))

    // General Error codes
    ERRORCODE_C BASE_FOR_GENERAL_ERR(50); // <<<----- BASE counter
    ERRORCODE_C erNULLArg(BASE_FOR_GENERAL_ERR + 1);
    ERRORCODE_C erFILE_NOT_FOUND(BASE_FOR_GENERAL_ERR + 2);

    // XML Error codes
    ERRORCODE_C BASE_FOR_XML_ERR(100); // <<<----- BASE counter
    ERRORCODE_C erXMLInit(BASE_FOR_XML_ERR + 1);
    ERRORCODE_C erXMLReadConfig(BASE_FOR_XML_ERR + 2);
    ERRORCODE_C erXMLNullNode(BASE_FOR_XML_ERR + 3);

    /**
     *
     * @brief Retrieves a string, which represent the last error.
     * @param[in,out] _msg - an error message will be written to this buffer,
     *must not be NULL.
     * @return errno - system error code.
     *
     */
    inline int errno2str(std::string* _msg)
    {
        if (!_msg)
            return erNULLArg;
        char* p = strerror(errno);
        *_msg = p;
        return errno;
    }
    /**
     *
     * @brief Retrieves a string, which represent the last error.
     * @return string, which represent the last error.
     *
     */
    inline std::string errno2str()
    {
        char* p = strerror(errno);
        return std::string(p);
    }
    /**
     *
     * @brief The system_error exception class retrieves a string, which
     *represent the last error
     * @brief and can be thrown when any of system (or functions which support
     *"errno") functions fails.
     *
     */
    class system_error : public std::exception
    {
    public:
        explicit system_error(const std::string& _ErrorPrefix)
        {
            m_errno = errno;
            const char* const szError = strerror(m_errno);
            std::stringstream ss;
            if (!_ErrorPrefix.empty())
                ss << _ErrorPrefix << ". ";
            ss << "System error description [" << m_errno << "]: " << szError;
            m_Msg = ss.str();
        }
        virtual ~system_error() throw()
        {
        }
        virtual const char* what() const throw()
        {
            return m_Msg.c_str();
        }
        int getErrno() const throw()
        {
            return m_errno;
        }

    private:
        std::string m_Msg;
        int m_errno;
    };
};

#endif
