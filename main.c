#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sodium.h>
#include <secp256k1.h>
#include "hd-wallet.h"

secp256k1_context *ctx;

int read_seed_from_stdin(uint8_t *seed, size_t seedlen)
{
	ssize_t bytes_read;

	bytes_read = read(STDIN_FILENO, seed, seedlen);
	if (bytes_read < 0)
	{
		perror("read");
		return -1;
	}
	else if ((size_t)bytes_read != seedlen)
	{
		error_print("read", "insufficient entropy");
		return -1;
	}

	return 0;
}

int main(void)
{
	int ret;
	uint8_t seed[SEED_MIN_ENTROPY_SIZE];
	size_t seedlen = sizeof(seed);
	struct s_wallet_node master_node = {0};

	if (sodium_init() < 0)
	{
		error_print("sodium_init", "couldn't be initialized");
		return EXIT_FAILURE;
	}

	ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
	if (ctx == NULL)
	{
		error_print("secp256k1_context_create", "failed to create context!");
		return EXIT_FAILURE;
	}

	if (isatty(STDIN_FILENO))
		randombytes_buf(seed, seedlen);
	else
	{
		ret = read_seed_from_stdin(seed, seedlen);
		if (ret == -1)
			goto cleanup;
	}

	ret = node_master_generate(seed, seedlen, &master_node);
	if (ret == -1)
		goto cleanup;

	node_dump(&master_node);

	return EXIT_SUCCESS;

cleanup:
	secp256k1_context_destroy(ctx);
	sodium_memzero(seed, seedlen);

	return EXIT_FAILURE;
}