#include <stdio.h>
#include <sys/memory_info.h>

#define is_kb(x) ((x) >= 1024)
#define is_mb(x) ((x) >= 1024 * 1024)
#define is_gb(x) ((x) >= 1024 * 1024 * 1024)

#define to_kb(x) ((x) / 1024)
#define to_mb(x) ((x) / 1024 / 1024)
#define to_gb(x) ((x) / 1024 / 1024 / 1024)

void format_memory_ussage(char* out_buf, uint64_t ussage) {
	if (is_gb(ussage)) {
		sprintf(out_buf, "%d,%d GB", to_gb(ussage), to_mb(ussage) % 1024);
	} else if (is_mb(ussage)) {
		sprintf(out_buf, "%d,%d MB", to_mb(ussage), to_kb(ussage) % 1024);
	} else if (is_kb(ussage)) {
		sprintf(out_buf, "%d,%d KB", to_kb(ussage), ussage % 1024);
	} else {
		sprintf(out_buf, "%d B", ussage);
	}
}

int main() {
	memory_info_t info = memory_info();

	char total_memory_str[32] = { 0 };
	char free_memory_str[32] = { 0 };
	char used_memory_str[32] = { 0 };
	char reserved_memory_str[32] = { 0 };

	format_memory_ussage(total_memory_str, info.total);
	format_memory_ussage(free_memory_str, info.free);
	format_memory_ussage(used_memory_str, info.used);
	format_memory_ussage(reserved_memory_str, info.reserved);

	printf("Total memory: %s\n", total_memory_str);
	printf("Free memory: %s\n", free_memory_str);
	printf("Used memory: %s\n", used_memory_str);
	printf("Reserved memory: %s\n", reserved_memory_str);
}