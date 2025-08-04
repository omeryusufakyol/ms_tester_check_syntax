/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gc.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oakyol <oakyol@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 15:36:23 by oakyol            #+#    #+#             */
/*   Updated: 2025/08/01 22:55:21 by oakyol           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../libft/libft.h"

static size_t	ft_strnlen(const char *s, size_t maxlen)
{
	size_t	i = 0;
	while (i < maxlen && s[i])
		i++;
	return (i);
}

void	gc_init(t_ms *ms)
{
	ms->gc_list = NULL;
}

// GC MALLOC
void	*gc_malloc(t_ms *ms, size_t size)
{
	void		*ptr;
	t_gc_node	*node;

	ptr = ft_calloc(1, size);
	if (!ptr)
		return (NULL);
	node = malloc(sizeof(t_gc_node));
	if (!node)
	{
		free(ptr);
		return (NULL);
	}
	node->ptr = ptr;
	node->next = ms->gc_list;
	ms->gc_list = node;
	return (ptr);
}

// GC FREE ALL
void	gc_free_all(t_ms *ms)
{
	t_gc_node	*tmp;

	while (ms->gc_list)
	{
		tmp = ms->gc_list;
		ms->gc_list = ms->gc_list->next;
		free(tmp->ptr);
		free(tmp);
	}
	ms->gc_list = NULL;
}

// GC STRDUP
char	*gc_strdup(t_ms *ms, const char *s)
{
	size_t	len;
	char	*copy;

	if (!s)
		return (NULL);
	len = ft_strlen(s) + 1;
	copy = gc_malloc(ms, len);
	if (!copy)
		return (NULL);
	ft_memcpy(copy, s, len);
	return (copy);
}

// GC STRNDUP
char	*gc_strndup(t_ms *ms, const char *s, size_t n)
{
	size_t	len;
	char	*copy;

	len = ft_strnlen(s, n);
	copy = gc_malloc(ms, len + 1);
	if (!copy)
		return (NULL);
	ft_memcpy(copy, s, len);
	copy[len] = '\0';
	return (copy);
}

// GC STRJOIN
char	*gc_strjoin(t_ms *ms, const char *s1, const char *s2)
{
	size_t	len1 = ft_strlen(s1);
	size_t	len2 = ft_strlen(s2);
	char	*joined = gc_malloc(ms, len1 + len2 + 1);

	if (!joined)
		return (NULL);
	ft_memcpy(joined, s1, len1);
	ft_memcpy(joined + len1, s2, len2);
	joined[len1 + len2] = '\0';
	return (joined);
}

static int	ptr_in_gc(void *ptr, t_ms *ms)
{
	t_gc_node *node = ms->gc_list;

	while (node)
	{
		if (node->ptr == ptr)
			return (1); // Evet, GC listesinde
		node = node->next;
	}
	return (0); // Hayır, GC listesinde değil
}

static void	safe_free(t_ms *ms, void *ptr)
{
	if (!ptr)
		return;
	if (!ptr_in_gc(ptr, ms))
		free(ptr);
}

// GC STRJOIN + FREE S1
char	*gc_strjoin_free(t_ms *ms, char *s1, char *s2)
{
	char	*joined = gc_strjoin(ms, s1, s2);
	safe_free(ms, s1);
	return (joined);
}

// GC STRJOIN + FREE BOTH
char	*gc_strjoin_free_both(t_ms *ms, char *s1, char *s2)
{
	char	*joined = gc_strjoin(ms, s1, s2);
	safe_free(ms, s1);
	safe_free(ms, s2);
	return (joined);
}

// GC SPLIT
char	**gc_split(t_ms *ms, const char *s, char c)
{
	size_t	i = 0, j = 0, start = 0, count = 0;
	char	**result;

	if (!s)
		return (NULL);
	while (s[i])
	{
		while (s[i] && s[i] == c)
			i++;
		if (s[i])
		{
			count++;
			while (s[i] && s[i] != c)
				i++;
		}
	}
	result = gc_malloc(ms, sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	while (s[i] && j < count)
	{
		while (s[i] && s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		result[j++] = gc_strndup(ms, &s[start], i - start);
	}
	result[j] = NULL;
	return (result);
}

char	*gc_strtrim(t_ms *ms, const char *s1, const char *set)
{
	size_t	start;
	size_t	end;
	char	*trimmed;

	if (!s1 || !set)
		return (NULL);

	start = 0;
	while (s1[start] && ft_strchr(set, s1[start]))
		start++;

	end = ft_strlen(s1);
	while (end > start && ft_strchr(set, s1[end - 1]))
		end--;

	trimmed = gc_malloc(ms, end - start + 1);
	if (!trimmed)
		return (NULL);

	ft_memcpy(trimmed, s1 + start, end - start);
	trimmed[end - start] = '\0';
	return (trimmed);
}

char	*gc_itoa(t_ms *ms, int n)
{
	char	buffer[12]; // int için yeterli
	int		i = 10;
	int		neg = (n < 0);
	unsigned int num = neg ? -n : n;

	buffer[11] = '\0';
	if (n == 0)
		return (gc_strdup(ms, "0"));
	while (num > 0 && i >= 0)
	{
		buffer[i--] = (num % 10) + '0';
		num /= 10;
	}
	if (neg)
		buffer[i--] = '-';
	return (gc_strdup(ms, &buffer[i + 1]));
}