/*
********************************************************************************
*
*   File Name:
*       dytlv.h
*   Author:
*       SW R&D Department
*   Version:
*       V1.0
*   Description:
*
*
********************************************************************************
*/

#ifndef _DYTLV_H
#define _DYTLV_H

#ifdef __cplusplus
extern "C"
{
#endif//__cplusplus

/*-----------------------------------------------------------------------------
|   Includes
+----------------------------------------------------------------------------*/

#ifdef _DYTLV_C
#define GLOBAL
#else
#define GLOBAL  extern
#endif


/*-----------------------------------------------------------------------------
|   Macros
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Enumerations
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Typedefs
+----------------------------------------------------------------------------*/
typedef struct _dytlv_t dytlv_t;

struct _dytlv_t
{
	dytlv_t*       only_parent;
	dytlv_t*       first_child;
	dytlv_t*       last_brother;
	dytlv_t*       next_brother;
	
	unsigned int   tag;
	unsigned int   len;
	void*          val;
};


/*-----------------------------------------------------------------------------
|   Variables
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Constants
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   prototypes
+----------------------------------------------------------------------------*/
GLOBAL unsigned int dytlv_init(void* (*)(unsigned int), void (*)(void*));

GLOBAL dytlv_t* dytlv_parse(unsigned char* hex_tlv);
GLOBAL unsigned char* dytlv_format(dytlv_t* dytlv);

GLOBAL dytlv_t* dytlv_create(unsigned int tag, unsigned int len, unsigned char* val);
GLOBAL dytlv_t* dytlv_find(dytlv_t* dytlv, unsigned int tag);
GLOBAL dytlv_t* dytlv_get_only_parent(dytlv_t* dytlv);
GLOBAL dytlv_t* dytlv_get_first_child(dytlv_t* dytlv);
GLOBAL dytlv_t* dytlv_get_last_brother(dytlv_t* dytlv);
GLOBAL dytlv_t* dytlv_get_next_brother(dytlv_t* dytlv);

GLOBAL unsigned int dytlv_is_composite(dytlv_t* dytlv);

GLOBAL unsigned int dytlv_append_child(dytlv_t* dytlv, dytlv_t* node);
GLOBAL unsigned int dytlv_insert_brother_before(dytlv_t* dytlv, dytlv_t* node);
GLOBAL unsigned int dytlv_insert_brother_after(dytlv_t* dytlv, dytlv_t* node);
GLOBAL unsigned int dytlv_delete(dytlv_t* dytlv);

GLOBAL unsigned int dytlv_modify_tag(dytlv_t* dytlv, unsigned int tag);
GLOBAL unsigned int dytlv_modify_val(dytlv_t* dytlv, unsigned int len, unsigned char* val);

GLOBAL unsigned int dytlv_get_tag(dytlv_t* dytlv);
GLOBAL unsigned int dytlv_get_len(dytlv_t* dytlv);
GLOBAL unsigned char* dytlv_get_val(dytlv_t* dytlv);
GLOBAL unsigned int dytlv_get_tag_l(dytlv_t* dytlv);
GLOBAL unsigned int dytlv_get_len_l(dytlv_t* dytlv);
GLOBAL unsigned int dytlv_get_node_l(dytlv_t* dytlv);

/*
*******************************************************************************
*   End of File
*******************************************************************************
*/

#undef GLOBAL

#ifdef __cplusplus
}
#endif//__cplusplus

/*-----------------------------------------------------------------------------
|   Includes
+----------------------------------------------------------------------------*/

#endif  /* #ifndef _DYTLV_H */
