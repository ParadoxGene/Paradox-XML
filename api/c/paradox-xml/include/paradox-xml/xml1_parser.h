#ifndef PARADOX_SOFTWARE_C_HEADER_XML1_PARSER
#define PARADOX_SOFTWARE_C_HEADER_XML1_PARSER

#include <paradox-xml/xml1_document.h>

typedef enum paradox_xml1_parser_errno_t {
    PARADOX_XML1_PARSER_SUCCESS,
    PARADOX_XML1_PARSER_NULL_DOCUMENT,
    PARADOX_XML1_PARSER_INVALID_DOCUMENT,
    PARADOX_XML1_PARSER_NULL_INDEX
} paradox_xml1_parser_errno_t;

// Document

// [1] document ::= ( prolog element Misc* ) - ( Char* RestrictedChar Char* )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_document(paradox_str_t xml_string, paradox_xml1_document* document);

// Character Range

// [2] Char ::= [#x1-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_char(paradox_str_t xml_string, const paradox_uint64_t index);
// [2a] RestrictedChar ::= [#x1-#x8] | [#xB-#xC] | [#xE-#x1F] | [#x7F-#x84] | [#x86-#x9F]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_restricted_char(paradox_str_t xml_string, const paradox_uint64_t index);

// White Space

// [3] S ::= (#x20 | #x9 | #xD | #xA)+
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_space(paradox_str_t xml_string, paradox_uint64_t* index);

// Names and Tokens

// [4] NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_name_start_char(paradox_str_t xml_string, const paradox_uint64_t index);
// [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_name_char(paradox_str_t xml_string, const paradox_uint64_t index);
// [5] Name ::= NameStartChar (NameChar)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_name(paradox_str_t xml_string, paradox_uint64_t* index);
// [6] Names ::= Name (#x20 Name)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_names(paradox_str_t xml_string, paradox_uint64_t* index);
// [7] Nmtoken ::= (NameChar)+
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_nm_token(paradox_str_t xml_string, paradox_uint64_t* index);
// [8] Nmtokens ::= Nmtoken (#x20 Nmtoken)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_nm_tokens(paradox_str_t xml_string, paradox_uint64_t* index);

// Literals

// [9] EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"' | "'" ([^%&'] | PEReference | Reference)* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_value(paradox_str_t xml_string, paradox_uint64_t* index);
// [10] AttValue ::= '"' ([^<&"] | Reference)* '"' | "'" ([^<&'] | Reference)* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_value(paradox_str_t xml_string, paradox_uint64_t* index);
// [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'")
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_system_literal(paradox_str_t xml_string, paradox_uint64_t* index);
// [12] PubidLiteral ::= '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pubid_literal(paradox_str_t xml_string, paradox_uint64_t* index);
// [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
PARADOX_XML_API paradox_bool8_t paradox_is_xml1_pubid_char(paradox_str_t xml_string, const paradox_uint64_t index);

// Character Data

// [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_char_data(paradox_str_t xml_string, paradox_uint64_t* index);

// [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_comment(paradox_str_t xml_string, paradox_uint64_t* index);

// Processing Instructions

// [16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pi(paradox_str_t xml_string, paradox_uint64_t* index);

// [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pi_target(paradox_str_t xml_string, paradox_uint64_t* index);

// CDATA Sections

// [18] CDSect ::= CDStart CData CDEnd
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_sect(paradox_str_t xml_string, paradox_uint64_t* index);
// [19] CDStart ::= '<![CDATA['
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_start(paradox_str_t xml_string, paradox_uint64_t* index);
// [20] CData ::= (Char* - (Char* ']]>' Char*))
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_c_data(paradox_str_t xml_string, paradox_uint64_t* index);
// [21] CDEnd ::= ']]>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cd_end(paradox_str_t xml_string, paradox_uint64_t* index);

// Prolog

// [22] prolog ::= XMLDecl Misc* (doctypedecl Misc*)?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_prolog(paradox_str_t xml_string, paradox_uint64_t* index);
// [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_xml_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [24] VersionInfo ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_version_info(paradox_str_t xml_string, paradox_uint64_t* index);
// [25] Eq ::= S? '=' S?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_eq(paradox_str_t xml_string, paradox_uint64_t* index);
// [26] VersionNum ::= '1.1'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_version_num(paradox_str_t xml_string, paradox_uint64_t* index);
// [27] Misc ::= Comment | PI | S
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_misc(paradox_str_t xml_string, paradox_uint64_t* index);

// Document Type Definition

