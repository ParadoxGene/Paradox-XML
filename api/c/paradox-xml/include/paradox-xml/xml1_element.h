#ifndef PARADOX_SOFTWARE_C_HEADER_XML_ELEMENT
#define PARADOX_SOFTWARE_C_HEADER_XML_ELEMENT

#include <paradox-xml/xml1_attribute.h>

typedef struct paradox_xml1_element
{
    paradox_str_t tag;
    paradox_str_t value;
    struct paradox_xml1_element* children;
    paradox_xml1_attribute* attributes;

} paradox_xml1_element;

#endif