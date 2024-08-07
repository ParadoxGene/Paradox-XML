#include <paradox-xml/xml1_parser.h>
#include <paradox-platform/characters.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helpers
void paradox_xml1_parser_next_index(paradox_str_t xml_string, paradox_uint64_t* index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + *index, &num_bytes);
    if(!num_bytes) return;
    (*index) += num_bytes;
}

// Document

// [1] document ::= ( prolog element Misc* ) - ( Char* RestrictedChar Char* )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_document(paradox_str_t xml_string, paradox_xml1_document** document)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string || NULL == document)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    *document = malloc(sizeof(document));

    paradox_uint64_t index = 0;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_prolog(xml_string, &index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_element(xml_string, &index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_misc(xml_string, &index));
    if('\0' == xml_string[index]) result = PARADOX_XML1_PARSER_SUCCESS;
    else result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(NULL != document && NULL != *document)
        {
            free(*document);
            *document = NULL;
        }
    }
    
    return result;
}

// Character Range

// [2] Char ::= ([#x1-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]) - RestrictedChar
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_char(paradox_str_t xml_string, const paradox_uint64_t index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + index, &num_bytes);
    if(!num_bytes) return PARADOX_FALSE;
    if( (0x1 <= code && code <= 0xD7FF)
    ||  (0xE000 <= code && code <= 0xFFFD)
    ||  (0x10000 <= code && code <= 0x10FFFF))
        return !paradox_is_xml1_restricted_char(xml_string, index);
    return PARADOX_FALSE;
}
// [2a] RestrictedChar ::= [#x1-#x8] | [#xB-#xC] | [#xE-#x1F] | [#x7F-#x84] | [#x86-#x9F]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_restricted_char(paradox_str_t xml_string, const paradox_uint64_t index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + index, &num_bytes);
    if(!num_bytes) return PARADOX_FALSE;
    if( (0x1 <= code && code <= 0x8)
    ||  (0xB <= code && code <= 0xC)
    ||  (0xE <= code && code <= 0x1F)
    ||  (0x7F <= code && code <= 0x84)
    ||  (0x86 <= code && code <= 0x9F))
        return PARADOX_TRUE;
    return PARADOX_FALSE;
}

// White Space