// [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>' [VC: Root Element Type][WFC: External Subset]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_doctypedecl(paradox_str_t xml_string, paradox_uint64_t* index);
// [28a] DeclSep ::= PEReference | S [WFC: PE Between Declarations]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_decl_sep(paradox_str_t xml_string, paradox_uint64_t* index);
// [28b] intSubset ::= (markupdecl | DeclSep)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_int_subset(paradox_str_t xml_string, paradox_uint64_t* index);
// [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment [VC: Proper Declaration/PE Nesting][WFC: PEs in Internal Subset]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_markupdecl(paradox_str_t xml_string, paradox_uint64_t* index);

// External Subset

// [30] extSubset ::= TextDecl? extSubsetDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_subset(paradox_str_t xml_string, paradox_uint64_t* index);
// [31] extSubsetDecl ::= ( markupdecl | conditionalSect | DeclSep)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_subset_decl(paradox_str_t xml_string, paradox_uint64_t* index);

// Standalone Document Declaration

// [32] SDDecl ::= S 'standalone' Eq (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"')) [VC: Standalone Document Declaration]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_sd_decl(paradox_str_t xml_string, paradox_uint64_t* index);

// Element

// [39] element ::= EmptyElemTag | STag content ETag [WFC: Element Type Match][VC: Element Valid]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_element(paradox_str_t xml_string, paradox_uint64_t* index);

// Start-tag

// [40] STag ::= '<' Name (S Attribute)* S? '>' [WFC: Unique Att Spec]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_s_tag(paradox_str_t xml_string, paradox_uint64_t* index);
// [41] Attribute ::= Name Eq AttValue [VC: Attribute Value Type][WFC: No External Entity References][WFC: No < in Attribute Values]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_attribute(paradox_str_t xml_string, paradox_uint64_t* index);

// End-tag

// [42] ETag ::= '</' Name S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_e_tag(paradox_str_t xml_string, paradox_uint64_t* index);

// Content of Elements

// [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_content(paradox_str_t xml_string, paradox_uint64_t* index);

// Tags for Empty Elements

// [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>' [WFC: Unique Att Spec]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_empty_elem_tag(paradox_str_t xml_string, paradox_uint64_t* index);

// Element Type Declaration

// [45] elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>' [VC: Unique Element Type Declaration]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_elementdecl(paradox_str_t xml_string, paradox_uint64_t* index);
// [46] contentspec ::= 'EMPTY' | 'ANY' | Mixed | children
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_contentspec(paradox_str_t xml_string, paradox_uint64_t* index);

// Element-content Models

// [47] children ::= (choice | seq) ('?' | '*' | '+')?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_children(paradox_str_t xml_string, paradox_uint64_t* index);
// [48] cp ::= (Name | choice | seq) ('?' | '*' | '+')?
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_cp(paradox_str_t xml_string, paradox_uint64_t* index);
// [49] choice ::= '(' S? cp ( S? '|' S? cp )+ S? ')' [VC: Proper Group/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_choice(paradox_str_t xml_string, paradox_uint64_t* index);
// [50] seq ::= '(' S? cp ( S? ',' S? cp )* S? ')' [VC: Proper Group/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_seq(paradox_str_t xml_string, paradox_uint64_t* index);

// Mixed-content Declaration

// [51] Mixed ::= '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')' [VC: Proper Group/PE Nesting][VC: No Duplicate Types]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_mixed(paradox_str_t xml_string, paradox_uint64_t* index);

// Attribute-list Declaration

// [52] AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_attlist_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [53] AttDef ::= S Name S AttType S DefaultDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_def(paradox_str_t xml_string, paradox_uint64_t* index);

// Attribute Types

// [54] AttType ::= StringType | TokenizedType | EnumeratedType
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_att_type(paradox_str_t xml_string, paradox_uint64_t* index);
// [55] StringType ::= 'CDATA'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_string_type(paradox_str_t xml_string, paradox_uint64_t* index);

/** [56] TokenizedType ::= 'ID' [VC: ID][VC: One ID per Element Type][VC: ID Attribute Default]
 * | 'IDREF'	[VC: IDREF]
 * | 'IDREFS'	[VC: IDREF]
 * | 'ENTITY'	[VC: Entity Name]
 * | 'ENTITIES'	[VC: Entity Name]
 * | 'NMTOKEN'	[VC: Name Token]
 * | 'NMTOKENS'	[VC: Name Token] */
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_tokenized_type(paradox_str_t xml_string, paradox_uint64_t* index);

// Enumerated Attribute Types

