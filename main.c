/********************************************************************
vi: set sw=4 ts=4:
$Id: datafeedd.c v0.1 2007/03/31 21:51 Kevin Exp $
********************************************************************/

#include "datafeedd.h"

// global
extern char			    *__progname;
int 					prog_stop 			= 0;
int						prog_debug			= 0;
int						prog_timeout		= TIMEOUT_THREAD;
int						prog_threads		= 0;

// listen
struct listen_pool_t	listen_pool[MAX_FD];
int						listen_changed		= 1;	// 1 for create maxfd and allfds
int						listen_port			= TCP_LISTEN_PORT;
int						listen_fd			= -1;

// ring buffer
struct ring_buf_t		ring_buf[MAX_RINGBUF];
pthread_mutex_t			ring_in_mutex 		= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t			ring_in_done 		= PTHREAD_COND_INITIALIZER;
int						ring_in_pointer		= 0;
pthread_mutex_t			ring_out_mutex 		= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t			ring_out_done 		= PTHREAD_COND_INITIALIZER;
int						ring_out_pointer	= 0;

// config file read
int         g_bMySQLInited = -1;
char 		*g_szMySQLHost = NULL;
char 		*g_szMySQLUser = NULL;
char 		*g_szMySQLPasswd = NULL;
char 		*g_szMySQLDb = NULL;
char 		*config_file = "/etc/datafeedd.conf";
char 		*server = NULL;
char 		*address = NULL;
char 		*port = NULL;

static struct conf_cmd conf_commands[] = {
  { CMD_g_szMySQLHost,   "g_szMySQLHost",   CONF_NEED_ARG, 1, conf_handler, "%s=<MySQLHost>" },
  { CMD_g_szMySQLUser,   "g_szMySQLUser",   CONF_NEED_ARG, 1, conf_handler, "%s=<MySQLUser>" },
  { CMD_g_szMySQLPasswd, "g_szMySQLPasswd", CONF_NEED_ARG, 1, conf_handler, "%s=<MySQLPasswd>" },
  { CMD_g_szMySQLDb,     "g_szMySQLDb",     CONF_NEED_ARG, 1, conf_handler, "%s=<MySQLDb>" },
  { CMD_address,         "address",         CONF_NEED_ARG, 1, conf_handler, "%s=<ip address>" },
  { CMD_daemon,          "daemon",          CONF_NO_ARG,   1, conf_handler, "%s=<command>" },
  { CMD_server,          "server",          CONF_NEED_ARG, 1, conf_handler, "%s=<server name>" },
  { CMD_interface,       "interface",       CONF_NEED_ARG, 1, conf_handler, "%s=<interface>" },
  { 0, 0, 0, 0, 0 }
};


// FIXME: if add keyword
struct keyword_string_t	keyword_string[] = {
	{keyword_AE01,	"AE01"},
	{keyword_AE02,	"AE02"},
	{keyword_AE03,	"AE03"},
	{keyword_AE04,	"AE04"},
	{keyword_AE05,	"AE05"},
	{keyword_AE06,	"AE06"},
	{keyword_unknown, 			NULL},		// Do not remove this, it means end !!!
};



void usage()
{
	fprintf (stderr, "Usage: %s [-d] [-p listen_port]\n"
		"-d: debug.      Specify -d multiple times for increased verbosity.\n"
		"-p listen_port. Default: %d\n"
		, __progname, TCP_LISTEN_PORT);
	exit (1);
} // usage()



void stop_threads()
{
	signal (SIGALRM, SIG_IGN);

	// release threads
	pthread_mutex_lock (&ring_in_mutex);
	if (prog_debug)
		printf ("release_thread(): prog_stop=%d, ring_in_pointer=%d, ", prog_stop, ring_in_pointer);
	pthread_cond_signal (&ring_in_done);
	pthread_mutex_unlock (&ring_in_mutex);

	pthread_mutex_lock (&ring_out_mutex);
	if (prog_debug)
		printf ("ring_out_pointer=%d\n", ring_out_pointer);
	pthread_cond_signal (&ring_in_done);
	pthread_mutex_unlock (&ring_out_mutex);

	// worse case
	if (prog_timeout-- <= 0) {
		if (prog_debug)
			printf ("%s: progname exit.\n", __progname);
		exit(0);
	}

	// wake up myself after one second
	signal (SIGALRM, stop_threads);
    alarm (1);
} // stop_threads()