// [3] S ::= (#x20 | #x9 | #xD | #xA)+
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_space(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_bool8_t space_found = PARADOX_FALSE;
    do
    {
        switch(xml_string[*index])
        {
        case 0x9:
        case 0xA:
        case 0xD:
        case 0x20:
            space_found = PARADOX_TRUE;
            (*index)++;
            break;
        default: break;
        }
    } while(PARADOX_TRUE == space_found);

    if(PARADOX_TRUE == space_found) result = PARADOX_XML1_PARSER_SUCCESS;
    else result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Names and Tokens

// [4] NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_name_start_char(paradox_str_t xml_string, const paradox_uint64_t index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + index, &num_bytes);
    if(!num_bytes) return PARADOX_FALSE;
    switch(code)
    {
    case ':':
    case '_': return PARADOX_TRUE;
    default:
    {
        if( ('A' <= code && code <= 'Z')
        ||  ('a' <= code && code <= 'z')
        ||  (0xC0 <= code && code <= 0xD6)
        ||  (0xD8 <= code && code <= 0xF6)
        ||  (0xF8 <= code && code <= 0x2FF)
        ||  (0x370 <= code && code <= 0x37D)
        ||  (0x37F <= code && code <= 0x1FFF)
        ||  (0x200C <= code && code <= 0x200D)
        ||  (0x2070 <= code && code <= 0x218F)
        ||  (0x2C00 <= code && code <= 0x2FEF)
        ||  (0x3001 <= code && code <= 0xD7FF)
        ||  (0xF900 <= code && code <= 0xFDCF)
        ||  (0xFDF0 <= code && code <= 0xFFFD)
        ||  (0x10000 <= code && code <= 0xEFFFF))
            return PARADOX_TRUE;
        break;
    }
    }
    return PARADOX_FALSE;
}
// [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_name_char(paradox_str_t xml_string, const paradox_uint64_t index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + index, &num_bytes);
    if(!num_bytes) return PARADOX_FALSE;
    switch(code)
    {
    case ':':
    case '_':
    case '-':
    case '.':
    case 0xB7: return PARADOX_TRUE;
    default:
    {
        if( ('A' <= code && code <= 'Z')
        ||  ('a' <= code && code <= 'z')
        ||  ('0' <= code && code <= '9')
        ||  (0xC0 <= code && code <= 0xD6)
        ||  (0xD8 <= code && code <= 0xF6)
        ||  (0xF8 <= code && code <= 0x2FF)
        ||  (0x300 <= code && code <= 0x36F)
        ||  (0x370 <= code && code <= 0x37D)
        ||  (0x37F <= code && code <= 0x1FFF)
        ||  (0x200C <= code && code <= 0x200D)
        ||  (0x203F <= code && code <= 0x2040)
        ||  (0x2070 <= code && code <= 0x218F)
        ||  (0x2C00 <= code && code <= 0x2FEF)
        ||  (0x3001 <= code && code <= 0xD7FF)
        ||  (0xF900 <= code && code <= 0xFDCF)
        ||  (0xFDF0 <= code && code <= 0xFFFD)
        ||  (0x10000 <= code && code <= 0xEFFFF))
            return PARADOX_TRUE;
        break;
    }
    }
    return PARADOX_FALSE;
}
// [5] Name ::= NameStartChar (NameChar)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_name(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_FALSE == paradox_is_xml1_name_start_char(xml_string, *index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else paradox_xml1_parser_next_index(xml_string, index);

    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_name_char(xml_string, *index)) break;
        paradox_xml1_parser_next_index(xml_string, index);
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [6] Names ::= Name (#x20 Name)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_names(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    result = paradox_parse_xml1_name(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != result) goto INVALID_PARSING;

    while('\0' != xml_string[*index])
    {
        paradox_uint64_t last_index = *index;
        if(0x20 != xml_string[last_index++]) break;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, &last_index)) break;
        *index = last_index;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [7] Nmtoken ::= (NameChar)+
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_nm_token(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_bool8_t name_char_found = PARADOX_FALSE;
    do
    {
        if(PARADOX_TRUE == paradox_is_xml1_name_char(xml_string, *index))
        {
            name_char_found = PARADOX_TRUE;
            paradox_xml1_parser_next_index(xml_string, index);
        }
    } while(PARADOX_TRUE == name_char_found);
    
    if(PARADOX_TRUE == name_char_found) result = PARADOX_XML1_PARSER_SUCCESS;
    else result = PARADOX_XML1_PARSER_INVALID_DOCUMENT; 

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [8] Nmtokens ::= Nmtoken (#x20 Nmtoken)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_nm_tokens(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    result = paradox_parse_xml1_nm_token(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != result) goto INVALID_PARSING;

    while('\0' != xml_string[*index])
    {
        paradox_uint64_t last_index = *index;
        if(0x20 != xml_string[last_index++]) break;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_nm_token(xml_string, &last_index)) break;
        *index = last_index;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Literals

// [9] EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"' | "'" ([^%&'] | PEReference | Reference)* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_value(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_char8_t quote;
    if('"' != xml_string[*index] || '\'' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else
    {
        quote = xml_string[*index];
        (*index)++;
    }
    while('\0' != xml_string[*index])
    {
        if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_pe_reference(xml_string, index)) continue;
        if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_reference(xml_string, index)) continue;
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if('%' != xml_string[*index] && '&' != xml_string[*index] && quote != xml_string[*index])
        {
            paradox_xml1_parser_next_index(xml_string, index);
            continue;
        }
        break;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}
// [10] AttValue ::= '"' ([^<&"] | Reference)* '"' | "'" ([^<&'] | Reference)* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_value(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_char8_t quote;
    if('"' != xml_string[*index] || '\'' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else
    {
        quote = xml_string[*index];
        (*index)++;
    }
    while('\0' != xml_string[*index])
    {
        if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_reference(xml_string, index)) continue;
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if('%' != xml_string[*index] && '&' != xml_string[*index] && quote != xml_string[*index])
        {
            paradox_xml1_parser_next_index(xml_string, index);
            continue;
        }
        break;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}
// [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'")
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_system_literal(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_char8_t quote;
    if('"' != xml_string[*index] || '\'' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else
    {
        quote = xml_string[*index];
        (*index)++;
    }
    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if(quote != xml_string[*index])
        {
            paradox_xml1_parser_next_index(xml_string, index);
            continue;
        }
        break;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}
// [12] PubidLiteral ::= '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pubid_literal(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_char8_t quote;
    if('"' != xml_string[*index] || '\'' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else
    {
        quote = xml_string[*index];
        (*index)++;
    }
    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_pubid_char(xml_string, *index)) break;
        if(('"' == quote) || ('\'' == quote && '\'' != xml_string[*index]))
        {
            paradox_xml1_parser_next_index(xml_string, index);
            continue;
        }
        break;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}
// [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_pubid_char(paradox_str_t xml_string, const paradox_uint64_t index)
{
    size_t num_bytes;
    const paradox_uint32_t code = paradox_utf8_to_codepoint(xml_string + index, &num_bytes);
    if(!num_bytes) return PARADOX_FALSE;
    switch(code)
    {
    case 0xA:
    case 0xD:
    case 0x20:
    case '-':
    case '\'':
    case '(':
    case ')':
    case '+':
    case ',':
    case '.':
    case '/':
    case ':':
    case '=':
    case '?':
    case ';':
    case '!':
    case '*':
    case '#':
    case '@':
    case '$':
    case '_':
    case '%': return PARADOX_TRUE;
    default:
    {
        if( ('A' <= code && code <= 'Z')
        ||  ('a' <= code && code <= 'z')
        ||  ('0' <= code && code <= '9'))
            return PARADOX_TRUE;
        break;
    }
    }
    return PARADOX_FALSE;
}

// Character Data

// [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_char_data(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if('<' == xml_string[*index] || '&' == xml_string[*index]) break;
        if(!strncmp(xml_string + *index, "]]>", 3)) break;
        paradox_xml1_parser_next_index(xml_string, index);
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_comment(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<!--", 4))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 4;
    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if(!strncmp(xml_string + *index, "-->", 3)) break;
        paradox_xml1_parser_next_index(xml_string, index);
    }
    if(strncmp(xml_string + *index, "-->", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// Processing Instructions

// [16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pi(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<?", 2))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 2;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pi_target(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_space(xml_string, index))
    {
        while('\0' != xml_string[*index])
        {
            if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
            if(!strncmp(xml_string + *index, "?>", 2)) break;
            paradox_xml1_parser_next_index(xml_string, index);
        }
    }
    if(strncmp(xml_string + *index, "?>", 2))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 2;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pi_target(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( ('\0' != xml_string[*index] && ('X' == xml_string[*index] || 'x' == xml_string[*index]))
    &&  ('\0' != xml_string[(*index) + 1] && ('M' == xml_string[(*index) + 1] || 'm' == xml_string[(*index) + 1]))
    &&  ('\0' != xml_string[(*index) + 2] && ('L' == xml_string[(*index) + 2] || 'l' == xml_string[(*index) + 2])))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// CDATA Sections

// [18] CDSect ::= CDStart CData CDEnd
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_sect(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_cd_start(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_c_data(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_cd_end(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [19] CDStart ::= '<![CDATA['
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_start(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<![CDATA[", 9))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 9;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [20] CData ::= (Char* - (Char* ']]>' Char*))
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_c_data(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if(!strncmp(xml_string + *index, "]]>", 3)) break;
        paradox_xml1_parser_next_index(xml_string, index);
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [21] CDEnd ::= ']]>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_end(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "]]>", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Prolog

// [22] prolog ::= XMLDecl Misc* (doctypedecl Misc*)?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_prolog(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_xml_decl(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_misc(xml_string, index));
    if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_doctypedecl(xml_string, index))
    {
        while(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_misc(xml_string, index));
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_xml_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<?xml", 5))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 5;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_version_info(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_parse_xml1_encoding_decl(xml_string, index);
    paradox_parse_xml1_sd_decl(xml_string, index);
    paradox_parse_xml1_space(xml_string, index);
    if(strncmp(xml_string + *index, "?>", 2))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 2;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [24] VersionInfo ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_version_info(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(strncmp(xml_string + *index, "version", 7))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 7;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_eq(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_char8_t quote;
    if('"' == xml_string[*index] || '\'' == xml_string[*index])
    {
        quote = xml_string[(*index)++];
    }
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_version_num(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [25] Eq ::= S? '=' S?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_eq(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_parse_xml1_space(xml_string, index);
    if('=' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    paradox_parse_xml1_space(xml_string, index);
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [26] VersionNum ::= '1.1'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_version_num(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "1.1", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [27] Misc ::= Comment | PI | S
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_misc(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_comment(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pi(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Document Type Definition

// [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>' [VC: Root Element Type][WFC: External Subset]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_doctypedecl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [28a] DeclSep ::= PEReference | S [WFC: PE Between Declarations]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_decl_sep(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [28b] intSubset ::= (markupdecl | DeclSep)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_int_subset(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    while(  PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_markupdecl(xml_string, index)
        ||  PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_decl_sep(xml_string, index));
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment [VC: Proper Declaration/PE Nesting][WFC: PEs in Internal Subset]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_markupdecl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// External Subset

// [30] extSubset ::= TextDecl? extSubsetDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_subset(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [31] extSubsetDecl ::= ( markupdecl | conditionalSect | DeclSep)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_subset_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    while(  PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_markupdecl(xml_string, index)
        ||  PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_conditional_sect(xml_string, index)
        ||  PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_decl_sep(xml_string, index));
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Standalone Document Declaration

// [32] SDDecl ::= S 'standalone' Eq (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"')) [VC: Standalone Document Declaration]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_sd_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(strncmp(xml_string + *index, "standalone", 10))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 10;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_eq(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    
    if(!strncmp(xml_string + *index, "\"yes\"", 5) || !strncmp(xml_string + *index, "'yes'", 5) )
    {
        (*index) += 5;
    }
    else if(!strncmp(xml_string + *index, "\"no\"", 4) || !strncmp(xml_string + *index, "'no'", 4) )
    {
        (*index) += 4;
    }
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Element

// [39] element ::= EmptyElemTag | STag content ETag [WFC: Element Type Match][VC: Element Valid]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_element(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Start-tag

// [40] STag ::= '<' Name (S Attribute)* S? '>' [WFC: Unique Att Spec]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_s_tag(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [41] Attribute ::= Name Eq AttValue [VC: Attribute Value Type][WFC: No External Entity References][WFC: No < in Attribute Values]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_attribute(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// End-tag

// [42] ETag ::= '</' Name S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_e_tag(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Content of Elements

// [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_content(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Tags for Empty Elements

// [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>' [WFC: Unique Att Spec]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_empty_elem_tag(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Element Type Declaration

// [45] elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>' [VC: Unique Element Type Declaration]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_elementdecl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [46] contentspec ::= 'EMPTY' | 'ANY' | Mixed | children
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_contentspec(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Element-content Models

// [47] children ::= (choice | seq) ('?' | '*' | '+')?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_children(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_choice(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_seq(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    switch(xml_string[*index])
    {
    case '?':
    case '*':
    case '+':
        (*index)++;
        break;
    default: break;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [48] cp ::= (Name | choice | seq) ('?' | '*' | '+')?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cp(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_choice(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_seq(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    switch(xml_string[*index])
    {
    case '?':
    case '*':
    case '+':
        (*index)++;
        break;
    default: break;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [49] choice ::= '(' S? cp ( S? '|' S? cp )+ S? ')' [VC: Proper Group/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_choice(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}
// [50] seq ::= '(' S? cp ( S? ',' S? cp )* S? ')' [VC: Proper Group/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_seq(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Mixed-content Declaration

// [51] Mixed ::= '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')' [VC: Proper Group/PE Nesting][VC: No Duplicate Types]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_mixed(paradox_str_t xml_string, paradox_uint64_t* index)
{
    return PARADOX_XML1_PARSER_INVALID_DOCUMENT;
}

// Attribute-list Declaration

// [52] AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_attlist_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<!ATTLIST", 9))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    } else (*index) += 9;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_att_def(xml_string, index));
    paradox_parse_xml1_space(xml_string, index);
    if('>' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    } else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [53] AttDef ::= S Name S AttType S DefaultDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_def(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_att_type(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_default_decl(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Attribute Types

// [54] AttType ::= StringType | TokenizedType | EnumeratedType
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_type(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_string_type(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_tokenized_type(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_enumerated_type(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [55] StringType ::= 'CDATA'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_string_type(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "CDATA", 5))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 5;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

/** [56] TokenizedType ::= 'ID' [VC: ID][VC: One ID per Element Type][VC: ID Attribute Default]
 * | 'IDREF'	[VC: IDREF]
 * | 'IDREFS'	[VC: IDREF]
 * | 'ENTITY'	[VC: Entity Name]
 * | 'ENTITIES'	[VC: Entity Name]
 * | 'NMTOKEN'	[VC: Name Token]
 * | 'NMTOKENS'	[VC: Name Token] */
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_tokenized_type(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(!strncmp(xml_string + *index, "ID", 2)) (*index) += 2;
    else if(!strncmp(xml_string + *index, "IDREF", 5)) (*index) += 5;
    else if(!strncmp(xml_string + *index, "IDREFS", 6)) (*index) += 6;
    else if(!strncmp(xml_string + *index, "ENTITY", 6)) (*index) += 6;
    else if(!strncmp(xml_string + *index, "ENTITIES", 8)) (*index) += 8;
    else if(!strncmp(xml_string + *index, "NMTOKEN", 7)) (*index) += 7;
    else if(!strncmp(xml_string + *index, "NMTOKENS", 8)) (*index) += 8;
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// Enumerated Attribute Types

// [57] EnumeratedType ::= NotationType | Enumeration
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enumerated_type(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_notation_type(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_enumeration(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')' [VC: Notation Attributes][VC: One Notation Per Element Type][VC: No Notation on Empty Element][VC: No Duplicate Tokens]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_notation_type(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "NOTATION", 8))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 8;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if('(' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    paradox_parse_xml1_space(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while('\0' != xml_string[*index])
    {
        const paradox_uint64_t last_index = *index;
        paradox_parse_xml1_space(xml_string, index);
        if('|' != xml_string[*index])
        {
            *index = last_index;
            break;
        }
        else (*index)++;
        paradox_parse_xml1_space(xml_string, index);
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
        {
            *index = last_index;
            break;
        }
    }
    paradox_parse_xml1_space(xml_string, index);
    if(')' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}
// [59] Enumeration ::= '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')' [VC: Enumeration][VC: No Duplicate Tokens]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enumeration(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if('(' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    paradox_parse_xml1_space(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_nm_token(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while('\0' != xml_string[*index])
    {
        const paradox_uint64_t last_index = *index;
        paradox_parse_xml1_space(xml_string, index);
        if('|' != xml_string[*index])
        {
            *index = last_index;
            break;
        }
        else (*index)++;
        paradox_parse_xml1_space(xml_string, index);
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_nm_token(xml_string, index))
        {
            *index = last_index;
            break;
        }
    }
    paradox_parse_xml1_space(xml_string, index);
    if(')' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// Attribute Defaults

// [60] DefaultDecl ::= '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue) [VC: Required Attribute][VC: Attribute Default Value Syntactically Correct][WFC: No < in Attribute Values][VC: Fixed Attribute Default][WFC: No External Entity References]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_default_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(!strncmp(xml_string, "#REQUIRED", 9)) (*index) += 9;
    else if(!strncmp(xml_string, "#IMPLIED", 8)) (*index) += 8;
    else
    {
        if(!strncmp(xml_string, "#FIXED", 6))
        {
            (*index) += 6;
            if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
                (*index) = base_index;
        }
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_att_value(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Conditional Section

// [61] conditionalSect ::= includeSect | ignoreSect
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_conditional_sect(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_include_sect(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ignore_sect(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [62] includeSect ::= '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>' [VC: Proper Conditional Section/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_include_sect(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<![", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    paradox_parse_xml1_space(xml_string, index);
    if(strncmp(xml_string + *index, "INCLUDE", 7))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 7;
    paradox_parse_xml1_space(xml_string, index);
    if('[' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ext_subset_decl(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(strncmp(xml_string + *index, "]]>", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [63] ignoreSect ::= '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>' [VC: Proper Conditional Section/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore_sect(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<![", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    paradox_parse_xml1_space(xml_string, index);
    if(strncmp(xml_string + *index, "IGNORE", 6))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 6;
    paradox_parse_xml1_space(xml_string, index);
    if('[' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    while(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_ignore_sect_contents(xml_string, index));
    if(strncmp(xml_string + *index, "]]>", 3))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 3;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [64] ignoreSectContents ::= Ignore ('<![' ignoreSectContents ']]>' Ignore)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore_sect_contents(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ignore(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    while('\0' != xml_string[*index])
    {
        paradox_uint64_t next_index = *index;
        if(strncmp(xml_string + next_index, "<![", 3)) break;
        next_index += 3;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ignore_sect_contents(xml_string, &next_index)) break;
        if(strncmp(xml_string + next_index, "]]>", 3)) break;
        next_index += 3;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ignore(xml_string, &next_index)) break;
        *index = next_index;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [65] Ignore ::= Char* - (Char* ('<![' | ']]>') Char*)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    while('\0' != xml_string[*index])
    {
        if(PARADOX_FALSE == paradox_is_xml1_char(xml_string, *index)) break;
        if(!strncmp(xml_string + *index, "<![", 3)) break;
        if(!strncmp(xml_string + *index, "]]>", 3)) break;
        paradox_xml1_parser_next_index(xml_string, index);
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }
    
    return result;
}

// Character Reference

// [66] CharRef ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';' [WFC: Legal Character]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_char_ref(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(!strncmp(xml_string + *index, "&#x", 3))
    {
        (*index) += 3;
        paradox_bool8_t found = PARADOX_FALSE;
        while('\0' != xml_string[*index])
        {
            if(paradox_char8_ishex(xml_string[*index]))
            {
                found = PARADOX_TRUE;
                (*index)++;
            }
            else break;
        }
        if(PARADOX_FALSE == found)
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
    }
    else if(!strncmp(xml_string + *index, "&#", 2))
    {
        (*index) += 2;
        paradox_bool8_t found = PARADOX_FALSE;
        while('\0' != xml_string[*index])
        {
            if(paradox_char8_isdigit(xml_string[*index]))
            {
                found = PARADOX_TRUE;
                (*index)++;
            }
            else break;
        }
        if(PARADOX_FALSE == found)
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
    }
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(';' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Entity Reference

// [67] Reference ::= EntityRef | CharRef
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_reference(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_entity_ref(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_char_ref(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [68] EntityRef ::= '&' Name ';' [WFC: Entity Declared][VC: Entity Declared][WFC: Parsed Entity][WFC: No Recursion]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_ref(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if('&' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(';' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [69] PEReference ::= '%' Name ';' [VC: Entity Declared][WFC: No Recursion][WFC: In DTD]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_reference(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if('%' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(';' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Entity Declaration

// [70] EntityDecl ::= GEDecl | PEDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_ge_decl(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pe_decl(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [71] GEDecl ::= '<!ENTITY' S Name S EntityDef S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ge_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<!ENTITY", 8))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 8;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_entity_def(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_parse_xml1_space(xml_string, index);
    if('>' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [72] PEDecl ::= '<!ENTITY' S '%' S Name S PEDef S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<!ENTITY", 8))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 8;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if('%' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pe_def(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_parse_xml1_space(xml_string, index);
    if('>' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [73] EntityDef ::= EntityValue | (ExternalID NDataDecl?)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_def(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_entity_value(xml_string, index)) {}
    else if(PARADOX_XML1_PARSER_SUCCESS == paradox_parse_xml1_external_id(xml_string, index))
    {
        paradox_parse_xml1_ndata_decl(xml_string, index);
    }
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [74] PEDef ::= EntityValue | ExternalID
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_def(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_entity_value(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_external_id(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// External Entity Declaration

// [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_external_id(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(!strncmp(xml_string + *index, "SYSTEM", 6))
    {
        (*index) += 6;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_system_literal(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
    }
    else if(!strncmp(xml_string + *index, "PUBLIC", 6))
    {
        (*index) += 6;
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pubid_literal(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
        if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_system_literal(xml_string, index))
        {
            result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
            goto INVALID_PARSING;
        }
    }
    else
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [76] NDataDecl ::= S 'NDATA' S Name [VC: Notation Declared]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ndata_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(strncmp(xml_string + *index, "NDATA", 5))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 5;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Text Declaration

// [77] TextDecl ::= '<?xml' VersionInfo? EncodingDecl S? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_text_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<?xml", 5))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 5;
    paradox_parse_xml1_version_info(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_encoding_decl(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_parse_xml1_space(xml_string, index);
    if(strncmp(xml_string + *index, "?>", 2))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 2;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Well-Formed External Parsed Entity

// [78] extParsedEnt ::= ( TextDecl? content ) - ( Char* RestrictedChar Char* )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_parsed_ent(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    paradox_parse_xml1_text_decl(xml_string, index);
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_content(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Encoding Declaration

// [80] EncodingDecl ::= S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_encoding_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(strncmp(xml_string + *index, "encoding", 8))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 8;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_eq(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_char8_t quote;
    if('"' == xml_string[*index] || '\'' == xml_string[*index])
    {
        quote = xml_string[(*index)++];
    }
    else
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_enc_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(quote != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [81] EncName ::= [A-Za-z] ([A-Za-z0-9._] | '-')*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enc_name(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(!isalpha(xml_string[*index]))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    while('\0' != xml_string[*index])
    {
        paradox_bool8_t found = PARADOX_FALSE;
        switch(xml_string[*index])
        {
        case '-':
        case '_':
        case '.':
        {
            found = PARADOX_TRUE;
            break;
        }
        default:
        {
            if(isalnum(xml_string[*index])) found = PARADOX_TRUE;
            break;
        }
        }
        if(PARADOX_TRUE == found) (*index)++;
        else break;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}

// Notation Declarations

// [82] NotationDecl ::= '<!NOTATION' S Name S (ExternalID | PublicID) S? '>' [VC: Unique Notation Name]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_notation_decl(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "<!NOTATION", 10))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 10;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_name(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if( PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_external_id(xml_string, index)
    &&  PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_public_id(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    paradox_parse_xml1_space(xml_string, index);
    if('>' != xml_string[*index])
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index)++;
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}
// [83] PublicID ::= 'PUBLIC' S PubidLiteral
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_public_id(paradox_str_t xml_string, paradox_uint64_t* index)
{
    paradox_xml1_parser_errno_t result;
    if(NULL == xml_string)
    {
        result = PARADOX_XML1_PARSER_NULL_DOCUMENT;
        goto INVALID_PARSING;
    }
    else if(NULL == index)
    {
        result = PARADOX_XML1_PARSER_NULL_INDEX;
        goto INVALID_PARSING;
    }
    const paradox_uint64_t base_index = *index;

    if(strncmp(xml_string + *index, "PUBLIC", 6))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    else (*index) += 6;
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_space(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    if(PARADOX_XML1_PARSER_SUCCESS != paradox_parse_xml1_pubid_literal(xml_string, index))
    {
        result = PARADOX_XML1_PARSER_INVALID_DOCUMENT;
        goto INVALID_PARSING;
    }
    result = PARADOX_XML1_PARSER_SUCCESS;

    INVALID_PARSING:
    if(result != PARADOX_XML1_PARSER_SUCCESS)
    {
        if(PARADOX_XML1_PARSER_NULL_INDEX != result) *index = base_index;
    }

    return result;
}