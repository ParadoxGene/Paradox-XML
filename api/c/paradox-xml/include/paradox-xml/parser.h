#ifndef PARADOX_SOFTWARE_C_HEADER_XML_PARSER
#define PARADOX_SOFTWARE_C_HEADER_XML_PARSER

#include <paradox-xml/element.h>

typedef enum paradox_xml1_errno_t {
    PARADOX_XML1_SUCCESS,
    PARADOX_XML1_NULL_DOCUMENT,
    PARADOX_XML1_INVALID_DOCUMENT
} paradox_xml1_errno_t;

PARADOX_XML_API paradox_xml1_errno_t paradox_parse_xml1(paradox_str_t xml_string, paradox_xml1_element* element);

#endif