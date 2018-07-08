#include "datafeedd.h"

int option_handler(int id, char *optarg)
{
  char *tmp;

  switch(id)
  {
    case CMD_g_szMySQLHost:
      if(g_szMySQLHost) { free(g_szMySQLHost); }
    	g_szMySQLHost = strdup(optarg);
        printf("MySQLHost:%s \n",g_szMySQLHost);
      break;
    case CMD_g_szMySQLUser:
      if(g_szMySQLUser) { free(g_szMySQLUser); }
      g_szMySQLUser = strdup(optarg);
        printf("MySQLUser:%s \n",g_szMySQLUser);
      break;
    case CMD_g_szMySQLPasswd:
      if(g_szMySQLPasswd) { free(g_szMySQLPasswd); }
      g_szMySQLPasswd = strdup(optarg);
      break;
    case CMD_g_szMySQLDb:
      if(g_szMySQLDb) { free(g_szMySQLDb); }
      g_szMySQLDb = strdup(optarg);
      break;

    case CMD_address:
      if(address) { free(address); }
      address = strdup(optarg);
      break;
    case CMD_server:
      if(server) { free(server); }
      server = strdup(optarg);
      tmp = strchr(server, ':');
      if(tmp)
      {
        *tmp++ = '\0';
        if(port) { free(port); }
        port = strdup(tmp);
      }
      printf("server: %s\n", server);
      printf("port: %s\n", port);
      break;
  }

  return 0;
}

int conf_handler(struct conf_cmd *cmd, char *arg)
{
  return(option_handler(cmd->id, arg));
}

int parse_conf_file(char *fname, struct conf_cmd *commands)
{
  char buf[BUFSIZ+1];
  FILE *in;
  int using_a_file = 1;
  char *p;
  char *cmd_start;
  char *arg;
  struct conf_cmd *cmd;
  int lnum = 0;

  // safety first
  buf[BUFSIZ] = '\0';

  if(strcmp("-", fname) == 0)
  {
		in = stdin;
		using_a_file = 0;
  }
  else
  {
    if((in=fopen(fname, "r")) == NULL)
    {
      fprintf(stderr, "could not open config file \"%s\": %s\n", fname, "error_string");
      return(-1);
    }
  }

  while(lnum++, fgets(buf, BUFSIZ, in) != NULL)
  {
    p = buf;

    /* eat space */
    while(*p == ' ' || *p == '\t') { p++; }
      
    /* ignore comments and blanks */
    if(*p == '#' || *p == '\r' || *p == '\n' || *p == '\0')
    {
      continue;
    }

    cmd_start = p;

    /* chomp new line */
    while(*p != '\0' && *p != '\r' && *p != '\n') { p++; }
    *p = '\0';
    p = cmd_start;

    /* find the end of the command */
    while(*p != '\0' && *p != '=') { p++; }

    /* insure that it is terminated and find arg */
    if(*p == '\0')
    {
      arg = NULL;
    }
    else
    {
      *p = '\0';
      p++;
      arg = p;
    }

    /* look up the command */
    cmd = commands;
    while(cmd->name != NULL)
    {
      if(strcmp(cmd->name, cmd_start) == 0)
      {
        printf("using cmd %s\n", cmd->name);
        break;
      }
      cmd++;
    }
    if(cmd->name == NULL)
    {
      fprintf(stderr, "%s,%d: unknown command: %s\n", fname, lnum, cmd_start);

      fprintf(stderr, "commands are:\n");
      cmd = commands;
      while(cmd->name != NULL)
      {
        fprintf(stderr, "  %-14s usage: ", cmd->name);
        fprintf(stderr, cmd->help, cmd->name);
        fprintf(stderr, "\n");
        cmd++;
      }
      goto ERR;
    }

    /* check the arg */
    switch(cmd->arg_type)
    {
      case CONF_NEED_ARG:
        if(arg == NULL)
        {
          fprintf(stderr, "option \"%s\" requires an argument\n", cmd->name);
          goto ERR;
        }
        break;
      case CONF_OPT_ARG:
        if(arg == NULL)
        {
          arg = "";
        }
        break;
      case CONF_NO_ARG:
        arg = "";
        break;
      default:
        printf( "case not handled: %d\n", cmd->arg_type);
        break;
    }
    
    /* is the command implemented? */
    if(!cmd->available)
    {
      fprintf(stderr, "the command \"%s\" is not available\n", cmd->name);
      continue;
    }

    /* handle the command */
    cmd->proc(cmd, arg);
  }

  if(using_a_file)
  {
    if(in)
    {
      fclose(in);
    }
  }
  return 0;

ERR:
  if(using_a_file)
  {
    if(in)
    {
      fclose(in);
    }
  }
  return(-1);
}

          //if(parse_conf_file(config_file, conf_commands) != 0)
