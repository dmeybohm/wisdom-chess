typedef void (*CommandFunc) (void);

#define COMMMAND_HANDLER(name) \
	void command_##name (void)

struct cmd
{
	char       *str;
	CommandFunc func;
};

COMMAND_HANDLER (moves);
COMMAND_HANDLER (edit);

struct cmd commands[] =
{
	{ "moves", command_moves, },
	{ "edit",  command_edit,  },
	{ NULL,    NULL,          },
};

int read_command (struct board *board, color_t *side, move_tree_t *history)
{
	char buf[128];
	struct cmd *ptr;

	memset (buf, 0, sizeof (buf));
	if (fgets (buf, sizeof (buf) - 1, stdin) == NULL)
		return 0;

	for (ptr = commands; ptr->str != NULL; ptr++)
	{
		if (!strncmp (ptr->str, buf, strlen (ptr->str)))
			(*ptr->func) (board, side, history);
	}

	return 1;
}

/* vi: set ts=4 sw=4: */