// [57] EnumeratedType ::= NotationType | Enumeration
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enumerated_type(paradox_str_t xml_string, paradox_uint64_t* index);
// [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')' [VC: Notation Attributes][VC: One Notation Per Element Type][VC: No Notation on Empty Element][VC: No Duplicate Tokens]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_notation_type(paradox_str_t xml_string, paradox_uint64_t* index);
// [59] Enumeration ::= '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')' [VC: Enumeration][VC: No Duplicate Tokens]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enumeration(paradox_str_t xml_string, paradox_uint64_t* index);

// Attribute Defaults

// [60] DefaultDecl ::= '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue) [VC: Required Attribute][VC: Attribute Default Value Syntactically Correct][WFC: No < in Attribute Values][VC: Fixed Attribute Default][WFC: No External Entity References]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_default_decl(paradox_str_t xml_string, paradox_uint64_t* index);

// Conditional Section

// [61] conditionalSect ::= includeSect | ignoreSect
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_conditional_sect(paradox_str_t xml_string, paradox_uint64_t* index);
// [62] includeSect ::= '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>' [VC: Proper Conditional Section/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_include_sect(paradox_str_t xml_string, paradox_uint64_t* index);
// [63] ignoreSect ::= '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>' [VC: Proper Conditional Section/PE Nesting]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore_sect(paradox_str_t xml_string, paradox_uint64_t* index);
// [64] ignoreSectContents ::= Ignore ('<![' ignoreSectContents ']]>' Ignore)*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore_sect_contents(paradox_str_t xml_string, paradox_uint64_t* index);
// [65] Ignore ::= Char* - (Char* ('<![' | ']]>') Char*)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ignore(paradox_str_t xml_string, paradox_uint64_t* index);

// Character Reference

// [66] CharRef ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';' [WFC: Legal Character]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_char_ref(paradox_str_t xml_string, paradox_uint64_t* index);

// Entity Reference

// [67] Reference ::= EntityRef | CharRef
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_reference(paradox_str_t xml_string, paradox_uint64_t* index);
// [68] EntityRef ::= '&' Name ';' [WFC: Entity Declared][VC: Entity Declared][WFC: Parsed Entity][WFC: No Recursion]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_ref(paradox_str_t xml_string, paradox_uint64_t* index);
// [69] PEReference ::= '%' Name ';' [VC: Entity Declared][WFC: No Recursion][WFC: In DTD]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_reference(paradox_str_t xml_string, paradox_uint64_t* index);

// Entity Declaration

// [70] EntityDecl ::= GEDecl | PEDecl
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [71] GEDecl ::= '<!ENTITY' S Name S EntityDef S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ge_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [72] PEDecl ::= '<!ENTITY' S '%' S Name S PEDef S? '>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [73] EntityDef ::= EntityValue | (ExternalID NDataDecl?)
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_entity_def(paradox_str_t xml_string, paradox_uint64_t* index);
// [74] PEDef ::= EntityValue | ExternalID
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_pe_def(paradox_str_t xml_string, paradox_uint64_t* index);

// External Entity Declaration

// [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_external_id(paradox_str_t xml_string, paradox_uint64_t* index);
// [76] NDataDecl ::= S 'NDATA' S Name [VC: Notation Declared]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ndata_decl(paradox_str_t xml_string, paradox_uint64_t* index);

// Text Declaration

// [77] TextDecl ::= '<?xml' VersionInfo? EncodingDecl S? '?>'
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_text_decl(paradox_str_t xml_string, paradox_uint64_t* index);

// Well-Formed External Parsed Entity

// [78] extParsedEnt ::= ( TextDecl? content ) - ( Char* RestrictedChar Char* )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_ext_parsed_ent(paradox_str_t xml_string, paradox_uint64_t* index);

// Encoding Declaration

// [80] EncodingDecl ::= S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_encoding_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [81] EncName ::= [A-Za-z] ([A-Za-z0-9._] | '-')*
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_enc_name(paradox_str_t xml_string, paradox_uint64_t* index);

// Notation Declarations

// [82] NotationDecl ::= '<!NOTATION' S Name S (ExternalID | PublicID) S? '>' [VC: Unique Notation Name]
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_notation_decl(paradox_str_t xml_string, paradox_uint64_t* index);
// [83] PublicID ::= 'PUBLIC' S PubidLiteral
PARADOX_XML_API paradox_xml1_parser_errno_t paradox_parse_xml1_public_id(paradox_str_t xml_string, paradox_uint64_t* index);

#endif