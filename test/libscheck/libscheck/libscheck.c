
#include "stdio.h"
#include "../../../libs/private/dymem/dymem.h"
#include "../../../libs/private/dytlv/dytlv.h"

void check_dymem(void)
{
	static unsigned char buffer[8192];
	static void* priv = 0;

	void* buff1 = 0;
	void* buff2 = 0;
	void* buff3 = 0;
	void* buff4 = 0;
	void* buff5 = 0;

	priv = dymem_generate(buffer, sizeof(buffer));
	buff1 = dymem_alloc(priv, 256);
	buff2 = dymem_alloc(priv, 512);
	buff3 = dymem_alloc(priv, 298);
	buff4 = dymem_alloc(priv, 5256);
	buff5 = dymem_alloc(priv, 222);
	dymem_free(priv, buff1);
	dymem_free(priv, buff2);
	dymem_free(priv, buff3);
	dymem_free(priv, buff4);
	dymem_free(priv, buff5);
}

static unsigned char dytlv_buffer[1024*128];
static void* dytlv_priv = 0;
static void* dytlv_alloc(unsigned int size)
{
	return dymem_alloc(dytlv_priv, size);
}
static void dytlv_free(void* buffer)
{
	dymem_free(dytlv_priv, buffer);
}
void check_dytlv(void)
{
	dytlv_t* dytlv = 0;
	dytlv_t* child_1 = 0;
	dytlv_t* child_2 = 0;
	dytlv_t* child_3 = 0;
	unsigned char* hex_tlv = 0;

	dytlv_priv = dymem_generate(dytlv_buffer, sizeof(dytlv_buffer));

	dytlv_init(dytlv_alloc, dytlv_free);

	dytlv = dytlv_create(0x7F00, 0, 0);

	child_1 = dytlv_create(0x9F01, 1, "\xFF");
	dytlv_append_child(dytlv, child_1);

	child_1 = dytlv_create(0x7F00, 0, 0);
	dytlv_append_child(dytlv, child_1);

	child_2 = dytlv_create(0x9F01, 1, "\xFF");
	dytlv_append_child(child_1, child_2);

	child_2 = dytlv_create(0x7F00, 0, 0);
	dytlv_append_child(child_1, child_2);

	child_3 = dytlv_create(0x9F01, 1, "\xFF");
	dytlv_append_child(child_2, child_3);

	child_1 = dytlv_create(0x7F00, 0, 0);
	dytlv_append_child(dytlv, child_1);

	child_2 = dytlv_create(0x9F01, 1, "\xFF");
	dytlv_append_child(child_1, child_2);

	child_2 = dytlv_create(0x7F00, 0, 0);
	dytlv_append_child(child_1, child_2);

	child_3 = dytlv_create(0x9F01, 1, "\xFF");
	dytlv_append_child(child_2, child_3);

	hex_tlv = dytlv_format(dytlv);
}

int main(void)
{
	check_dytlv();
	return 0;
}
