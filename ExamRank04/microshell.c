#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int	print(char *message)
{
	while (*message)
		write(STDERR_FILENO, message++, 1);
	return (1);
}

int	change_dir(char **cmd, int cmd_len)
{
	if (cmd_len != 2)
		return (print("error: cd: bad arguments\n"));	
	if (chdir(cmd[1]) == -1)
		return (print("error: cd: cannot change directory to "), print(cmd[1]), print("\n"));
	return (0);
}

int	execution(char **cmd, int pipe_cmd, int cmd_len, char **envp)
{
	int	fd[2];
	int status;
	int	pid;

	status = 0;
	if (!pipe_cmd && !strcmp(*cmd, "cd"))
		return (change_dir(cmd, cmd_len));
	if (pipe_cmd && pipe(fd) == -1)
		return (print("error: fatal\n"));
	pid = fork();
	if (!pid)
	{
		cmd[cmd_len] = 0;
		if (pipe_cmd && (dup2(fd[1], STDIN_FILENO) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return (print("error: fatal\n"));
		if (!strcmp(*cmd, "cd"))
			return (change_dir(cmd, cmd_len));
		execve(*cmd, cmd, envp);
		return (print("error: cannot execute "), print(*cmd), print("\n"));
	}
	waitpid(pid, &status, 0);
	if (pipe_cmd && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return (print("error: fatal\n"));
	return (WIFEXITED(status) && WEXITSTATUS(status));
}

int	is_pipe(char *str)
{
	if (!str)
		return (0);
	if (!strcmp(str, "|"))
		return (1);
	return (0);
}

int	main(int argc, char *argv[], char **envp)
{
	int	i;
	int cmd_len;
	int	status;

	status = 0;
	i = 0;
	cmd_len = 1;
	if (argc > 1)
	{
		argv += 1;
		while (argv[i])
		{
			while (argv[cmd_len] && strcmp(argv[cmd_len], "|") && strcmp(argv[cmd_len], ";"))
				cmd_len++;
			status = execution(argv + i, is_pipe(argv[cmd_len]), cmd_len, envp);
			i = cmd_len;
			if (argv[i])
				i++;
		}
	}
	return (status);
}
