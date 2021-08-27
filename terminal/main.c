#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <sys/read.h>
#include <sys/spawn.h>
#include <errno.h>

char** terminal_envp;

char** argv_split(char* str) {
	int len = strlen(str);

	int argc = 1;
	bool quote_open = false;
	bool double_quote_open = false;
	bool special_char_next = false;

	for (int i = 0; i < len; i++) {
		if(str[i] == ' ') {
			if (!quote_open && !double_quote_open) {
				argc++;
			}
		} else if (str[i] == '\\') {
			special_char_next = true;
		} else if (str[i] == '\"') {
			if (special_char_next || double_quote_open) {
				special_char_next = false;
			} else {
				quote_open = !quote_open;
			}
		} else if (str[i] == '\'') {
			if (special_char_next || quote_open) {
				special_char_next = false;
			} else {
				double_quote_open = !double_quote_open;
			}
		}
	}

	char** argv = malloc(sizeof(char*) * (argc + 1));

	argc = 1;
	argv[0] = &str[0];

	quote_open = false;
	double_quote_open = false;
	special_char_next = false;

	for (int i = 0; i < len; i++) {
		if(str[i] == ' ') {
			if (!quote_open && !double_quote_open) {
				argv[argc] = &str[i + 1];
				str[i] = 0;
				argc++;
			}
		} else if (str[i] == '\\') {
			special_char_next = true;
		} else if (str[i] == '\"') {
			if (special_char_next || double_quote_open) {
				special_char_next = false;
			} else {
				quote_open = !quote_open;
			}
		} else if (str[i] == '\'') {
			if (special_char_next || quote_open) {
				special_char_next = false;
			} else {
				double_quote_open = !double_quote_open;
			}
		}
	}

	return argv;
}

void command_received(char* command) {
	if (strcmp(command, (char*)"test") == 0) {
		printf("Yay this terminal works!");
	} else {
		const char** argv = (const char**) argv_split(command);
		const char** envp = (const char**) terminal_envp; //Maybe use actual enviromental vars?

		errno = 0;
		task* t = spawn(argv[0], argv, envp);

		if (errno != 0) {
			goto error;
		}

		while (true) {
			__asm__ __volatile__("pause" :: : "memory");
		}

	error:
		free(argv);
		printf("Error: command not found: %s", command);
	}
}

bool is_quote_open(char* command) {
	bool quote_open = false;
	bool double_quote_open = false;
	bool special_char_next = false;

	int len = strlen(command);

	for (int i = 0; i < len; i++) {
		if (command[i] == '\\') {
			special_char_next = true;
		} else if (command[i] == '\"') {
			if (special_char_next || double_quote_open) {
				special_char_next = false;
			} else {
				quote_open = !quote_open;
			}
		} else if (command[i] == '\'') {
			if (special_char_next || quote_open) {
				special_char_next = false;
			} else {
				double_quote_open = !double_quote_open;
			}
		}
	}

	return double_quote_open || quote_open;
}

int main(int argc, char* argv[], char* envp[]) {
	printf("\nTerminal initialising...\n");

	terminal_envp = envp;

	printf("FoxOS > ");

	char* buffer = (char*) calloc(2048, sizeof(char));
	int buffer_len = 0;

	while (true) {
		char input[1];
		errno = 0;
		read(STDIN, input, 1);

		if (errno == 0xded) {
			printf("Other procces is already reading from stdin!");
			abort();
		}

		if (input[0] == '\n') {
			if (buffer_len == 0) {
				printf("FoxOS > ");
			} else if (is_quote_open(buffer)) {
				printf(" quote> ");
			} else {
				command_received(buffer); //This should block while command is running.

				memset(buffer, 0, sizeof(char) * 2048);
				buffer_len = 0;
				printf("\nFoxOS > ");
			}
		} else if (input[0] == '\b') {
			buffer[buffer_len] = 0;
			buffer_len--;
			if (buffer_len == -1) {
				printf(" ");
				buffer_len = 0;
			}
		} else {
			buffer[buffer_len] = input[0];
			buffer_len++;
		}
	}

	printf("\n[TERMINAL PROCESS ENDED]\n");
	free(buffer);
	return 0;
}
