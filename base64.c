#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef uint8_t byte;
typedef uint32_t u32;

#define print_bytes(prefix, ptr, n) do { \
	printf("%s: ", prefix); \
	byte* _ptr = ((byte*)ptr); \
	for(int _b_counter = 0; _b_counter < n; _b_counter++){ \
		printf("%02x ", (int)_ptr[_b_counter]); \
	} printf("\n"); } while(0);

static void* mem_copy(void* dest, void const* src, size_t n){
	return __builtin_memmove(dest, src, n);
}

static void swap_bytes(byte* data, size_t n){
	for(size_t i = 0; i < (n / 2); i += 1){
		byte t = data[i];
		data[i] = data[n - (i + 1)];
		data[n - (i + 1)] = t;
	}
}

static const byte dict[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

static const byte rev_dict[] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
	0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
};

static inline
u32 _base64_to_u24(byte c[3]){
	u32 n = 0;
	n |= (u32)c[0] << 16;
	n |= (u32)c[1] << 8;
	n |= (u32)c[2];
	return n;
}

// static inline
// struct {byte[3] b;} from_u24

static inline
void _base64_encode_cluster(byte in[3], byte out[4]){
	u32 n = _base64_to_u24(in);

	out[0] = dict[(n >> 18) & 63];
	out[1] = dict[(n >> 12) & 63];
	out[2] = dict[(n >> 6)  & 63];
	out[3] = dict[(n >> 0)  & 63];
}

static inline
void decode_cluster(byte in[4], byte out[3]){
	byte t[4] = {0};
	print_bytes("IN", in, 4);
	t[0] = rev_dict[in[0]];
	t[1] = rev_dict[in[1]];
	t[2] = rev_dict[in[2]];
	t[3] = rev_dict[in[3]];

	out[0] = (t[0] << 2) | (t[1] >> 4);
	out[1] = ((t[1] & 15) << 4) | (t[2] >> 2);
	out[2] = ((t[2] & 3) << 6) | t[3];
}

size_t base64_encode(byte const* data, size_t datalen, byte* outbuf, size_t outlen){
	int mod = datalen % 3;

	/* How many groups of 4 bytes have been encoded & how many we can encode. */
	size_t encoded_groups = 0;
	size_t max_groups = outlen / 4;

	for(size_t i = 2; i < datalen; i += 3){
		byte bytes_in[3] = {0};
		byte bytes_out[4] = {0};

		mem_copy(bytes_in, &data[i - 2], 3);
		_base64_encode_cluster(bytes_in, bytes_out);

		if(encoded_groups < max_groups){
			mem_copy(&outbuf[encoded_groups * 4], bytes_out, 4);
			encoded_groups += 1;
		}
	}

	/* Decode remaining */
	if(mod != 0){
		byte bytes_out[4] = {0};
		byte bytes_in[3] = {0};
		mem_copy(bytes_in, &data[datalen - 2], mod);
		_base64_encode_cluster(bytes_in, bytes_out);

		bytes_out[3] = '=';
		if(mod == 1){
			bytes_out[2] = '=';
		}

		if(encoded_groups < max_groups){
			mem_copy(&outbuf[encoded_groups * 4], bytes_out, 4);
			encoded_groups += 1;
		}
	}

	return encoded_groups * 4;
}


int main(){
	char const* msg = "Pog";

	byte encoded[128] = {0};
	size_t n = base64_encode((byte*)msg, __builtin_strlen(msg), encoded, sizeof(encoded) - 1);
	print_bytes("msg", msg, __builtin_strlen(msg));
	printf("%s -> [%zu] %s\n", msg, n, encoded);

	byte cl[4];
	mem_copy(cl, encoded, 4);

	byte dec[3];
	decode_cluster(cl, dec);
	// print_bytes("cl", cl, sizeof(cl));

	// print_bytes("dec", dec, sizeof(dec));
	printf("dec: %.3s\n", dec);

	// printf("%c :: %02x\n", 'T', rev_dict['T']);

	return 0;
}
