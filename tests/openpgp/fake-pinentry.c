/* Fake pinentry program for the OpenPGP test suite.
 *
 * Copyright (C) 2016 g10 code GmbH
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

FILE *log_stream;

int
reply (const char *fmt, ...)
{
  int result;
  va_list ap;

  if (log_stream)
    {
      fprintf (log_stream, "> ");
      va_start (ap, fmt);
      vfprintf (log_stream, fmt, ap);
      va_end (ap);
    }
  va_start (ap, fmt);
  result = vprintf (fmt, ap);
  va_end (ap);

  return result;
}

/* Return the first line from FNAME, removing it from the file.  */
char *
get_passphrase (const char *fname)
{
  char *passphrase = NULL;
  size_t fname_len;
  char *fname_new;
  FILE *source, *sink;
  char linebuf[80];

  fname_len = strlen (fname);
  fname_new = malloc (fname_len + 5);
  if (fname_new == NULL)
    {
      perror ("malloc");
      exit (1);
    }
  snprintf (fname_new, fname_len + 5, "%s.new", fname);

  source = fopen (fname, "r");
  if (! source)
    {
      perror (fname);
      exit (1);
    }

  sink = fopen (fname_new, "w");
  if (! sink)
    {
      perror (fname_new);
      exit (1);
    }

  while (fgets (linebuf, sizeof linebuf, source))
    {
      linebuf[sizeof linebuf - 1] = 0;
      if (passphrase == NULL)
        {
          passphrase = strdup (linebuf);
          if (passphrase == NULL)
            {
              perror ("strdup");
              exit (1);
            }
        }
      else
        fputs (linebuf, sink);
    }

  if (ferror (source))
    {
      perror (fname);
      exit (1);
    }

  if (ferror (sink))
    {
      perror (fname_new);
      exit (1);
    }

  fclose (source);
  fclose (sink);
  rename (fname_new, fname);
  return passphrase;
}


#define spacep(p)   (*(p) == ' ' || *(p) == '\t')

/* Skip over options in LINE.

   Blanks after the options are also removed.  Options are indicated
   by two leading dashes followed by a string consisting of non-space
   characters.  The special option "--" indicates an explicit end of
   options; all what follows will not be considered an option.  The
   first no-option string also indicates the end of option parsing. */
char *
skip_options (const char *line)
{
  while (spacep (line))
    line++;
  while (*line == '-' && line[1] == '-')
    {
      while (*line && !spacep (line))
        line++;
      while (spacep (line))
        line++;
    }
  return (char*) line;
}


/* Return a pointer to the argument of the option with NAME.  If such
   an option is not given, NULL is returned. */
char *
option_value (const char *line, const char *name)
{
  char *s;
  int n = strlen (name);

  s = strstr (line, name);
  if (s && s >= skip_options (line))
    return NULL;
  if (s && (s == line || spacep (s-1))
      && s[n] && (spacep (s+n) || s[n] == '='))
    {
      s += n + 1;
      s += strspn (s, " ");
      if (*s && !spacep(s))
        return s;
    }
  return NULL;
}

int
main (int argc, char **argv)
{
  char *args;
  char *logfile;
  char *passphrasefile;
  char *passphrase;

  /* We get our options via PINENTRY_USER_DATA.  */
  (void) argc, (void) argv;

  setvbuf (stdin, NULL, _IOLBF, BUFSIZ);
  setvbuf (stdout, NULL, _IOLBF, BUFSIZ);

  args = getenv ("PINENTRY_USER_DATA");
  if (! args)
    args = "";

  logfile = option_value (args, "--logfile");
  if (logfile)
    {
      char *p = logfile, more;
      while (*p && ! spacep (p))
        p++;
      more = !! *p;
      *p = 0;
      args = more ? p+1 : p;

      log_stream = fopen (logfile, "a");
      if (! log_stream)
        {
          perror (logfile);
          return 1;
        }
    }

  passphrasefile = option_value (args, "--passphrasefile");
  if (passphrasefile)
    {
      char *p = passphrasefile, more;
      while (*p && ! spacep (p))
        p++;
      more = !! *p;
      *p = 0;
      args = more ? p+1 : p;

      passphrase = get_passphrase (passphrasefile);
      if (! passphrase)
        {
          reply ("# Passphrasefile '%s' is empty.  Terminating.\n",
                 passphrasefile);
          return 1;
        }

      p = passphrase + strlen (passphrase) - 1;
      if (*p == '\n')
        *p = 0;
    }
  else
    {
      passphrase = skip_options (args);
      if (*passphrase == 0)
        passphrase = "no PINENTRY_USER_DATA -- using default passphrase";
    }

  reply ("# fake-pinentry started.  Passphrase='%s'.\n", passphrase);
  reply ("OK - what's up?\n");

  while (! feof (stdin))
    {
      char buffer[1024];

      if (fgets (buffer, sizeof buffer, stdin) == NULL)
	break;

      if (log_stream)
        fprintf (log_stream, "< %s", buffer);

      if (strncmp (buffer, "GETPIN", 6) == 0)
        reply ("D %s\n", passphrase);
      else if (strncmp (buffer, "BYE", 3) == 0)
	{
	  reply ("OK\n");
	  break;
	}

      reply ("OK\n");
    }

  reply ("# Connection terminated.\n");
  if (log_stream)
    fclose (log_stream);

  return 0;
}
