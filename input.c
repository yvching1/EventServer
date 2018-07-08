#include "datafeedd.h"
// listen
extern struct keyword_string_t 	keyword_string[];
extern struct ring_buf_t	ring_buf[MAX_RINGBUF];
extern struct listen_pool_t	listen_pool[MAX_FD];
extern int			listen_changed;
extern int			listen_port;
extern int			listen_fd;

void new_listen_pool(int fd)
{
	listen_changed = 1;

	// initialize one listen_pool
	listen_pool[fd].enable = 1;
	if (listen_pool[fd].buf == NULL)	// first time use, alloc it
		listen_pool[fd].buf = malloc (MAX_BUFSIZE);
	listen_pool[fd].len = 0;
	if (listen_pool[fd].source)			// clear for asprintf()
		free (listen_pool[fd].source);
} // new_listen_pool()

void incoming_connection()
{
	struct sockaddr_in 	client_addr;
	int 				accept_fd, addrlen = sizeof(client_addr);
	int					i;
	char				*p;

	if ((accept_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &addrlen)) < 0) {
		warn ("accept()");
		return;
	}
	new_listen_pool (accept_fd);
	asprintf (&listen_pool[accept_fd].source, "%s", inet_ntoa(client_addr.sin_addr));

	// check duplicated source ip
	for (i=0, p=listen_pool[accept_fd].source; i<MAX_FD; ++i)
		if (i != accept_fd && 
			i != listen_fd && 
			listen_pool[i].enable && 
			strcmp (listen_pool[i].source, p) == 0) {
			// found reconnect
			close (i);
			listen_pool[i].enable = 0;
			++listen_changed;
			if (prog_debug)
				printf ("reconnect from %s, old fd=%d closed.\n", p, i);
		}
	if (prog_debug)
		printf ("accept(fd=%d) from %s\n", accept_fd, listen_pool[accept_fd].source);
} // incoming_connections()

void insert_into_ring_buffer(char *sb, char *se)
{
	char *p, *q, buft[512];
	int  found, nr, ring_next, ring_out_now;

	if (prog_stop)
		return;

	// remove char before KEYWORD
	for (p=sb, found=0; *p && *(p+1); ++p)
		if (isupper(*p) && isupper(*(p+1))) {
			++found;
			break;
		}
	if (!found)
		return;

	// Is it a KEYWORD ?
	if ((q = strsep (&p, " \t\n")) == NULL)
		return;
	for (nr=0, found=0; keyword_string[nr].nr != keyword_unknown; ++nr)
		if (strcmp (q, keyword_string[nr].name) == 0) {
			++found;
			break;
		}
	if (!found)
		return;
	ring_buf[ring_in_pointer].keyword_nr = keyword_string[nr].nr;			// got ring_buf->keyword_nr
	memcpy (ring_buf[ring_in_pointer].buf, p, se-p+1);	// got ring_buf->buf

	sprintf(buft, "%s %s", keyword_string[ring_buf[ring_in_pointer].keyword_nr-1].name, ring_buf[ring_in_pointer].buf);
#ifdef LOG_LOCAL6
	openlog(__progname, LOG_PID, LOG_LOCAL6);
#else
	openlog(__progname, LOG_PID);
#endif	
	syslog(LOG_INFO, buft);
	closelog();

	// parse
	q = ring_buf[ring_in_pointer].buf;
	for (found=0; (p = strsep (&q, " \t\n")) != NULL; )
		if (*p)
			ring_buf[ring_in_pointer].argv[found++] = p;// got ring_buf->argv[]
	ring_buf[ring_in_pointer].argc = found;				// got ring_buf->argc

	// thread_ref except myself							// set ring_buf->thread_ref
	ring_buf[ring_in_pointer].thread_ref = prog_threads - 1;

	if (prog_debug > 1) {
		int i;
		printf ("ring_in=%d, keyword_nr=%d(%s), argc=%d, argv=%s",
			ring_in_pointer, nr, keyword_string[nr].name, found, 
			ring_buf[ring_in_pointer].argv[0]);

		if (prog_debug > 2)
			for (i=1; i<found; ++i)
				printf (" %s", ring_buf[ring_in_pointer].argv[i]);
		printf ("\n");
	}
			
	// prevent ring buffer overflow 
	ring_next = (ring_in_pointer + 1) % MAX_RINGBUF;
	pthread_mutex_lock (&ring_out_mutex);
	while (!prog_stop && ring_next == ring_out_pointer)
		pthread_cond_wait (&ring_out_done, &ring_out_mutex);
	ring_out_now = ring_out_pointer;
	pthread_mutex_unlock (&ring_out_mutex);

	// next
	pthread_mutex_lock (&ring_in_mutex);
	ring_in_pointer = ring_next;
	pthread_cond_signal (&ring_in_done);
	pthread_mutex_unlock (&ring_in_mutex);
	if (prog_debug > 1)
		printf ("insert_into_ring_buffer(): move ring_in=%d (ring_out=%d)\n",
			ring_in_pointer, ring_out_now);

} // insert_into_ring_buffer()

