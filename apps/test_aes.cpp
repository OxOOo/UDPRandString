#include <aes.h>
#include <cassert>

int main(int argc, char *argv[]) {
	
	/* 256 bit key */
	uint8_t key[] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13,
		0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f};

	uint8_t in[] = {
		0x00, 0x11, 0x22, 0x33,
		0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb,
		0xcc, 0xdd, 0xee, 0xff};
	
	uint8_t out[sizeof(in)], out2[sizeof(in)];

	aes_encode(key, sizeof(key), in, sizeof(in), out);

	printf("out:\n");
	for (int i = 0; i < 4; i++) {
		printf("%x %x %x %x ", out[4*i+0], out[4*i+1], out[4*i+2], out[4*i+3]);
	}
	printf("\n");

	aes_decode(key, sizeof(key), out, sizeof(out), out2);

	printf("msg:\n");
	for (int i = 0; i < 4; i++) {
		printf("%x %x %x %x ", out2[4*i+0], out2[4*i+1], out2[4*i+2], out2[4*i+3]);
	}
	printf("\n");

	for(int i = 0; i < (int)sizeof(in); i ++) {
		assert(in[i] == out2[i]);
	}
	printf("test success\n");

    return 0;
}