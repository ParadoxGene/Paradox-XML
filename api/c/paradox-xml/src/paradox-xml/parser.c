#include <paradox-xml/parser.h>
#include <stdlib.h>
#include <string.h>

// Prolog Section Declarations

paradox_xml1_errno_t paradox_parse_xml1_prolog(paradox_str_t xml_string, paradox_uint64_t* index);

paradox_xml1_errno_t paradox_parse_xml1_xml_decl(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_version_info(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_eq(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_encoding_decl(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_sd_decl(paradox_str_t xml_string, paradox_uint64_t* index);

paradox_xml1_errno_t paradox_parse_xml1_misc(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_comment(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_pi(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_space(paradox_str_t xml_string, paradox_uint64_t* index);

paradox_xml1_errno_t paradox_parse_xml1_doctype_decl(paradox_str_t xml_string, paradox_uint64_t* index);

//


paradox_xml1_errno_t paradox_parse_xml1_element(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_element* element);



paradox_xml1_errno_t paradox_parse_xml1_content(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_element* element);
paradox_xml1_errno_t paradox_parse_xml1_name(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* name);
paradox_xml1_errno_t paradox_parse_xml1_char_data(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* data);
paradox_xml1_errno_t paradox_parse_xml1_attribute(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_attribute* attribute);

// Miscellaneous Parsing


paradox_xml1_errno_t paradox_parse_xml1_reference(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* reference);
paradox_xml1_errno_t paradox_parse_xml1_cd_sect(paradox_str_t xml_string, paradox_uint64_t* index);

// General Parsing

paradox_xml1_errno_t paradox_parse_xml1_name_start_char(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_name_char(paradox_str_t xml_string, paradox_uint64_t* index);
paradox_xml1_errno_t paradox_parse_xml1_digit(paradox_str_t xml_string, paradox_uint64_t* index);

paradox_bool8_t paradox_is_xml1_space(paradox_str_t xml_string, paradox_uint64_t index);
paradox_bool8_t paradox_is_xml1_name_start_char(paradox_str_t xml_string, paradox_uint64_t index);
paradox_bool8_t paradox_is_xml1_name_char(paradox_str_t xml_string, paradox_uint64_t index);
paradox_bool8_t paradox_is_xml1_char(paradox_str_t xml_string, paradox_uint64_t index);
paradox_bool8_t paradox_is_xml1_restricted_char(paradox_str_t xml_string, paradox_uint64_t index);

PARADOX_XML_API paradox_xml1_errno_t paradox_parse_xml1(paradox_str_t xml_string, paradox_xml1_element* element)
{   // document ::= ( prolog element Misc* )
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;
        goto REMOVE_ELEMENT;
    }
    paradox_uint64_t index = 0;

    result = paradox_parse_xml1_prolog(xml_string, &index);
    if(result != PARADOX_XML1_SUCCESS) goto REMOVE_ELEMENT;

    result = paradox_parse_xml1_element(xml_string, &index, element);
	if(result != PARADOX_XML1_SUCCESS) goto REMOVE_ELEMENT;

	while(paradox_parse_xml1_misc(xml_string, &index) == PARADOX_XML1_SUCCESS) {}

    if(xml_string[index] != '\0')
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto REMOVE_ELEMENT;
    }

    REMOVE_ELEMENT:
    if(result != PARADOX_XML1_SUCCESS)
    {
        if(element != NULL) free(element);
        element = NULL;
    }

    return result;
}

// [22] Prolog Section Implementations

paradox_xml1_errno_t paradox_parse_xml1_prolog(paradox_str_t xml_string, paradox_uint64_t* index)
{   //	prolog ::= XMLDecl Misc* (doctypedecl Misc*)?
    paradox_uint64_t base_index = *index;
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;
        goto INVALID_PROLOG;
    }

    result = paradox_parse_xml1_xml_decl(xml_string, index);
    if(result != PARADOX_XML1_SUCCESS) goto INVALID_PROLOG;
    
    while(paradox_parse_xml1_misc(xml_string, index) == PARADOX_XML1_SUCCESS) {}

    paradox_xml1_errno_t doctype_result = paradox_parse_xml1_doctype_decl(xml_string, index);
	if(doctype_result == PARADOX_XML1_SUCCESS)
	{
		while(paradox_parse_xml1_misc(xml_string, index) == PARADOX_XML1_SUCCESS) {}
	}

	INVALID_PROLOG:
    if(result != PARADOX_XML1_SUCCESS)
    {
        *index = base_index;
    }

    return result;
}
paradox_xml1_errno_t paradox_parse_xml1_xml_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{   // [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
    paradox_uint64_t base_index = *index;
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;    
        goto INVALID_XML_DECL;
    }

    if(strncmp(xml_string + *index, "<?xml", 5))
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_DECL;
    }
    else *index += 5;

    result = paradox_parse_xml1_version_info(xml_string, index);
    if(result != PARADOX_XML1_SUCCESS) goto INVALID_XML_DECL;

    paradox_parse_xml1_encoding_decl(xml_string, index);
    paradox_parse_xml1_sd_decl(xml_string, index);
    paradox_parse_xml1_space(xml_string, index);

    if(strncmp(xml_string + *index, "?>", 2))
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_DECL;
    }
    else *index += 2;

    INVALID_XML_DECL:
    if(result != PARADOX_XML1_SUCCESS)
    {
        *index = base_index;
    }

    return result;
}
paradox_xml1_errno_t paradox_parse_xml1_version_info(paradox_str_t xml_string, paradox_uint64_t* index)
{   // [24] VersionInfo ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
    paradox_uint64_t base_index = *index;
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;    
        goto INVALID_XML_VERSION_INFO;
    }

    result = paradox_parse_xml1_space(xml_string, index);
    if(result != PARADOX_XML1_SUCCESS) goto INVALID_XML_VERSION_INFO;

    if(strncmp(xml_string + *index, "version", 7))
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_VERSION_INFO;
    }
    else *index += 7;

    result = paradox_parse_xml1_eq(xml_string, index);
    if(result != PARADOX_XML1_SUCCESS) goto INVALID_XML_VERSION_INFO;

    paradox_char8_t quote = '\0';
    if('\'' == xml_string[*index] || '"' == xml_string[*index] )
    {
        quote = xml_string[*index];
        *index += 1;
    }
    else
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_VERSION_INFO;
    }

    // [26] VersionNum ::= '1.1'
    if(strncmp(xml_string + *index, "1.1", 3))
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_VERSION_INFO;
    }
    else *index += 3;

    if(quote == xml_string[*index])
    {
        *index += 1;
    }
    else
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_VERSION_INFO;
    }

    result = PARADOX_XML1_SUCCESS;


    INVALID_XML_VERSION_INFO:
    if(result != PARADOX_XML1_SUCCESS)
    {
        *index = base_index;
    }

    return result;
}
paradox_xml1_errno_t paradox_parse_xml1_eq(paradox_str_t xml_string, paradox_uint64_t* index)
{   // [25] Eq ::= S? '=' S?
    paradox_uint64_t base_index = *index;
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;    
        goto INVALID_XML_EQ;
    }

    result = paradox_parse_xml1_space(xml_string, index);
    if(result != PARADOX_XML1_SUCCESS) goto INVALID_XML_EQ;

    if(strncmp(xml_string + *index, "version", 7))
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_EQ;
    }
    else *index += 7;

    INVALID_XML_EQ:
    if(result != PARADOX_XML1_SUCCESS)
    {
        *index = base_index;
    }

    return result;
}

