/*
********************************************************************************
*
*   File Name:
*       dymem.c
*   Author:
*       SW R&D Department
*   Version:
*       V1.0
*   Description:
*
*
********************************************************************************
*/

#define _DYMEM_MODULE
#define _DYMEM_C

/*-----------------------------------------------------------------------------
|   Includes
+----------------------------------------------------------------------------*/
#include "dymem.h"



/*-----------------------------------------------------------------------------
|   Macros
+----------------------------------------------------------------------------*/
#if 0
#define CFG_DYMEM_DEBUG
#include "stdio.h"
#define dymem_trace printf
#endif



/*-----------------------------------------------------------------------------
|   Enumerations
+----------------------------------------------------------------------------*/
#define MAGIC1    0x55AA55AA
#define MAGIC2    0xAA55AA55



/*-----------------------------------------------------------------------------
|   Typedefs
+----------------------------------------------------------------------------*/
typedef struct _dymem_priv_t dymem_priv_t;

struct _dymem_priv_t
{
	unsigned int  magic1;
	unsigned int  free;
	unsigned int  size;
	dymem_priv_t* last;
	dymem_priv_t* next;
	unsigned int  magic2;
};



/*-----------------------------------------------------------------------------
|   Variables
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Constants
+----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
|   Functions
+----------------------------------------------------------------------------*/
/*--------------------------------------
|	Function Name:
|		dymem_dump
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
#if defined(CFG_DYMEM_DEBUG)
static void dymem_dump(void* priv)
{
	dymem_priv_t* dymem_priv = 0;
	unsigned int nums = 0;

	dymem_priv = (dymem_priv_t*)priv;

	dymem_trace("\r\n-------------------------------------------------------------------------\r\n");

	while (dymem_priv != 0)
	{
		dymem_trace("----------------------------------------\r\n");
		dymem_trace("blk%04d---------------------------------\r\n", nums++);
		dymem_trace("magic1 = %08XH\r\n", dymem_priv->magic1);
		dymem_trace("magic2 = %08XH\r\n", dymem_priv->magic2);
		dymem_trace("size = %d\r\n", dymem_priv->size);
		dymem_trace("free = %d\r\n", dymem_priv->free);
		dymem_trace("last = %08XH\r\n", (unsigned int)dymem_priv->last);
		dymem_trace("this = %08XH\r\n", (unsigned int)dymem_priv);
		dymem_trace("next = %08XH\r\n", (unsigned int)dymem_priv->next);
		dymem_trace("addr last = %08XH\r\n", (unsigned int)dymem_priv->last + sizeof(dymem_priv_t));
		dymem_trace("addr this = %08XH\r\n", (unsigned int)dymem_priv + sizeof(dymem_priv_t));
		dymem_trace("addr next = %08XH\r\n", (unsigned int)dymem_priv->next + sizeof(dymem_priv_t));
		dymem_priv = dymem_priv->next;
	}

	dymem_trace("-------------------------------------------------------------------------\r\n\r\n");
}
#endif

/*--------------------------------------
|	Function Name:
|		dymem_copy
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void dymem_copy(unsigned char* dst, unsigned char* src, unsigned int size)
{
	unsigned int i = 0;

	for(i=0;i<size;i++)
	{
		dst[i] = src[i];
	}
}

/*--------------------------------------
|	Function Name:
|		dymem_generate
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
void* dymem_generate(unsigned char* buffer, unsigned int size)
{
	dymem_priv_t* dymem_priv = 0;
#if defined(CFG_DYMEM_DEBUG)
	dymem_trace("%s\r\n", __FUNCTION__);
#endif

	if ((unsigned int)buffer % 4 != 0) 
	{
		buffer = buffer + 4 - (unsigned int)buffer % 4;
		size = size - 4 + (unsigned int)buffer % 4;
	}

	size = ((size % 4) == 0) ? (size) : (size - 4 + size % 4);

	if (size <= sizeof(dymem_priv_t)) 
	{
#if defined(CFG_DYMEM_DEBUG)
		dymem_trace("error: %s[%d]\r\n", __FUNCTION__, __LINE__);
#endif
		return 0;
	}

	dymem_priv = (dymem_priv_t*)buffer;

	dymem_priv->magic1 = MAGIC1;
	dymem_priv->size = size - sizeof(dymem_priv_t);
	dymem_priv->free = 1;
	dymem_priv->last = 0;
	dymem_priv->next = 0;
	dymem_priv->magic2 = MAGIC2;
	
#if defined(CFG_DYMEM_DEBUG)
	dymem_dump((void*)dymem_priv);
#endif

	return (void*)dymem_priv;
}

/*--------------------------------------
|	Function Name:
|		dymem_alloc
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
void* dymem_alloc(void* priv, unsigned int size)
{
	dymem_priv_t* dymem_priv = 0;
	dymem_priv_t* dymem_priv_child = 0;
#if defined(CFG_DYMEM_DEBUG)
	dymem_trace("%s\r\n", __FUNCTION__);
#endif

	size = (size >= 16) ? size : 16;
	size = ((size % 4) == 0) ? (size) : (size + 4 - size % 4);

	dymem_priv = (dymem_priv_t*)priv;

	while (dymem_priv != 0) 
	{
		if (dymem_priv->magic1 != MAGIC1 \
			|| dymem_priv->magic2 != MAGIC2 \
			)
		{
#if defined(CFG_DYMEM_DEBUG)
			dymem_trace("error: %s[%d]\r\n", __FUNCTION__, __LINE__);
#endif
			return 0;
		}

		if (dymem_priv->free == 1) 
		{
			if (dymem_priv->size > size + sizeof(dymem_priv_t)) 
			{
				dymem_priv_child = (dymem_priv_t*)((unsigned int)dymem_priv + sizeof(dymem_priv_t) + size);
				dymem_priv_child->magic1 = MAGIC1;
				dymem_priv_child->free = 1;
				dymem_priv_child->size = dymem_priv->size - size - sizeof(dymem_priv_t);
				dymem_priv_child->last = dymem_priv;
				dymem_priv_child->next = dymem_priv->next;
				dymem_priv_child->magic2 = MAGIC2;
				dymem_priv->free = 0;
				dymem_priv->size = size;
				dymem_priv->next = dymem_priv_child;
#if defined(CFG_DYMEM_DEBUG)
				dymem_dump((void*)priv);
#endif
				return (void*)((unsigned int)dymem_priv + sizeof(dymem_priv_t));
			}
		}

		dymem_priv = dymem_priv->next;
	}

#if defined(CFG_DYMEM_DEBUG)
	dymem_trace("error: %s[%d]\r\n", __FUNCTION__, __LINE__);
#endif

	return 0;
}

/*--------------------------------------
|	Function Name:
|		dymem_realloc
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
void* dymem_realloc(void* priv, void* buffer, unsigned int size)
{
	dymem_priv_t* dymem_priv = 0;
	void* buffer_new = 0;

	buffer_new = dymem_alloc(priv, size);

	dymem_priv = (dymem_priv_t*)((unsigned int)buffer - sizeof(dymem_priv_t));
	if(dymem_priv->size > size)
	{
		dymem_copy(buffer, buffer_new, size);
	}
	else
	{
		dymem_copy(buffer, buffer_new, dymem_priv->size);
	}

	dymem_free(priv, buffer);

	return buffer_new;
}

/*--------------------------------------
|	Function Name:
|		dymem_free
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
void dymem_free(void* priv, void* buffer)
{
	dymem_priv_t* dymem_priv = 0;
#if defined(CFG_DYMEM_DEBUG)
	dymem_trace("%s\r\n", __FUNCTION__);
#endif

	dymem_priv = (dymem_priv_t*)((unsigned int)buffer - sizeof(dymem_priv_t));

	dymem_priv->free = 1;

	dymem_priv = (dymem_priv_t*)priv;

	while (dymem_priv != 0)
	{
		if (dymem_priv->magic1 != MAGIC1 \
			|| dymem_priv->magic2 != MAGIC2 \
			)
		{
#if defined(CFG_DYMEM_DEBUG)
			dymem_trace("error: %s[%d]\r\n", __FUNCTION__, __LINE__);
#endif
			return;
		}

		if (dymem_priv->free == 1 && dymem_priv->next != 0 && dymem_priv->next->free == 1)
		{
			dymem_priv->size = dymem_priv->size + sizeof(dymem_priv_t) + dymem_priv->next->size;
			dymem_priv->next = dymem_priv->next->next;
			continue;
		}

		dymem_priv = dymem_priv->next;
	}

#if defined(CFG_DYMEM_DEBUG)
	dymem_dump((void*)priv);
#endif
}
