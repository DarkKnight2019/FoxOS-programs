#include <commands.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/spawn.h>
#include <sys/env.h>
#include <sys/open.h>
#include <sys/close.h>
#include <errno.h>

#include <keyboard_helper.h>
#include <argv_tools.h>


char* search_executable(char* command) {
	char* path = getenv("PATH");

	if (path == NULL) {
		abort();
	}

	char* path_copy = malloc(strlen(path) + 1);
	memset(path_copy, 0, strlen(path) + 1);
	strcpy(path_copy, path);
	char* path_token = strtok(path_copy, ";");

	while (path_token != NULL) {
		char* executable = malloc(strlen(path_token) + strlen(command) + 2);
		memset(executable, 0, strlen(path_token) + strlen(command) + 2);
		strcpy(executable, path_token);
		strcat(executable, "/");
		strcat(executable, command);


		int fd;
		if ((fd = open(executable)) != -1) {
			close(fd);
			free(path_copy);
			return executable;
		}

		free(executable);

		char* executable2 = malloc(strlen(path_token) + strlen(command) + strlen(".elf") + 2);
		memset(executable2, 0, strlen(path_token) + strlen(command) + strlen(".elf") + 2);
		strcpy(executable2, path_token);
		strcat(executable2, "/");
		strcat(executable2, command);
		strcat(executable2, ".elf");

		if ((fd = open(executable2)) != -1) {
			close(fd);
			free(path_copy);
			return executable2;
		}

		free(executable2);
		path_token = strtok(NULL, ";");
	}

	free(path_copy);

	char* command_copy = malloc(strlen(command) + 1);
	memset(command_copy, 0, strlen(command) + 1);
	strcpy(command_copy, command);
	return command_copy;
}

void load_keymap(char* command) {
	if (command[11] == 0) {
		printf("No keymap specified!");
		return;
	} else {
		char* keymap_name = &command[11];
		printf("Loading keymap %s!\n", keymap_name);

		if (strcmp(keymap_name, (char*)"de") == 0) {
			set_keymap(0);
		} else if (strcmp(keymap_name, (char*)"us") == 0) {
			set_keymap(1);
		} else if (strcmp(keymap_name, (char*)"fr") == 0) {
			set_keymap(2);
		} else {
			printf("Keymap %s not found!\n", keymap_name);
		}
	}
}

void keydbg(bool onoff) {
	if (onoff) {
		set_keyboard_debug(true);
		printf("Keyboard debugging enabled!");
	} else {
		set_keyboard_debug(false);
		printf("Keyboard debugging disabled!");
	}
}

void cd(char* command) {
	char* path = &command[2];
	if (path[0] == 0) {
		printf("No path specified!");
		return;
	}

	path++;
	// printf("Changing directory to %s!\n", path);

	char path_buf[256];
	memset(path_buf, 0, 256);
	resolve(path, path_buf);

	env_set(ENV_SET_CWD, path_buf);
}

void spawn_process(char* command, char** terminal_envp) {
	const char** argv = (const char**) argv_split(command);
	char* executable = search_executable((char*) argv[0]);
	const char** envp = (const char**) terminal_envp; //Maybe use actual enviromental vars?

	errno = 0;
	task* t = spawn(executable, argv, envp, true);

	if (t == NULL) {
		goto error;
	}

	bool task_exit = false;
	t->on_exit = &task_exit;

	while (!task_exit) {
		__asm__ __volatile__("pause" :: : "memory");
	}

	goto _exit;

error:
	printf("Error: command not found: %s", command);

_exit:
	free(executable);
	free(argv);
	return;
}