paradox_xml1_errno_t paradox_parse_xml1_misc(paradox_str_t xml_string, paradox_uint64_t* index)
{   // [27] Misc ::= Comment | PI | S
    paradox_uint64_t base_index = *index;
    paradox_xml1_errno_t result;
    if(xml_string == NULL)
    {
        result = PARADOX_XML1_NULL_DOCUMENT;    
        goto INVALID_XML_MISC;
    }

    if( PARADOX_XML1_SUCCESS == paradox_parse_xml1_comment(xml_string, index)
    ||  PARADOX_XML1_SUCCESS == paradox_parse_xml1_pi(xml_string, index)
    ||  PARADOX_XML1_SUCCESS == paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_SUCCESS;
    } 
    else
    {
        result = PARADOX_XML1_INVALID_DOCUMENT;
        goto INVALID_XML_MISC;
    }
    

    INVALID_XML_MISC:
    if(result != PARADOX_XML1_SUCCESS)
    {
        *index = base_index;
    }

    return result;
}

//


paradox_xml1_errno_t paradox_parse_xml1_comment(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_pi(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_encoding_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_sd_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_space(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_doctype_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{

}

paradox_xml1_errno_t paradox_parse_xml1_element(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_element* element)
{

}
paradox_xml1_errno_t paradox_parse_xml1_content(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_element* element)
{

}
paradox_xml1_errno_t paradox_parse_xml1_name(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* name)
{

}
paradox_xml1_errno_t paradox_parse_xml1_char_data(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* data)
{

}
paradox_xml1_errno_t paradox_parse_xml1_attribute(paradox_str_t xml_string, paradox_uint64_t* index, paradox_xml1_attribute* attribute)
{

}

paradox_xml1_errno_t paradox_parse_xml1_reference(paradox_str_t xml_string, paradox_uint64_t* index, paradox_str_t* reference)
{

}
paradox_xml1_errno_t paradox_parse_xml1_cd_sect(paradox_str_t xml_string, paradox_uint64_t* index)
{

}

paradox_xml1_errno_t paradox_parse_xml1_name_start_char(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_name_char(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_xml1_errno_t paradox_parse_xml1_digit(paradox_str_t xml_string, paradox_uint64_t* index)
{

}
paradox_bool8_t paradox_is_xml1_space(paradox_str_t xml_string, paradox_uint64_t index)
{
    
}
paradox_bool8_t paradox_is_xml1_name_start_char(paradox_str_t xml_string, paradox_uint64_t index)
{
    
}
paradox_bool8_t paradox_is_xml1_name_char(paradox_str_t xml_string, paradox_uint64_t index)
{
    
}
paradox_bool8_t paradox_is_xml1_char(paradox_str_t xml_string, paradox_uint64_t index)
{
    
}
paradox_bool8_t paradox_is_xml1_restricted_char(paradox_str_t xml_string, paradox_uint64_t index)
{
    
}