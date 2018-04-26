#include <stdint.h>
#include <stdlib.h>
#include <string>

std::string base64_encode(const unsigned char *src, size_t len);
std::string b64decode(const void* data, const size_t len);

