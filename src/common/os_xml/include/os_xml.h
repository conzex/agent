/* Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

/* os_xml C Library */

#pragma once

#include <stdio.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
#define ATTR_NONNULL __attribute__((nonnull))
#define ATTR_NONNULL_ONE __attribute__((nonnull(1)))
#define ATTR_NONNULL_TWO __attribute__((nonnull(2)))
#define ATTR_NONNULL_ONE_TWO __attribute__((nonnull(1, 2)))
#define ATTR_NONNULL_ONE_TWO_THREE_FIVE __attribute__((nonnull(1, 2, 3, 5)))
#define ATTR_PRINTF_TWO_THREE_NONNULL __attribute__((format(printf, 2, 3), nonnull))
#define ATTR_UNUSED __attribute__((unused))
#define UNREFERENCED_PARAMETER(P)
#else
#define ATTR_NONNULL
#define ATTR_NONNULL_ONE
#define ATTR_NONNULL_TWO
#define ATTR_NONNULL_ONE_TWO
#define ATTR_NONNULL_ONE_TWO_THREE_FIVE
#define ATTR_PRINTF_TWO_THREE_NONNULL
#define ATTR_UNUSED
#define UNREFERENCED_PARAMETER(P) (P)
#endif

/* XML Node structure */
typedef struct _xml_node {
    unsigned int key;
    char *element;
    char *content;
    char **attributes;
    char **values;
} xml_node;

#define XML_ERR_LENGTH  128
#define XML_STASH_LEN   2
#define xml_getc_fun(x,y) (x)? _xml_fgetc(x,y) : _xml_sgetc(y)
typedef enum _XML_TYPE { XML_ATTR, XML_ELEM, XML_VARIABLE_BEGIN = '$' } XML_TYPE;

/* XML structure */
typedef struct _OS_XML {
    unsigned int cur;           /* Current position (and last after reading) */
    int fol;                    /* Current position for the xml_access */
    XML_TYPE *tp;               /* Item type */
    unsigned int *rl;           /* Relation in the XML */
    int *ck;                    /* If the item was closed or not */
    unsigned int *ln;           /* Current xml file line */
    unsigned int err_line;      /* Line number of the possible error */
    char **ct;                  /* Content is stored */
    char **el;                  /* The element/attribute name is stored */
    char err[XML_ERR_LENGTH];   /* Error messages are stored in here */
    unsigned int line;          /* Current line */
    char stash[XML_STASH_LEN];  /* Ungot characters stash */
    int stash_i;                /* Stash index */
    FILE *fp;                   /* File descriptor */
    char *string;               /* XML string */
} OS_XML;

typedef xml_node **XML_NODE;

/**
 * @brief Parses a XML file and stores the content in the OS_XML struct.
 *        This legacy method will always fail if the content of a tag is bigger than XML_MAXSIZE.
 *
 * @param file The source file to read.
 * @param lxml The struct to store the result.
 * @return int OS_SUCCESS on success, OS_INVALID otherwise.
 */
int OS_ReadXML(const char *file, OS_XML *lxml) ATTR_NONNULL;

/**
 * @brief Parses a XML file and stores the content in the OS_XML struct.
 *
 * @param file The source file to read.
 * @param lxml The struct to store the result.
 * @param flag_truncate Toggles the behavior in case of the content of a tag is bigger than XML_MAXSIZE:
 *                      truncate on TRUE or fail on FALSE.
 * @return int OS_SUCCESS on success, OS_INVALID otherwise.
 */
int OS_ReadXML_Ex(const char *file, OS_XML *_lxml, bool flag_truncate) ATTR_NONNULL;

/**
 * @brief Parses a XML string and stores the content in the OS_XML struct.
 *        This legacy method will always fail if the content of a tag is bigger than XML_MAXSIZE.
 *
 * @param string The source string to read.
 * @param lxml The struct to store the result.
 * @return int OS_SUCCESS on success, OS_INVALID otherwise.
 */
int OS_ReadXMLString(const char *string, OS_XML *_lxml) ATTR_NONNULL;

/**
 * @brief Parses a XML string and stores the content in the OS_XML struct.
 *
 * @param string The source string to read.
 * @param lxml The struct to store the result.
 * @param flag_truncate Toggles the behavior in case of the content of a tag is bigger than XML_MAXSIZE:
 *                      truncate on TRUE or fail on FALSE.
 * @return int OS_SUCCESS on success, OS_INVALID otherwise.
 */
int OS_ReadXMLString_Ex(const char *string, OS_XML *_lxml, bool flag_truncate) ATTR_NONNULL;

/**
 * @brief Parses a XML string or file.
 *
 * @param _lxml The xml struct.
 * @param flag_truncate Toggles the behavior in case of the content of a tag is bigger than XML_MAXSIZE:
 *                      truncate on TRUE or fail on FALSE.
 * @return int OS_SUCCESS on success, OS_INVALID otherwise.
 */
int ParseXML(OS_XML *_lxml, bool flag_truncate) ATTR_NONNULL;

/* Clear the XML structure memory */
void OS_ClearXML(OS_XML *_lxml) ATTR_NONNULL;

/* Clear a node */
void OS_ClearNode(xml_node **node);


/* Functions to read the XML */

/* Return 1 if element_name is a root element */
unsigned int OS_RootElementExist(const OS_XML *_lxml, const char *element_name) ATTR_NONNULL;

/* Return 1 if the element_name exists */
unsigned int OS_ElementExist(const OS_XML *_lxml, const char **element_name) ATTR_NONNULL;

/* Return the elements "children" of the element_name */
char **OS_GetElements(const OS_XML *_lxml, const char **element_name) ATTR_NONNULL_ONE;

/* Return the elements "children" of the element_name */
xml_node **OS_GetElementsbyNode(const OS_XML *_lxml, const xml_node *node) ATTR_NONNULL_ONE;

/* Return the attributes of the element name */
char **OS_GetAttributes(const OS_XML *_lxml, const char **element_name) ATTR_NONNULL_ONE;

/* Return one value from element_name */
char *OS_GetOneContentforElement(OS_XML *_lxml, const char **element_name) ATTR_NONNULL;

/* Return an array with the content of all entries of element_name */
char **OS_GetElementContent(OS_XML *_lxml, const char **element_name) ATTR_NONNULL;

/* Return an array with the contents of an element_nane */
char **OS_GetContents(OS_XML *_lxml, const char **element_name) ATTR_NONNULL_ONE;

/* Return the value of a specific attribute of the element_name */
char *OS_GetAttributeContent(OS_XML *_lxml, const char **element_name,
                             const char *attribute_name) ATTR_NONNULL_ONE_TWO;

/* Apply the variables to the xml */
int OS_ApplyVariables(OS_XML *_lxml) ATTR_NONNULL;

/* Error from writer */
#define XMLW_ERROR              006
#define XMLW_NOIN               007
#define XMLW_NOOUT              010

/* Write an XML file, based on the input and values to change */
int OS_WriteXML(const char *infile, const char *outfile, const char **nodes,
                const char *oldval, const char *newval) ATTR_NONNULL_ONE_TWO_THREE_FIVE;

/**
 * @brief Get value of an attribute of a node
 * @param node node to find value of attribute
 * @param name name of the attribute
 * @return value of attribute on success. NULL otherwise
 */
const char * w_get_attr_val_by_name(xml_node * node, const char * name);

