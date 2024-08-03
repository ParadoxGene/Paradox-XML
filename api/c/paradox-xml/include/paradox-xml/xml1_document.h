#ifndef PARADOX_SOFTWARE_C_HEADER_XML1_DOCUMENT
#define PARADOX_SOFTWARE_C_HEADER_XML1_DOCUMENT

#include <paradox-xml/xml1_element.h>

typedef struct paradox_xml1_document {
    paradox_xml1_element* root;
    paradox_str_t error;
} paradox_xml1_document;

#endif