void fd_reading(int fd)
{
	int 	nread, nlines = 0;
	char 	*p = listen_pool[fd].buf + listen_pool[fd].len;
	char	*q = listen_pool[fd].buf;
	char	*newline;

	// prevent buffer overflow
	if (listen_pool[fd].len >= MAX_BUFSIZE) {
		if (prog_debug)
			printf ("listen_pool[%d] overflow, clearing.\n", fd);
		listen_pool[fd].len = 0;
		p = q;
	}

	nread = read (fd, p, MAX_BUFSIZE - listen_pool[fd].len);
	if (nread <= 0) {
		if (prog_debug) {
			if (nread == 0)
				printf ("fd_reading(): source=%s, fd=%d, nread=%d: session end.\n",
					listen_pool[fd].source, fd, nread);
			else
				printf ("fd_reading(): source=%s, fd=%d, nread=%d\n",
					listen_pool[fd].source, fd, nread);
		}
		close (fd);
		listen_pool[fd].enable = 0;
		++listen_changed;
		return;
	}
	listen_pool[fd].len += nread;
	if (prog_debug > 2)
		printf ("source=%s, fd=%d, nread=%d, pool_len=%d\n",
			listen_pool[fd].source, fd, nread, listen_pool[fd].len);

	// search newline then insert_into_ring_buffer()
	while (nread > 0 && (newline = memchr (p, '\n', nread))) {
		*newline = 0;
		insert_into_ring_buffer(q, newline);
		++nlines;
		++newline;
		nread = nread - (newline - p);
		p = q = newline;
	} // while()

	// move remains to the header
	if (q > listen_pool[fd].buf) {
		if (nread > 0)
			memmove (listen_pool[fd].buf, q, nread);
		listen_pool[fd].len = nread;
	}
	if (prog_debug > 2)
		printf ("remains=%d, pool_len=%d, %d lines printed.\n", nread, listen_pool[fd].len, nlines);
} // fd_reading()

void *thread_input(void *arg)
{
	FILE			*fp;
	fd_set			allfds, readfds;
	int				i, maxfd=0;
	struct timeval	tv;

	if (prog_debug)
		printf ("thread_input(): begin.\n");

	// MAIN LOOP
	while (!prog_stop) {

		// generate maxfd and allfds
		if (listen_changed) {
			FD_ZERO(&allfds);
			for (i=0; i<MAX_FD; ++i) {
				if (listen_pool[i].enable) {
					FD_SET(i, &allfds);
					maxfd = i + 1;
				}
			}
			listen_changed = 0;
		}

		// select
		readfds = allfds;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		if (select(maxfd, &readfds, NULL, NULL, &tv) <= 0)
			continue;
		for (i=0; i<maxfd; ++i) {
			if (FD_ISSET(i, &readfds)) {
				if (i == listen_fd)
					incoming_connection();
				else
					fd_reading(i);
			}
		}
	} // while()

	// close all active input
	if (prog_debug)
		printf ("thread_input(): close %d active input fd(s).\n", maxfd);
	for (i=0; i<maxfd; ++i)
		if (listen_pool[i].enable)
			close (i);
	if (prog_debug)
		printf ("thread_input(): end.\n");
} // thread_input()
