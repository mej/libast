/*
 * Copyright (C) 1997-2004, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file conf.c
 * Config File Parser Source File
 *
 * This file contains the functions which comprise the config file
 * parser.
 *
 * @author Michael Jennings <mej@eterm.org>
 */

static const char cvs_ident[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libast_internal.h>

static spifconf_var_t *spifconf_new_var(void);
static void spifconf_free_var(spifconf_var_t *);
static char *spifconf_get_var(const char *);
static void spifconf_put_var(char *, char *);
static char *builtin_random(char *);
static char *builtin_exec(char *);
static char *builtin_get(char *);
static char *builtin_put(char *);
static char *builtin_dirscan(char *);
static char *builtin_version(char *);
static char *builtin_appname(char *);
static void *parse_null(char *, void *);

static ctx_t *context;
static ctx_state_t *ctx_state;
static spifconf_func_t *builtins;
static unsigned char ctx_cnt, ctx_idx, ctx_state_idx, ctx_state_cnt, fstate_cnt, builtin_cnt, builtin_idx;
static spifconf_var_t *spifconf_vars = NULL;

const char *true_vals[] = { "1", "on", "true", "yes" };
const char *false_vals[] = { "0", "off", "false", "no" };

fstate_t *fstate;
unsigned char fstate_idx;

/***** The Config File Section *****/
/* This function must be called before any other spifconf_*() function.
   Otherwise you will be bitten by dragons.  That's life. */
void
spifconf_init_subsystem(void)
{

    /* Initialize the context list and establish a catch-all "null" context */
    ctx_cnt = 20;
    ctx_idx = 0;
    context = (ctx_t *) MALLOC(sizeof(ctx_t) * ctx_cnt);
    MEMSET(context, 0, sizeof(ctx_t) * ctx_cnt);
    context[0].name = STRDUP("null");
    context[0].handler = parse_null;

    /* Initialize the context state stack and set the current context to "null" */
    ctx_state_cnt = 20;
    ctx_state_idx = 0;
    ctx_state = (ctx_state_t *) MALLOC(sizeof(ctx_state_t) * ctx_state_cnt);
    MEMSET(ctx_state, 0, sizeof(ctx_state_t) * ctx_state_cnt);

    /* Initialize the file state stack */
    fstate_cnt = 10;
    fstate_idx = 0;
    fstate = (fstate_t *) MALLOC(sizeof(fstate_t) * fstate_cnt);
    MEMSET(fstate, 0, sizeof(fstate_t) * fstate_cnt);

    /* Initialize the builtin function table */
    builtin_cnt = 10;
    builtin_idx = 0;
    builtins = (spifconf_func_t *) MALLOC(sizeof(spifconf_func_t) * builtin_cnt);
    MEMSET(builtins, 0, sizeof(spifconf_func_t) * builtin_cnt);

    /* Register the omni-present builtin functions */
    spifconf_register_builtin("appname", builtin_appname);
    spifconf_register_builtin("version", builtin_version);
    spifconf_register_builtin("exec", builtin_exec);
    spifconf_register_builtin("random", builtin_random);
    spifconf_register_builtin("get", builtin_get);
    spifconf_register_builtin("put", builtin_put);
    spifconf_register_builtin("dirscan", builtin_dirscan);
}

/* Register a new config file context */
unsigned char
spifconf_register_context(char *name, ctx_handler_t handler)
{
    if (strcasecmp(name, "null")) {
        if (++ctx_idx == ctx_cnt) {
            ctx_cnt *= 2;
            context = (ctx_t *) REALLOC(context, sizeof(ctx_t) * ctx_cnt);
        }
    } else {
        FREE(context[0].name);
    }
    context[ctx_idx].name = STRDUP(name);
    context[ctx_idx].handler = handler;
    D_CONF(("Added context \"%s\" with ID %d and handler 0x%08x\n", context[ctx_idx].name, ctx_idx, context[ctx_idx].handler));
    return (ctx_idx);
}

/* Register a new file state structure */
unsigned char
spifconf_register_fstate(FILE * fp, char *path, char *outfile, unsigned long line, unsigned char flags)
{

    if (++fstate_idx == fstate_cnt) {
        fstate_cnt *= 2;
        fstate = (fstate_t *) REALLOC(fstate, sizeof(fstate_t) * fstate_cnt);
    }
    fstate[fstate_idx].fp = fp;
    fstate[fstate_idx].path = path;
    fstate[fstate_idx].outfile = outfile;
    fstate[fstate_idx].line = line;
    fstate[fstate_idx].flags = flags;
    return (fstate_idx);
}

/* Register a new builtin function */
unsigned char
spifconf_register_builtin(char *name, spifconf_func_ptr_t ptr)
{

    builtins[builtin_idx].name = STRDUP(name);
    builtins[builtin_idx].ptr = ptr;
    if (++builtin_idx == builtin_cnt) {
        builtin_cnt *= 2;
        builtins = (spifconf_func_t *) REALLOC(builtins, sizeof(spifconf_func_t) * builtin_cnt);
    }
    return (builtin_idx - 1);
}

/* Register a new config file context */
unsigned char
spifconf_register_context_state(unsigned char ctx_id)
{

    if (++ctx_state_idx == ctx_state_cnt) {
        ctx_state_cnt *= 2;
        ctx_state = (ctx_state_t *) REALLOC(ctx_state, sizeof(ctx_state_t) * ctx_state_cnt);
    }
    ctx_state[ctx_state_idx].ctx_id = ctx_id;
    ctx_state[ctx_state_idx].state = NULL;
    return (ctx_state_idx);
}

void
spifconf_free_subsystem(void)
{
    spifconf_var_t *v, *tmp;
    unsigned long i;

    for (v = spifconf_vars; v;) {
        tmp = v;
        v = v->next;
        spifconf_free_var(tmp);
    }
    for (i = 0; i < builtin_idx; i++) {
        FREE(builtins[i].name);
    }
    for (i = 0; i <= ctx_idx; i++) {
        FREE(context[i].name);
    }
    FREE(ctx_state);
    FREE(builtins);
    FREE(fstate);
    FREE(context);
}

static spifconf_var_t *
spifconf_new_var(void)
{
    spifconf_var_t *v;

    v = (spifconf_var_t *) MALLOC(sizeof(spifconf_var_t));
    MEMSET(v, 0, sizeof(spifconf_var_t));
    return v;
}

static void
spifconf_free_var(spifconf_var_t *v)
{
    if (v->var) {
        FREE(v->var);
    }
    if (v->value) {
        FREE(v->value);
    }
    FREE(v);
}

static char *
spifconf_get_var(const char *var)
{
    spifconf_var_t *v;

    D_CONF(("var == \"%s\"\n", var));
    for (v = spifconf_vars; v; v = v->next) {
        if (!strcmp(v->var, var)) {
            D_CONF(("Found it at %10p:  \"%s\" == \"%s\"\n", v, v->var, v->value));
            return (v->value);
        }
    }
    D_CONF(("Not found.\n"));
    return NULL;
}

static void
spifconf_put_var(char *var, char *val)
{
    spifconf_var_t *v, *loc = NULL, *tmp;

    ASSERT(var != NULL);
    D_CONF(("var == \"%s\", val == \"%s\"\n", var, val));

    for (v = spifconf_vars; v; loc = v, v = v->next) {
        int n;

        n = strcmp(var, v->var);
        D_CONF(("Comparing at %10p:  \"%s\" -> \"%s\", n == %d\n", v, v->var, v->value, n));
        if (n == 0) {
            FREE(v->value);
            if (val) {
                v->value = val;
                D_CONF(("Variable already defined.  Replacing its value with \"%s\"\n", v->value));
            } else {
                D_CONF(("Variable already defined.  Deleting it.\n"));
                if (loc) {
                    loc->next = v->next;
                } else {
                    spifconf_vars = v->next;
                }
                spifconf_free_var(v);
            }
            return;
        } else if (n < 0) {
            break;
        }
    }
    if (!val) {
        D_CONF(("Empty value given for non-existant variable \"%s\".  Aborting.\n", var));
        return;
    }
    D_CONF(("Inserting new var/val pair between \"%s\" and \"%s\"\n", ((loc) ? loc->var : "-beginning-"), ((v) ? v->var : "-end-")));
    tmp = spifconf_new_var();
    if (loc == NULL) {
        tmp->next = spifconf_vars;
        spifconf_vars = tmp;
    } else {
        tmp->next = loc->next;
        loc->next = tmp;
    }
    tmp->var = var;
    tmp->value = val;
}

static char *
builtin_random(char *param)
{
    unsigned long n, index;
    static unsigned int rseed = 0;

    D_PARSE(("builtin_random(%s) called\n", NONULL(param)));

    if (rseed == 0) {
        rseed = (unsigned int) (getpid() * time(NULL) % ((unsigned int) -1));
        srand(rseed);
    }
    n = spiftool_num_words(param);
    index = (int) (n * ((float) rand()) / (RAND_MAX + 1.0)) + 1;
    D_PARSE(("random index == %lu\n", index));

    return (spiftool_get_word(index, param));
}

static char *
builtin_exec(char *param)
{
    unsigned long fsize;
    char *Command, *Output = NULL;
    char OutFile[256];
    FILE *fp;
    int fd;

    D_PARSE(("builtin_exec(%s) called\n", NONULL(param)));

    Command = (char *) MALLOC(CONFIG_BUFF);
    strcpy(OutFile, "Eterm-exec-");
    fd = spiftool_temp_file(OutFile, sizeof(OutFile));
    if ((fd < 0) || fchmod(fd, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        libast_print_error("Unable to create unique temporary file for \"%s\" -- %s\n", param, strerror(errno));
        return ((char *) NULL);
    }
    if (strlen(param) + strlen(OutFile) + 8 > CONFIG_BUFF) {
        libast_print_error("Parse error in file %s, line %lu:  Cannot execute command, line too long\n", file_peek_path(), file_peek_line());
        return ((char *) NULL);
    }
    strcpy(Command, param);
    strcat(Command, " >");
    strcat(Command, OutFile);
    system(Command);
    if ((fp = fdopen(fd, "rb")) != NULL) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        if (fsize) {
            Output = (char *) MALLOC(fsize + 1);
            fread(Output, fsize, 1, fp);
            Output[fsize] = 0;
            fclose(fp);
            remove(OutFile);
            Output = spiftool_condense_whitespace(Output);
        } else {
            libast_print_warning("Command at line %lu of file %s returned no output.\n", file_peek_line(), file_peek_path());
        }
    } else {
        libast_print_warning("Output file %s could not be created.  (line %lu of file %s)\n", NONULL(OutFile), file_peek_line(), file_peek_path());
    }
    FREE(Command);

    return (Output);
}

static char *
builtin_get(char *param)
{
    char *s, *f, *v;
    unsigned short n;

    if (!param || ((n = spiftool_num_words(param)) > 2)) {
        libast_print_error("Parse error in file %s, line %lu:  Invalid syntax for %get().  Syntax is:  %get(variable)\n", file_peek_path(),
                    file_peek_line());
        return NULL;
    }

    D_PARSE(("builtin_get(%s) called\n", param));
    s = spiftool_get_word(1, param);
    if (n == 2) {
        f = spiftool_get_word(2, param);
    } else {
        f = NULL;
    }
    v = spifconf_get_var(s);
    FREE(s);
    if (v) {
        if (f) {
            FREE(f);
        }
        return (STRDUP(v));
    } else if (f) {
        return f;
    } else {
        return NULL;
    }
}

static char *
builtin_put(char *param)
{
    char *var, *val;

    if (!param || (spiftool_num_words(param) != 2)) {
        libast_print_error("Parse error in file %s, line %lu:  Invalid syntax for %put().  Syntax is:  %put(variable value)\n", file_peek_path(),
                    file_peek_line());
        return NULL;
    }

    D_PARSE(("builtin_put(%s) called\n", param));
    var = spiftool_get_word(1, param);
    val = spiftool_get_word(2, param);
    spifconf_put_var(var, val);
    return NULL;
}

static char *
builtin_dirscan(char *param)
{
    int i;
    unsigned long n;
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    char *dir, *buff;

    if (!param || (spiftool_num_words(param) != 1)) {
        libast_print_error("Parse error in file %s, line %lu:  Invalid syntax for %dirscan().  Syntax is:  %dirscan(directory)\n",
                    file_peek_path(), file_peek_line());
        return NULL;
    }
    D_PARSE(("builtin_dirscan(%s)\n", param));
    dir = spiftool_get_word(1, param);
    dirp = opendir(dir);
    if (!dirp) {
        return NULL;
    }
    buff = (char *) MALLOC(CONFIG_BUFF);
    *buff = 0;
    n = CONFIG_BUFF;

    for (i = 0; (dp = readdir(dirp)) != NULL;) {
        char fullname[PATH_MAX];

        snprintf(fullname, sizeof(fullname), "%s/%s", dir, dp->d_name);
        if (stat(fullname, &filestat)) {
            D_PARSE((" -> Couldn't stat() file %s -- %s\n", fullname, strerror(errno)));
        } else {
            if (S_ISREG(filestat.st_mode)) {
                unsigned long len;

                len = strlen(dp->d_name);
                if (len < n) {
                    strcat(buff, dp->d_name);
                    strcat(buff, " ");
                    n -= len + 1;
                }
            }
        }
        if (n < 2) {
            break;
        }
    }
    closedir(dirp);
    return buff;
}

static char *
builtin_version(char *param)
{
    USE_VAR(param);
    D_PARSE(("builtin_version(%s) called\n", NONULL(param)));

    return (STRDUP(libast_program_version));
}

static char *
builtin_appname(char *param)
{
    char buff[30];

    USE_VAR(param);
    D_PARSE(("builtin_appname(%s) called\n", NONULL(param)));

    snprintf(buff, sizeof(buff), "%s-%s", libast_program_name, libast_program_version);
    return (STRDUP(buff));
}

/* spifconf_shell_expand() takes care of shell variable expansion, quote conventions,
   calling of built-in functions, etc.                                -- mej */
char *
spifconf_shell_expand(char *s)
{
    register char *tmp;
    register char *pbuff = s, *tmp1;
    register unsigned long j, k, l = 0;
    char newbuff[CONFIG_BUFF];
    unsigned char in_single = 0, in_double = 0;
    unsigned long cnt1 = 0, cnt2 = 0;
    const unsigned long max = CONFIG_BUFF - 1;
    char *Command, *Output, *EnvVar;

    ASSERT_RVAL(s != NULL, (char *) NULL);

#if 0
    newbuff = (char *) MALLOC(CONFIG_BUFF);
#endif

    for (j = 0; *pbuff && j < max; pbuff++, j++) {
        switch (*pbuff) {
          case '~':
              D_CONF(("Tilde detected.\n"));
              if (!in_single && !in_double) {
                  strncpy(newbuff + j, getenv("HOME"), max - j);
                  cnt1 = strlen(getenv("HOME")) - 1;
                  cnt2 = max - j - 1;
                  j += MIN(cnt1, cnt2);
              } else {
                  newbuff[j] = *pbuff;
              }
              break;
          case '\\':
              D_CONF(("Escape sequence detected.\n"));
              if (!in_single || (in_single && *(pbuff + 1) == '\'')) {
                  switch (tolower(*(++pbuff))) {
                    case 'n':
                        newbuff[j] = '\n';
                        break;
                    case 'r':
                        newbuff[j] = '\r';
                        break;
                    case 't':
                        newbuff[j] = '\t';
                        break;
                    case 'b':
                        newbuff[j] = '\b';
                        break;
                    case 'f':
                        newbuff[j] = '\f';
                        break;
                    case 'a':
                        newbuff[j] = '\a';
                        break;
                    case 'v':
                        newbuff[j] = '\v';
                        break;
                    case 'e':
                        newbuff[j] = '\033';
                        break;
                    default:
                        newbuff[j] = *pbuff;
                        break;
                  }
              } else {
                  newbuff[j++] = *(pbuff++);
                  newbuff[j] = *pbuff;
              }
              break;
          case '%':
              D_CONF(("%% detected.\n"));
              for (k = 0, pbuff++; builtins[k].name != NULL; k++) {
                  D_PARSE(("Checking for function %%%s, pbuff == \"%s\"\n", builtins[k].name, pbuff));
                  l = strlen(builtins[k].name);
                  if (!strncasecmp(builtins[k].name, pbuff, l) && ((pbuff[l] == '(') || (pbuff[l] == ' ' && pbuff[l + 1] == ')'))) {
                      break;
                  }
              }
              if (builtins[k].name == NULL) {
                  newbuff[j] = *pbuff;
              } else {
                  D_CONF(("Call to built-in function %s detected.\n", builtins[k].name));
                  Command = (char *) MALLOC(CONFIG_BUFF);
                  pbuff += l;
                  if (*pbuff != '(')
                      pbuff++;
                  for (tmp1 = Command, pbuff++, l = 1; l && *pbuff; pbuff++, tmp1++) {
                      switch (*pbuff) {
                        case '(':
                            l++;
                            *tmp1 = *pbuff;
                            break;
                        case ')':
                            l--;
                        default:
                            *tmp1 = *pbuff;
                            break;
                      }
                  }
                  *(--tmp1) = 0;
                  if (l) {
                      libast_print_error("parse error in file %s, line %lu:  Mismatched parentheses\n", file_peek_path(), file_peek_line());
                      return ((char *) NULL);
                  }
                  Command = spifconf_shell_expand(Command);
                  Output = (builtins[k].ptr) (Command);
                  FREE(Command);
                  if (Output) {
                      if (*Output) {
                          l = strlen(Output) - 1;
                          strncpy(newbuff + j, Output, max - j);
                          cnt2 = max - j - 1;
                          j += MIN(l, cnt2);
                      } else {
                          j--;
                      }
                      FREE(Output);
                  } else {
                      j--;
                  }
                  pbuff--;
              }
              break;
          case '`':
#if ALLOW_BACKQUOTE_EXEC
              D_CONF(("Backquotes detected.  Evaluating expression.\n"));
              if (!in_single) {
                  Command = (char *) MALLOC(CONFIG_BUFF);
                  l = 0;
                  for (pbuff++; *pbuff && *pbuff != '`' && l < max; pbuff++, l++) {
                      Command[l] = *pbuff;
                  }
                  ASSERT_RVAL(l < CONFIG_BUFF, NULL);
                  Command[l] = 0;
                  Command = spifconf_shell_expand(Command);
                  Output = builtin_exec(Command);
                  FREE(Command);
                  if (Output) {
                      if (*Output) {
                          l = strlen(Output) - 1;
                          strncpy(newbuff + j, Output, max - j);
                          cnt2 = max - j - 1;
                          j += MIN(l, cnt2);
                      } else {
                          j--;
                      }
                      FREE(Output);
                  } else {
                      j--;
                  }
              } else {
                  newbuff[j] = *pbuff;
              }
#else
              libast_print_warning("Backquote execution support not compiled in, ignoring\n");
              newbuff[j] = *pbuff;
#endif
              break;
          case '$':
              D_CONF(("Environment variable detected.  Evaluating.\n"));
              if (!in_single) {
                  EnvVar = (char *) MALLOC(128);
                  switch (*(++pbuff)) {
                    case '{':
                        for (pbuff++, k = 0; *pbuff != '}' && k < 127; k++, pbuff++)
                            EnvVar[k] = *pbuff;
                        break;
                    case '(':
                        for (pbuff++, k = 0; *pbuff != ')' && k < 127; k++, pbuff++)
                            EnvVar[k] = *pbuff;
                        break;
                    default:
                        for (k = 0; (isalnum(*pbuff) || *pbuff == '_') && k < 127; k++, pbuff++)
                            EnvVar[k] = *pbuff;
                        break;
                  }
                  EnvVar[k] = 0;
                  if ((tmp = getenv(EnvVar))) {
                      strncpy(newbuff, tmp, max - j);
                      cnt1 = strlen(tmp) - 1;
                      cnt2 = max - j - 1;
                      j += MIN(cnt1, cnt2);
                  }
                  pbuff--;
              } else {
                  newbuff[j] = *pbuff;
              }
              break;
          case '\"':
              D_CONF(("Double quotes detected.\n"));
              if (!in_single) {
                  if (in_double) {
                      in_double = 0;
                  } else {
                      in_double = 1;
                  }
              }
              newbuff[j] = *pbuff;
              break;

          case '\'':
              D_CONF(("Single quotes detected.\n"));
              if (in_single) {
                  in_single = 0;
              } else {
                  in_single = 1;
              }
              newbuff[j] = *pbuff;
              break;

          default:
              newbuff[j] = *pbuff;
        }
    }
    ASSERT_RVAL(j < CONFIG_BUFF, NULL);
    newbuff[j] = 0;

    D_PARSE(("spifconf_shell_expand(%s) returning \"%s\"\n", s, newbuff));
    D_PARSE((" strlen(s) == %lu, strlen(newbuff) == %lu, j == %lu\n", strlen(s), strlen(newbuff), j));

    strcpy(s, newbuff);
#if 0
    FREE(newbuff);
#endif
    return (s);
}

/* The config file reader.  This looks for the config file by searching CONFIG_SEARCH_PATH.
   If it can't find a config file, it displays a warning but continues. -- mej */

char *
spifconf_find_file(const char *file, const char *dir, const char *pathlist)
{

    static char name[PATH_MAX], full_path[PATH_MAX];
    const char *path;
    char *p;
    short maxpathlen;
    unsigned short len;
    struct stat fst;

    REQUIRE_RVAL(file != NULL, NULL);

    getcwd(name, PATH_MAX);
    D_CONF(("spifconf_find_file(\"%s\", \"%s\", \"%s\") called from directory \"%s\".\n", file, NONULL(dir), NONULL(pathlist), name));

    if (dir) {
        strcpy(name, dir);
        strcat(name, "/");
        strcat(name, file);
    } else {
        strcpy(name, file);
    }
    len = strlen(name);
    D_CONF(("Checking for file \"%s\"\n", name));
    if ((!access(name, R_OK)) && (!stat(name, &fst)) && (!S_ISDIR(fst.st_mode))) {
        D_CONF(("Found \"%s\"\n", name));
        return ((char *) name);
    }

    /* maxpathlen is the longest possible path we can stuff into name[].  The - 2 saves room for
       an additional / and the trailing null. */
    if ((maxpathlen = sizeof(name) - len - 2) <= 0) {
        D_CONF(("Too big.  I lose. :(\n", name));
        return ((char *) NULL);
    }

    for (path = pathlist; path != NULL && *path != '\0'; path = p) {
        short n;

        /* Calculate the length of the next directory in the path */
        if ((p = strchr(path, ':')) != NULL) {
            n = p++ - path;
        } else {
            n = strlen(path);
        }

        /* Don't try if it's too long */
        if (n > 0 && n <= maxpathlen) {
            /* Compose the /path/file combo */
            strncpy(full_path, path, n);
            if (full_path[n - 1] != '/') {
                full_path[n++] = '/';
            }
            full_path[n] = '\0';
            strcat(full_path, name);

            D_CONF(("Checking for file \"%s\"\n", full_path));
            if ((!access(full_path, R_OK)) && (!stat(full_path, &fst)) && (!S_ISDIR(fst.st_mode))) {
                D_CONF(("Found \"%s\"\n", full_path));
                return ((char *) full_path);
            }
        }
    }
    D_CONF(("spifconf_find_file():  File \"%s\" not found in path.\n", name));
    return ((char *) NULL);
}

FILE *
spifconf_open_file(char *name)
{
    FILE *fp;
    spif_cmp_t ver;
    char buff[256], test[30], *begin_ptr, *end_ptr;

    ASSERT_RVAL(name != NULL, NULL);

    snprintf(test, sizeof(test), "<%s-", libast_program_name);
    fp = fopen(name, "rt");
    if (fp != NULL) {
        fgets(buff, 256, fp);
        if (strncasecmp(buff, test, strlen(test))) {
            libast_print_warning("%s exists but does not contain the proper magic string (<%s-%s>)\n", name, libast_program_name,
                                 libast_program_version);
            fclose(fp);
            fp = NULL;
        } else {
            begin_ptr = strchr(buff, '-') + 1;
            if ((end_ptr = strchr(buff, '>')) != NULL) {
                *end_ptr = 0;
            }
            ver = spiftool_version_compare(begin_ptr, libast_program_version);
            if (SPIF_CMP_IS_GREATER(ver)) {
                libast_print_warning("Config file is designed for a newer version of %s\n", libast_program_name);
            }
        }
    }
    return (fp);
}

#define SPIFCONF_PARSE_RET()  do {if (!fp) {file_pop(); ctx_end();} return;} while (0)
void
spifconf_parse_line(FILE * fp, char *buff)
{
    register unsigned long i = 0;
    unsigned char id;
    void *state = NULL;

    ASSERT(buff != NULL);

    if (!(*buff) || *buff == '\n' || *buff == '#' || *buff == '<') {
        SPIFCONF_PARSE_RET();
    }
    if (fp == NULL) {
        file_push(NULL, "<argv>", NULL, 0, 0);
        ctx_begin(1);
        buff = spiftool_get_pword(2, buff);
        if (!buff) {
            SPIFCONF_PARSE_RET();
        }
    }
    id = ctx_peek_id();
    spiftool_chomp(buff);
    D_CONF(("Parsing line #%lu of file %s\n", file_peek_line(), file_peek_path()));
    switch (*buff) {
      case '#':
      case '\0':
          SPIFCONF_PARSE_RET();
      case '%':
          if (!BEG_STRCASECMP(spiftool_get_pword(1, buff + 1), "include ")) {
              char *path;
              FILE *fp;

              spifconf_shell_expand(buff);
              path = spiftool_get_word(2, buff + 1);
              if ((fp = spifconf_open_file(path)) == NULL) {
                  libast_print_error("Parsing file %s, line %lu:  Unable to locate %%included config file %s (%s), continuing\n", file_peek_path(),
                              file_peek_line(), path, strerror(errno));
              } else {
                  file_push(fp, path, NULL, 1, 0);
              }
          } else if (!BEG_STRCASECMP(spiftool_get_pword(1, buff + 1), "preproc ")) {
              char cmd[PATH_MAX], fname[PATH_MAX], *outfile;
              int fd;
              FILE *fp;

              if (file_peek_preproc()) {
                  SPIFCONF_PARSE_RET();
              }
              strcpy(fname, "Eterm-preproc-");
              fd = spiftool_temp_file(fname, PATH_MAX);
              outfile = STRDUP(fname);
              snprintf(cmd, PATH_MAX, "%s < %s > %s", spiftool_get_pword(2, buff), file_peek_path(), fname);
              system(cmd);
              fp = fdopen(fd, "rt");
              if (fp != NULL) {
                  fclose(file_peek_fp());
                  file_poke_fp(fp);
                  file_poke_preproc(1);
                  file_poke_outfile(outfile);
              }
          } else {
              if (file_peek_skip()) {
                  SPIFCONF_PARSE_RET();
              }
              spifconf_shell_expand(buff);
          }
          break;
      case 'b':
          if (file_peek_skip()) {
              SPIFCONF_PARSE_RET();
          }
          if (!BEG_STRCASECMP(buff, "begin ")) {
              ctx_begin(2);
              break;
          }
          /* Intentional pass-through */
      case 'e':
          if (!BEG_STRCASECMP(buff, "end ") || !strcasecmp(buff, "end")) {
              ctx_end();
              break;
          }
          /* Intentional pass-through */
      default:
          if (file_peek_skip()) {
              SPIFCONF_PARSE_RET();
          }
          spifconf_shell_expand(buff);
          ctx_poke_state((*ctx_id_to_func(id)) (buff, ctx_peek_state()));
    }
    SPIFCONF_PARSE_RET();
}

#undef SPIFCONF_PARSE_RET

char *
spifconf_parse(char *conf_name, const char *dir, const char *path)
{
    FILE *fp;
    char *name = NULL, *p = ".";
    char buff[CONFIG_BUFF], orig_dir[PATH_MAX];

    REQUIRE_RVAL(conf_name != NULL, 0);

    *orig_dir = 0;
    if (path) {
        if ((name = spifconf_find_file(conf_name, dir, path)) != NULL) {
            if ((p = strrchr(name, '/')) != NULL) {
                getcwd(orig_dir, PATH_MAX);
                *p = 0;
                p = name;
                chdir(name);
            } else {
                p = ".";
            }
        } else {
            return NULL;
        }
    }
    if ((fp = spifconf_open_file(conf_name)) == NULL) {
        return NULL;
    }
    file_push(fp, conf_name, NULL, 1, 0);	/* Line count starts at 1 because spifconf_open_file() parses the first line */

    for (; fstate_idx > 0;) {
        for (; fgets(buff, CONFIG_BUFF, file_peek_fp());) {
            file_inc_line();
            if (!strchr(buff, '\n')) {
                libast_print_error("Parse error in file %s, line %lu:  line too long\n", file_peek_path(), file_peek_line());
                for (; fgets(buff, CONFIG_BUFF, file_peek_fp()) && !strrchr(buff, '\n'););
                continue;
            }
            spifconf_parse_line(fp, buff);
        }
        fclose(file_peek_fp());
        if (file_peek_preproc()) {
            remove(file_peek_outfile());
            FREE(file_peek_outfile());
        }
        file_pop();
    }
    if (*orig_dir) {
        chdir(orig_dir);
    }
    D_CONF(("Returning \"%s\"\n", p));
    return (STRDUP(p));
}

static void *
parse_null(char *buff, void *state)
{

    if (*buff == SPIFCONF_BEGIN_CHAR) {
        return (NULL);
    } else if (*buff == SPIFCONF_END_CHAR) {
        return (NULL);
    } else {
        libast_print_error("Parse error in file %s, line %lu:  Not allowed in \"null\" context:  \"%s\"\n", file_peek_path(), file_peek_line(),
                    buff);
        return (state);
    }
}

/**
 * @defgroup DOXGRP_CONF Configuration File Parser
 *
 * This group of functions/defines/macros comprises the configuration
 * file parsing engine.
 *
 *
 * A small sample program demonstrating some of these routines can be
 * found @link conf_example.c here @endlink.
 */

/**
 * @example conf_example.c
 * Example code for using the config file parser.
 *
 */

/**
 * @defgroup DOXGRP_CONF_FSS File State Stack
 * @ingroup DOXGRP_CONF
 *
 * @note An understanding of the inner workings of the file state
 * stack is not necessary to use the config file parser.  If you
 * aren't interested in understanding the LibAST internals, you should
 * skip most of this section and simply study the examples.
 *
 * Parsers must keep track of various state-related information when
 * parsing files, things like file name, line number, etc.  And since
 * LibAST's config file parser supports the inclusion of sub-files via
 * its %include directive, it must keep track of multiple instances of
 * this information, one for each file.  LibAST uses a structure array
 * called the file state stack.
 *
 * When config file parsing is initiated by a call to spifconf_parse(),
 * the information for that file is pushed onto the empty stack.  For
 * monolithic config files, the stack retains its height throughout
 * the parsing cycle.  However, if an @c %include directive is
 * encountered (and the file is successfully opened), a new set of
 * data is placed atop the stack via file_push().  The new file is
 * then parsed in its entirety, including any sub-files that it may
 * itself include, before its information is popped off the stack and
 * parsing of the original file can continue.
 *
 * Client programs should not need to modify the stack in any way.
 * However, use of the file_peek_path() and file_peek_line() macros
 * are encouraged, specifically for printing error/warning messages.
 * Many of the file state stack manipulation routines should probably
 * never be called by client programs (and are therefore marked as
 * internal); they are, however, made available on the off chance that
 * someone may get super-creative and do something neat with them.
 * Just don't blame LibAST if your (ab)use of internal functions
 * breaks the parser!
 */

/**
 * @defgroup DOXGRP_CONF_CTX Context Handling
 * @ingroup DOXGRP_CONF
 *
 * LibAST-style configuration files are organized into logical units
 * called "contexts."  A begin/end pair is used to surround groups of
 * directives which should be evaluated in a given context.  The begin
 * keyword is followed by the name of the context that will follow.
 * The end keyword may stand alone; anything after it is ignored.
 *
 * The parser starts out in a pseudo-context called @c null for which
 * LibAST employs a built-in handler that rejects any unexpected
 * directives with an error message.  Any other context must be dealt
 * with by a client-specified context handler.
 *
 * Context handlers defined by the client program must conform to the
 * following specification:
 * - Accept two parameters as follows:
 *    -# A char * containing the line of text to be parsed
 *    -# A void * containing optional state information, or NULL
 * - Return a void * containing optional state information, or NULL
 *
 * Although nothing else is strictly @em required by LibAST, if you
 * want your parser to actually work, it needs to handle the LibAST
 * context handler calling conventions.  The following is a
 * step-by-step walk-through of how LibAST calls parser functions:
 *
 * -# When LibAST encounters a @c begin keyword followed by a one or
 *    more additional words (words are separated by whitespace
 *    according to shell conventions), the word immediately following
 *    the @c begin keyword is interpreted as the context name.
 * -# LibAST checks its list of registered context handlers for one
 *    that matches the given context name.  If none is found, an error
 *    is printed, and the parser skips the entire context (until the
 *    next @c end keyword).  Otherwise, go to the next step.
 * -# The registered context handler function is called.  The value
 *    #SPIFCONF_BEGIN_STRING is passed as the first parameter (which I'll
 *    call @a buff ), and NULL is passed as the second parameter
 *    (which I'll call @a state ).
 * -# The context handler should handle this using a statement similar
 *    to the following:
 *     @code
 *     if (*buff == SPIFCONF_BEGIN_CHAR) {
 *     @endcode
 *    (The value of #SPIFCONF_BEGIN_CHAR is such that it should never
 *    occur in normal config file text.)
 *    If the handler does not require any persistent state information
 *    to be kept between calls, it may simply return NULL here.
 *    Otherwise, this portion of the handler should perform any
 *    initialization required for the state information and return a
 *    pointer to that information.
 * -# The value returned by the context handler is stored by LibAST
 *    for later use, and parsing of the config file continues with the
 *    next line.
 * -# Each subsequent line encountered in the config file which does
 *    not start with the keyword @c end is passed to the context
 *    handler function as the first parameter.  The second parameter
 *    will contain the handler's previous return value, the persistent
 *    state information pointer.  The handler, of course, should
 *    continue returning the state information pointer.
 * -# Once the @c end keyword is encountered, the context handler is
 *    called with #SPIFCONF_END_STRING as the first parameter and the
 *    state information pointer as the second parameter.  This
 *    situation should be caught by some code like this:
 *     @code
 *     if (*buff == SPIFCONF_END_CHAR) {
 *     @endcode
 *    Again, the handler should simply return NULL if no state
 *    information is being kept.  Otherwise, any post-processing or
 *    cleanup needed should be done, possibly including the freeing of
 *    the state pointer, etc.  The handler should then return NULL.
 * -# LibAST reverts to the aforementioned @c null context and
 *    continues parsing as above.
 *
 * A sample implementation of context handlers which demonstrate use
 * of this mechanism can be found in the @link conf_example.c config
 * file parser example @endlink.
 */

