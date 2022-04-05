#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int	flagpipe;
int	back_stdin;
int	ret;

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

void	ftal(void)
{
	ft_putstr_fd("error: fatal\n", 2);
	exit(1);
}

void	fterror(char *str)
{
	ft_putstr_fd("error: ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
}

void	ft_exec_error(char *str)
{
	ft_putstr_fd("error: cannot execute ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
	exit(1);
}

void	opening(int fd[2])
{
	if (flagpipe)
	{
		if (close(fd[0]) == -1)
			ftal();
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			ftal();
		if (close(fd[1]) == -1)
			ftal();
	}
}

void	closing(int fd[2])
{
	if (flagpipe)
	{
		if (dup2(fd[0], STDIN_FILENO) == -1)
			ftal();
		if (close(fd[0]) == -1)
			ftal();
		if (close(fd[1]) == -1)
			ftal();
	}
}

void	ft_cd(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
		i++;
	if (i != 2)
	{
		fterror("cd: bad arguments");
		ret = 1;
		return ;
	}
	if (chdir(argv[1]) == -1)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd("\n", 2);
		ret = 1;
		return ;
	}
}

void	ft_execute(char **argv, char **envp)
{
	pid_t	pid;
	int		fd[2];

	if (!strcmp(argv[0], "cd"))
		return (ft_cd(argv));
	if (flagpipe)
		if (pipe(fd) == -1)
			ftal();
	pid = fork();
	if (pid == 0)
	{
		opening(fd);
		if (execve(argv[0], argv, envp) == -1)
			ft_exec_error(argv[0]);
	}
	closing(fd);
}

void	ft_command(char **argv, char **envp)
{
	int	i;
	int	begin;
	int	nproc;

	i = -1;
	begin = 0;
	nproc = 0;
	while (argv[++i])
	{
		if (!strcmp(argv[i], "|") || !argv[i + 1])
		{
			flagpipe = 0;
			if (!strcmp(argv[i], "|"))
			{
				flagpipe = 1;
				argv[i] = NULL;
			}
			ft_execute(argv + begin, envp);
			begin = i + 1;
			nproc++;
		}
	}
	while (nproc-- > 0)
		waitpid(-1, 0, 0);
}

void	ft_restorefd(void)
{
	int	tmp;

	tmp = dup(STDIN_FILENO);
	if (dup2(back_stdin, STDIN_FILENO) == -1)
		ftal();
	if (close(tmp) == -1)
		ftal();
}

int	main(int argc, char **argv, char **envp)
{
	int	i;
	int	begin;

	i = 0;
	begin = 1;
	back_stdin = dup(STDIN_FILENO);
	(void)argc;
	while (argv[++i])
	{
		if (!strcmp(argv[i], ";") || !argv[i + 1])
		{
			if (!strcmp(argv[i], ";"))
					argv[i] = NULL;
			ft_command(argv + begin, envp);
			begin = i + 1;
		}
		ret = 1;
		ft_restorefd();
	}
	return (ret);
}