void sig_handler(int sig)
{
	switch (sig) {
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			if (prog_debug)
				printf ("\nSignal %d caught: program exiting.\n", sig);
			++prog_stop;
			stop_threads();
			break;
	}
} // sig_handler()



void parse_args(int argc, char *argv[])
{
	int i;

	// initialize global variable before getopt()
	for (i=0; i<MAX_FD; ++i) {
		listen_pool[i].enable = listen_pool[i].len = 0;
		listen_pool[i].buf = listen_pool[i].source = NULL;
	}
	for (i=0; i<MAX_RINGBUF; ++i)	// alloc big memory
		if ((ring_buf[i].buf = malloc(MAX_BUFSIZE)) == NULL)
			err (1, "malloc");

	// getopt()
	while ((i = getopt(argc, argv, "dp:f:1:2:3:")) != EOF) {
		switch (i) {
			case 'd':
				++prog_debug;
				break;
			case 'p':
				listen_port = atoi(optarg);
				if (listen_port <= 0 || listen_port > 65530)
					usage();
				break;
			case 'h':
			case '?':
			default:
				usage();
				break;
		}
	} // getopt()

} // parse_args()



void setup_server_listen()
{
	int 				opt;
	struct sockaddr_in	listen_tcpaddr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		err (2, "setup_server_listen: socket");
    
	opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
		(void *)&opt, sizeof(opt)) < 0)
		err (2, "setup_server_listen: setsockopt(SO_REUSEADDR)");
  
#ifdef SO_REUSEPORT
	opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT,
		(void *)&opt, sizeof(opt)) < 0)
		err (2, "setup_server_listen: setsockopt(SO_REUSEPORT)");
#endif

	listen_tcpaddr.sin_port = htons(listen_port);
	listen_tcpaddr.sin_addr.s_addr = INADDR_ANY;
	listen_tcpaddr.sin_family = AF_INET;
	if (bind(listen_fd, (struct sockaddr *)&listen_tcpaddr,
		sizeof(listen_tcpaddr)) < 0)
		err (2, "setup_server_listen: bind");
    
	if (listen(listen_fd, 1) < 0)
		err (2, "setup_server_listen: listen");

	if (prog_debug)
		printf("Listening(fd=%d) for TCP connections on port %hu\n",
			listen_fd, ntohs(listen_tcpaddr.sin_port));

	listen_pool[listen_fd].enable = 1;
} // setup_server_listen()



int main(int argc, char *argv[])
{
	int			i;
	FILE		*fp;
	char		*pidfile = NULL;
	pthread_t 	tid[MAX_THREADS];

	umask (022);
	parse_conf_file("/etc/datafeedd.conf", conf_commands);	
	parse_args(argc, argv);
	setup_server_listen();
	if (!prog_debug)
		daemon (1, 0);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGINT,  sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGPIPE, SIG_IGN);			// tcpserver die
	signal(SIGCHLD, SIG_IGN);
	asprintf (&pidfile, "/var/run/%s.pid", __progname);
    if ((fp = fopen(pidfile, "w"))) {
        fprintf (fp, "%u\n", getpid());
        fclose (fp);
    }

	openlog(__progname, LOG_PID, LOG_LOCAL6);
	syslog (LOG_INFO, "program start.");

	// FIXME: if add thread
	pthread_create (&tid[prog_threads++], NULL, &thread_input, NULL);
	//171
	pthread_create (&tid[prog_threads++], NULL, &thread_write_db, NULL);

	while (!prog_stop)
		sleep (86400);					// sleep forever

	// waitting all threads return
	for (i=0; i<prog_threads; ++i)
		pthread_join (tid[i], NULL);

	if (prog_debug)
		printf ("%s: progname exit.\n", __progname);
	if (pidfile)
		unlink (pidfile);
	exit (0);
} // main()
