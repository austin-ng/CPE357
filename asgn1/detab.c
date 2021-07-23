/* Austin Ng
 * CPE 357-03
 * Assignment 1: A program that replaces tabs in its input with the proper
 	number of blanks to space to the next tab stop (every 8 spaces). 

	The program reads each character of the input file until its end,
	replacing every tab with the approriate number of spaces depending
	on the position of the character. I used a switch to deal with the
	8 possible tab positions. */

#include <stdio.h>
int main(int argc, char *argv[]) {

	int ch; /* reads the current character in the file */
	int count = 0; /* counter for the current position */
	int count2; /* used for computing the amount of spaces */
	int charcount = 0; /* counter for the actual input of characters  */
	ch = getchar();
	while (ch != EOF)
	{
		if (ch == '\t')
		{
			count2 = count % 8;
			switch(count2)
			{
				case 0:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 1:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 2:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 3:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 4:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 5:
				putchar(' ');
				putchar(' ');
				putchar(' ');
				break;

				case 6:
				putchar(' ');
				putchar(' ');
				break;

				case 7:
				putchar(' ');
				break;

				default:
				break;
			}
			count = count + (8 - (count % 8));
			charcount = charcount + 1;
		}
		else if (ch == '\n')
		{
			putchar(ch);
			count = 0;
			charcount = charcount + 1;
		}

		else if (ch == '\r')
		{
			putchar(ch);
			count = 0;
			charcount = charcount + 1;
		}

		else if (ch == '\b')
		{
			putchar(ch);
			count = count - 1;
			charcount = charcount - 1;
			if (charcount < 0)
			{
				charcount = 0;
				count = 0;
			}
			if (count < 0)
			{
				count = 0;
			}
		}
		else
		{
			putchar(ch);
			count = count + 1;
			charcount = charcount + 1;
		}
		ch = getchar();
	}
	return 0;
}
