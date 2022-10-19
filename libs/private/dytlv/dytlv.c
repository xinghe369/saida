/*
********************************************************************************
*
*   File Name:
*       dytlv.c
*   Author:
*       SW R&D Department
*   Version:
*       V1.0
*   Description:
*
*
********************************************************************************
*/

#define _DYTLV_MODULE
#define _DYTLV_C

/*-----------------------------------------------------------------------------
|   Includes
+----------------------------------------------------------------------------*/
#include "dytlv.h"



/*-----------------------------------------------------------------------------
|   Macros
+----------------------------------------------------------------------------*/
#if 1
#define CFG_DYTLV_DEBUG
#include "stdio.h"
#define dytlv_trace printf
#endif

#define LONG_HH(val)        ((((unsigned int)(val)) >> 24) & 0xff)
#define LONG_HL(val)        ((((unsigned int)(val)) >> 16) & 0xff)
#define LONG_LH(val)        ((((unsigned int)(val)) >> 8) & 0xff)
#define LONG_LL(val)        (((unsigned int)(val)) & 0xff)

#define BIT(idx)            ((unsigned int)0x01 << (idx))
#define BIT_GET(var, idx)   ((((var) & BIT(idx)) == 0) ? 0 : 1)
#define BIT_SET(var, idx)   ((var) |= BIT(idx))
#define BIT_CLR(var, idx)   ((var) &= ~(BIT(idx)))

/*-----------------------------------------------------------------------------
|   Enumerations
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Typedefs
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Variables
+----------------------------------------------------------------------------*/
static void* (*dytlv_alloc)(unsigned int) = 0;
static void (*dytlv_free)(void*) = 0;



/*-----------------------------------------------------------------------------
|   Constants
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Functions
+----------------------------------------------------------------------------*/
static unsigned int dytlv_hex_tlv_get_t_l(unsigned char* hex_tlv);
static unsigned int dytlv_hex_tlv_get_l_l(unsigned char* hex_tlv);

#if defined(CFG_DYTLV_DEBUG)
static void dytlv_dump_space(unsigned int level) 
{
	unsigned int i = 0;

	for (i = 0; i < level; i++) 
	{
		dytlv_trace("    ");
	}
}

void dytlv_trace_value(const unsigned char* pbuf, unsigned int cnt)
{
	while (cnt-- != 0)
	{
		dytlv_trace("%02XH ", *(pbuf++));
	}
	dytlv_trace("\r\n");
}

void dytlv_dump(dytlv_t* dytlv, unsigned int level)
{
	dytlv_t* next = 0;
	dytlv_t* dest = 0;

	dytlv_dump_space(level);
	dytlv_trace("\r\n");

	dytlv_dump_space(level);
	dytlv_trace("TAG: %08XH LEN: %d NODE LEN: %d\r\n", dytlv->tag, dytlv->len, dytlv_get_node_l(dytlv));

	dytlv_dump_space(level);
	if (dytlv_is_composite(dytlv)) 
	{
		dytlv_trace("Is Composite : %s\r\n", "YES");
		next = dytlv->first_child;

		while (next != 0)
		{
			dytlv_dump(next, level + 1);
			next = next->next_brother;
		}
	}
	else 
	{
		dytlv_trace("Is Composite : %s\r\n", "NO");
		dytlv_dump_space(level);
		dytlv_trace_value(dytlv->val, dytlv->len);
	}

	dytlv_dump_space(level);
	dytlv_trace("\r\n");
}
#endif

