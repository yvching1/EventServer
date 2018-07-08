// vi: set sw=4 ts=4:

#ifndef _DATAFEEDD_H
#define _DATAFEEDD_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <setjmp.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include </usr/local/include/mysql/mysql.h>

#define MAX_FD			1024
#define MAX_RINGBUF		100000	// FIXME: (MAX_RINGBUF*MAX_BUFSIZE)<(physical memory/4)
#define MAX_BUFSIZE		512
#define MAX_TOKEN		128
#define MAX_THREADS		10
#define TCP_LISTEN_PORT	514
#define TIMEOUT_THREAD	10		// seconds
#define ARGMAX			128

extern  char                    *__progname;
extern int                      prog_debug;
extern int                      prog_stop;
extern int			prog_threads;
//extern struct keyword_string_t  keyword_string[];

//extern struct ring_buf_t        ring_buf[MAX_RINGBUF];
extern pthread_mutex_t          ring_in_mutex;
extern pthread_cond_t           ring_in_done;
extern int                      ring_in_pointer;
extern pthread_mutex_t          ring_out_mutex;
extern pthread_cond_t           ring_out_done;
extern int                      ring_out_pointer;

extern  int         g_bMySQLInited;
extern  char        *g_szMySQLHost;
extern  char        *g_szMySQLUser;
extern  char        *g_szMySQLPasswd;
extern  char        *g_szMySQLDb;
extern	char        *server;
extern	char        *address;
extern	char        *port;

// FIXME: if add keyword
enum keyword_num_t {
	keyword_unknown = 0,
	keyword_AE01,
	keyword_AE02,
	keyword_AE03,
	keyword_AE04,
	keyword_AE05,
	keyword_AE06
};

struct keyword_string_t {
	enum keyword_num_t	nr;
	char		   *name;
};
//typedef struct keyword_string_t *keyword_strings; 

struct listen_pool_t {
	int				enable;
	char				*buf;
	int				len;					// length of buf
	char				*source;				// accepted host ip:port for debug use
};

struct ring_buf_t {
	int			thread_ref;
	char			*buf;
	enum keyword_num_t	keyword_nr;
	int			argc;
	char			*argv[MAX_TOKEN];
};


// FIXME: if add thread
void *thread_input	__P((void *));
void *thread_write_db	__P((void *));

#define CONF_NO_ARG   0
#define CONF_NEED_ARG 1
#define CONF_OPT_ARG  2
#define g_nMySQLPort    3306

enum {
  CMD__start = 1,
  CMD_g_szMySQLHost,
  CMD_g_szMySQLUser,
  CMD_g_szMySQLPasswd,
  CMD_g_szMySQLDb,
  CMD_server,
  CMD_address,
  CMD_interface,
  CMD_daemon,
  CMD__end
};

struct conf_cmd
{
  int id;
  char *name;
  int arg_type;
  int available;
  int (*proc)(struct conf_cmd *, char *);
  char *help;
};

struct  kSyslog_t {
	char    *mysql;
	char	*tx;
	int     len;
};

int parse_conf_file(char *fname, struct conf_cmd *conf_commands);
int conf_handler(struct conf_cmd *cmd, char *arg);
void convert_1970(char *time_string, int ts_len, struct tm *tm_data);

#endif // _DATAFEEDD_H