/*--------------------------------------
|	Function Name:
|		dytlv_memcpy
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void dytlv_memcpy(void* dest, const void* src, unsigned int n)
{
	unsigned int i;
	unsigned char* buffer_dest = (unsigned char*)dest;
	unsigned char* buffer_src = (unsigned char*)src;

	for (i = 0; i < n; i++) 
	{
		buffer_dest[i] = buffer_src[i];
	}
}

/*--------------------------------------
|	Function Name:
|		dytlv_delete_nodes
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void dytlv_delete_nodes(dytlv_t* dytlv)
{
	dytlv_t* dest = 0;
	dytlv_t* next = 0;

	dest = dytlv;

	if (dest == 0)
	{
		return;
	}

	if (dest->first_child != 0)
	{
		next = dest->first_child;
		while (next != 0)
		{
			dytlv_delete_nodes(next);
			next = next->next_brother;
		}
	}

	if (dest->val != 0)
	{
		dytlv_free(dest->val);
	}

	dytlv_free(dest);
}

/*--------------------------------------
|	Function Name:
|		dytlv_update_len
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void dytlv_update_len(dytlv_t* dytlv)
{
	dytlv_t* parent = 0;
	dytlv_t* next = 0;
	unsigned int len = 0;

	parent = dytlv->only_parent;
	while (parent != 0)
	{
		len = 0;
		next = parent->first_child;
		while (next != 0) 
		{
			len += dytlv_get_node_l(next);

			next = next->next_brother;
		}

		parent->len = len;

		parent = parent->only_parent;
	}
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_get_t
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static unsigned int dytlv_hex_tlv_get_t(unsigned char* hex_tlv)
{
	unsigned int T;

	if (hex_tlv == 0) 
	{
		return 0;
	}

	if (((hex_tlv[0] & 0x1F) == 0x1F) && ((hex_tlv[1] & 0x80) == 0x80) && ((hex_tlv[2] & 0x80) == 0x80)) 
	{
		T = (hex_tlv[0] << 24) + (hex_tlv[1] << 16) + (hex_tlv[2] << 8) + hex_tlv[3];
	}
	else if (((hex_tlv[0] & 0x1F) == 0x1F) && ((hex_tlv[1] & 0x80) == 0x80)) 
	{
		T = (hex_tlv[0] << 16) + (hex_tlv[1] << 8) + hex_tlv[2];
	}
	else if ((hex_tlv[0] & 0x1F) == 0x1F)
	{
		T = (hex_tlv[0] << 8) + hex_tlv[1];
	}
	else if (hex_tlv[0] == 0x00)
	{
		return 0;
	}
	else 
	{
		T = hex_tlv[0];
	}

	return T;
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_get_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static unsigned int dytlv_hex_tlv_get_l(unsigned char* hex_tlv)
{
	unsigned int L;

	if (hex_tlv == 0) 
	{
		return 0;
	}

	hex_tlv += dytlv_hex_tlv_get_t_l(hex_tlv);

	if ((hex_tlv[0] & 0x80) == 0x80) 
	{
		if ((hex_tlv[0] & 0x7F) == 1) 
		{
			L = hex_tlv[1];
		}
		else if ((hex_tlv[0] & 0x7F) == 2) 
		{
			L = BIT(8) * hex_tlv[1];
			L += hex_tlv[2];
		}
		else if ((hex_tlv[0] & 0x7F) == 3)
		{
			L = BIT(16) * hex_tlv[1];
			L += BIT(8) * hex_tlv[2];
			L += hex_tlv[3];
		}
		else 
		{
			L = 0;
		}
	}
	else 
	{
		L = hex_tlv[0];
	}

	return L;
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_get_v
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static unsigned char* dytlv_hex_tlv_get_v(unsigned char* hex_tlv)
{
	if (hex_tlv == 0)
	{
		return 0;
	}

	return hex_tlv + dytlv_hex_tlv_get_t_l(hex_tlv) + dytlv_hex_tlv_get_l_l(hex_tlv);
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_get_t_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static unsigned int dytlv_hex_tlv_get_t_l(unsigned char* hex_tlv)
{
	unsigned int len = 0;

	if (hex_tlv == 0)
	{
		return 0;
	}

	len = dytlv_hex_tlv_get_t(hex_tlv);
	if (len >= BIT(24)) 
	{
		len = 4;
	}
	else if (len >= BIT(16)) 
	{
		len = 3;
	}
	else if (len >= BIT(8))
	{
		len = 2;
	}
	else 
	{
		len = 1;
	}

	return len;
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_get_l_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static unsigned int dytlv_hex_tlv_get_l_l(unsigned char* hex_tlv)
{
	unsigned int len;

	if (hex_tlv == 0)
	{
		return 0;
	}

	hex_tlv += dytlv_hex_tlv_get_t_l(hex_tlv);

	if ((hex_tlv[0] & 0x80) == 0x80)
	{
		if ((hex_tlv[0] & 0x7F) == 1)
		{
			len = 2;
		}
		else if ((hex_tlv[0] & 0x7F) == 2)
		{
			len = 3;
		}
		else if ((hex_tlv[0] & 0x7F) == 3)
		{
			len = 4;
		}
		else
		{
			len = 0;
		}
	}
	else
	{
		len = 1;
	}

	return len;
}

/*--------------------------------------
|	Function Name:
|		dytlv_hex_tlv_new
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void dytlv_hex_tlv_new(unsigned char* hex_tlv, unsigned int tag, unsigned int len, unsigned char* val)
{
	unsigned int tlvLen, loop;

	if ((hex_tlv == 0) || (len != 0 && val == 0)) 
	{
		return;
	}
	
	tlvLen = 0;
	if (tag >= BIT(24)) 
	{
		if (((LONG_HH(tag) & 0x1F) != 0x1F) || !BIT_GET(tag, 23) || !BIT_GET(tag, 15) || BIT_GET(tag, 7)) 
		{
			return;
		}
		
		hex_tlv[tlvLen++] = LONG_HH(tag);
		hex_tlv[tlvLen++] = LONG_HL(tag);
		hex_tlv[tlvLen++] = LONG_LH(tag);
		hex_tlv[tlvLen++] = LONG_LL(tag);
	}
	else if (tag >= BIT(16)) 
	{
		if (((LONG_HL(tag) & 0x1F) != 0x1F) || !BIT_GET(tag, 15) || BIT_GET(tag, 7)) 
		{
			return;
		}
		
		hex_tlv[tlvLen++] = LONG_HL(tag);
		hex_tlv[tlvLen++] = LONG_LH(tag);
		hex_tlv[tlvLen++] = LONG_LL(tag);
	}
	else if (tag >= BIT(8)) 
	{
		if (((LONG_LH(tag) & 0x1F) != 0x1F) || BIT_GET(tag, 7)) 
		{
			return;
		}
		
		hex_tlv[tlvLen++] = LONG_LH(tag);
		hex_tlv[tlvLen++] = LONG_LL(tag);
	}
	else 
	{
		if (((LONG_LL(tag) & 0x1F) == 0x1F) || LONG_LL(tag) == 0x00) 
		{
			return;
		}
		
		hex_tlv[tlvLen++] = LONG_LL(tag);
	}
	
	if (len < BIT(7)) 
	{
		hex_tlv[tlvLen++] = LONG_LL(len);
	}
	else if (len >= BIT(7) && len < BIT(8)) 
	{
		hex_tlv[tlvLen++] = 0x80 + 1;
		hex_tlv[tlvLen++] = LONG_LL(len);
	}
	else if (len >= BIT(8) && len < BIT(16)) 
	{
		hex_tlv[tlvLen++] = 0x80 + 2;
		hex_tlv[tlvLen++] = LONG_LH(len);
		hex_tlv[tlvLen++] = LONG_LL(len);
	}
	else if (len >= BIT(16) && len < BIT(24)) 
	{
		hex_tlv[tlvLen++] = 0x80 + 3;
		hex_tlv[tlvLen++] = LONG_HL(len);
		hex_tlv[tlvLen++] = LONG_LH(len);
		hex_tlv[tlvLen++] = LONG_LL(len);
	}
	else 
	{
		return;
	}

	for (loop = 0; loop < len; loop++) 
	{
		hex_tlv[tlvLen++] = val[loop];
	}

	return;
}

/*--------------------------------------
|	Function Name:
|		dytlv_init
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_init(void* (*fp_alloc)(unsigned int), void (*fp_free)(void*))
{
	dytlv_alloc = fp_alloc;
	dytlv_free = fp_free;

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_parse
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_parse(unsigned char* hex_tlv)
{
	dytlv_t* dytlv = 0;
	dytlv_t* child = 0;
	dytlv_t* last = 0;
	dytlv_t* dest = 0;
	unsigned char* ptr = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	dytlv = dytlv_alloc(sizeof(dytlv_t));

	dytlv->only_parent = 0;
	dytlv->first_child = 0;
	dytlv->last_brother = 0;
	dytlv->next_brother = 0;
	dytlv->tag = dytlv_hex_tlv_get_t(hex_tlv);

	if ((hex_tlv[0] & 0x20) == 0x20)
	{
		dytlv->len = dytlv_hex_tlv_get_l(hex_tlv);
		dytlv->val = 0;

		ptr = dytlv_hex_tlv_get_v(hex_tlv);
		for (i = 0; i < dytlv_hex_tlv_get_l(hex_tlv);) 
		{
			dest = dytlv_parse(ptr);

			dest->only_parent = dytlv;
			dest->last_brother = last;
			dest->next_brother = 0;

			if (dest->last_brother != 0)
			{
				last->next_brother = dest;
			}

			if (dytlv->first_child == 0) 
			{
				dytlv->first_child = dest;
			}

			last = dest;

			j = dytlv_hex_tlv_get_t_l(ptr) + dytlv_hex_tlv_get_l_l(ptr) + dytlv_hex_tlv_get_l(ptr);
			i += j;
			ptr += j;
		}
	}
	else
	{
		if (dytlv_hex_tlv_get_l(hex_tlv) > 0)
		{
			dytlv->val = dytlv_alloc(dytlv_hex_tlv_get_l(hex_tlv));
			dytlv_memcpy(dytlv->val, dytlv_hex_tlv_get_v(hex_tlv), dytlv->len);
		}
		else
		{
			dytlv->val = 0;
		}

		dytlv->len = dytlv_hex_tlv_get_l(hex_tlv);
	}

#if defined(CFG_DYTLV_DEBUG)
	dytlv_dump(dytlv, 0);
#endif

	return dytlv;
}

/*--------------------------------------
|	Function Name:
|		dytlv_format
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned char* dytlv_format(dytlv_t* dytlv)
{
	unsigned char* hex_tlv = 0;
	unsigned char* hex_tlv_val = 0;
	unsigned int hex_tlv_val_len = 0;
	unsigned int hex_tlv_val_len_l = 0;
	unsigned char* hex_tlv_node = 0;
	unsigned int hex_tlv_node_len = 0;
	dytlv_t* next = 0;

#if defined(CFG_DYTLV_DEBUG)
	dytlv_dump(dytlv, 0);
#endif

	hex_tlv = dytlv_alloc(dytlv_get_node_l(dytlv));

	if (dytlv->first_child != 0)
	{
		hex_tlv_val = dytlv_alloc(dytlv->len);

		hex_tlv_val_len = 0;

		next = dytlv->first_child;
		while (next != 0)
		{
			hex_tlv_node = dytlv_format(next);
			hex_tlv_node_len = dytlv_get_node_l(next);
			dytlv_memcpy(&hex_tlv_val[hex_tlv_val_len], hex_tlv_node, hex_tlv_node_len);
			hex_tlv_val_len += hex_tlv_node_len;
			dytlv_free(hex_tlv_node);
			next = next->next_brother;
		}

		dytlv_hex_tlv_new(hex_tlv, dytlv->tag, hex_tlv_val_len, hex_tlv_val);

		dytlv_free(hex_tlv_val);
	}
	else
	{
		dytlv_hex_tlv_new(hex_tlv, dytlv->tag, dytlv->len, dytlv->val);
	}

#if defined(CFG_DYTLV_DEBUG)
	dytlv_trace("hex_tlv[%d]\r\n", dytlv_get_node_l(dytlv));
	dytlv_trace_value(hex_tlv, dytlv_get_node_l(dytlv));
#endif

	return hex_tlv;
}

/*--------------------------------------
|	Function Name:
|		dytlv_create
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_create(unsigned int tag, unsigned int len, unsigned char* val)
{
	dytlv_t* dytlv = 0;

	dytlv = dytlv_alloc(sizeof(dytlv_t));

	dytlv->only_parent = 0;
	dytlv->first_child = 0;
	dytlv->last_brother = 0;
	dytlv->next_brother = 0;
	dytlv->tag = 0;
	dytlv->len = 0;
	dytlv->val = 0;

	dytlv_modify_tag(dytlv, tag);
	dytlv_modify_val(dytlv, len, val);

	return dytlv;
}

/*--------------------------------------
|	Function Name:
|		dytlv_find
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_find(dytlv_t* dytlv, unsigned int tag)
{
	dytlv_t* dest = 0;
	dytlv_t* next = 0;

	if (dytlv->tag == tag) 
	{
		return dytlv;
	}

	if (dytlv->first_child != 0) 
	{
		dest = dytlv_find(dytlv->first_child, tag);
		if (dest != 0)
		{
			return dest;
		}
	}

	next = dytlv->next_brother;

	while (next != 0)
	{
		dest = dytlv_find(next, tag);
		if (dest != 0)
		{
			return dest;
		}

		next = next->next_brother;
	}

	return 0;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_only_parent
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_get_only_parent(dytlv_t* dytlv)
{
	return dytlv->only_parent;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_first_child
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_get_first_child(dytlv_t* dytlv)
{
	return dytlv->first_child;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_last_brother
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_get_last_brother(dytlv_t* dytlv)
{
	return dytlv->last_brother;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_next_brother
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
dytlv_t* dytlv_get_next_brother(dytlv_t* dytlv)
{
	return dytlv->next_brother;
}


/*--------------------------------------
|	Function Name:
|		dytlv_is_composite
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_is_composite(dytlv_t* dytlv)
{
	unsigned int tag = 0;

	tag = dytlv->tag;

	while (tag >= 0x100)
	{
		tag >>= 8;
	}

	if (tag & 0x20)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*--------------------------------------
|	Function Name:
|		dytlv_append_child
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_append_child(dytlv_t* dytlv, dytlv_t* node)
{
	dytlv_t* next = 0;

	if (!dytlv_is_composite(dytlv))
	{
		return 0;
	}

	if (dytlv->first_child == 0)
	{
		dytlv->first_child = node;
	}
	else
	{
		next = dytlv->first_child;

		while (next->next_brother != 0)
		{
			next = next->next_brother;
		}

		next->next_brother = node;
		node->last_brother = next;
	}

	node->only_parent = dytlv;

	dytlv_update_len(node);

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_insert_brother_before
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_insert_brother_before(dytlv_t* dytlv, dytlv_t* node)
{
	if (node->only_parent != 0) 
	{
		return 0;
	}

	if (dytlv->last_brother != 0) 
	{
		dytlv->last_brother->next_brother = node;
	}
	else 
	{
		node->last_brother = 0;
	}

	node->next_brother = dytlv;
	node->only_parent = dytlv;

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_insert_brother_after
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_insert_brother_after(dytlv_t* dytlv, dytlv_t* node)
{
	if (node->only_parent != 0)
	{
		return 0;
	}

	if (dytlv->next_brother != 0)
	{
		dytlv->next_brother->last_brother = node;
	}
	else
	{
		node->next_brother = 0;
	}

	node->last_brother = dytlv;
	node->only_parent = dytlv;

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_delete
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_delete(dytlv_t* dytlv)
{
	if (dytlv == 0)
	{
		return 0;
	}

	if (dytlv->only_parent != 0 && dytlv->only_parent->first_child == dytlv)
	{
		dytlv->only_parent->first_child = dytlv->next_brother;
	}

	if (dytlv->last_brother != 0)
	{
		dytlv->last_brother->next_brother = dytlv->next_brother;
	}

	if (dytlv->next_brother != 0)
	{
		dytlv->next_brother->last_brother = dytlv->last_brother;
	}

	dytlv_update_len(dytlv);

	dytlv_delete_nodes(dytlv);

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_modify_tag
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_modify_tag(dytlv_t* dytlv, unsigned int tag)
{
	dytlv->tag = tag;

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_modify_val
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_modify_val(dytlv_t* dytlv, unsigned int len, unsigned char* val)
{
	if (dytlv->val == 0)
	{
		dytlv->val = dytlv_alloc(len);
		dytlv->len = len;
	}
	else if (dytlv->len != len)
	{
		dytlv_free(dytlv->val);
		dytlv->val = dytlv_alloc(len);
		dytlv->len = len;
	}

	dytlv_memcpy(dytlv->val, val, len);

	dytlv_update_len(dytlv);

	return 1;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_tag
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_get_tag(dytlv_t* dytlv)
{
	return dytlv->tag;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_len
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_get_len(dytlv_t* dytlv)
{
	return dytlv->len;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_val
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned char* dytlv_get_val(dytlv_t* dytlv)
{
	return dytlv->val;
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_tag_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_get_tag_l(dytlv_t* dytlv)
{
	if (dytlv->tag >= BIT(24)) 
	{
		return 4;
	}
	else if (dytlv->tag >= BIT(16)) 
	{
		return 3;
	}
	else if (dytlv->tag >= BIT(8)) 
	{
		return 2;
	}
	else {
		return 1;
	}
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_len_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_get_len_l(dytlv_t* dytlv)
{
	if (dytlv->len < BIT(7)) 
	{
		return 1;
	}
	else if (dytlv->len >= BIT(7) && dytlv->len < BIT(8)) 
	{
		return 2;
	}
	else if (dytlv->len >= BIT(8) && dytlv->len < BIT(16)) 
	{
		return 3;
	}
	else if (dytlv->len >= BIT(16) && dytlv->len < BIT(24)) 
	{
		return 4;
	}
	else 
	{
		return 0;
	}
}

/*--------------------------------------
|	Function Name:
|		dytlv_get_node_l
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
unsigned int dytlv_get_node_l(dytlv_t* dytlv)
{
	return dytlv_get_tag_l(dytlv) + dytlv_get_len_l(dytlv) + dytlv_get_len(dytlv);;
}
